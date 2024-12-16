import time
import tensorflow as tf
import numpy as np
import socket
from sionna.rt import load_scene, PlanarArray, Transmitter, Receiver, Paths
from sionna.constants import SPEED_OF_LIGHT
import os, subprocess, signal
import argparse

# Create the parser
parser = argparse.ArgumentParser(description='Sionna Server Script')

# Add arguments
parser.add_argument('--filename', type=str, default='scenarios/SionnaCircleScenario/scene.xml', help='Path to the scenario file', required=True)
parser.add_argument('--local-machine', action='store_true', help='Flag to indicate if running on a local machine')
parser.add_argument('--verbose', action='store_true', help='Flag to indicate if the user desires a verbose output')

# Parse the arguments
args = parser.parse_args()

# Use the parsed arguments
file_name = args.filename
local_machine = args.local_machine
verbose = args.verbose

# file_name = "scenarios/SionnaCircleScenario/scene.xml"
# local_machine = False
# verbose = False

def kill_process_using_port(port):
    try:
        result = subprocess.run(['lsof', '-i', f':{port}'], stdout=subprocess.PIPE)
        for line in result.stdout.decode('utf-8').split('\n'):
            if 'LISTEN' in line:
                pid = int(line.split()[1])
                os.kill(pid, signal.SIGKILL)
                if verbose:
                    print(f"Killed process {pid} using port {port}")
    except Exception as e:
        print(f"Error killing process using port {port}: {e}")

kill_process_using_port(8103)

# Configure which GPU to use
if os.getenv("CUDA_VISIBLE_DEVICES") is None:
    gpu_num = 2  # Use "" to use only CPU
    os.environ["CUDA_VISIBLE_DEVICES"] = f"{gpu_num}"
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'

# Set memory growth consistently for all GPUs
gpus = tf.config.list_physical_devices('GPU')
if gpus:
    try:
        for gpu in gpus:
            tf.config.experimental.set_memory_growth(gpu, True)
    except RuntimeError as e:
        print(e)

# Avoid warnings from TensorFlow
tf.get_logger().setLevel('ERROR')

# Scene, change here the path to your scenario
scene = load_scene(file_name)

# Radio settings
scene.frequency = 5.89e9  # in Hz
scene.synthetic_array = True  # If set to False, ray tracing will be done per antenna element (slower for large arrays)
# Antenna settings
antenna_displacement = [0, 0, 1.5]  # location of the antenna wrt the car mesh
element_spacing = SPEED_OF_LIGHT / scene.frequency / 2
array = PlanarArray(1, 1, element_spacing, element_spacing, "iso", "V")  # 1x1 isotropic antenna
# SUMO update granularity
position_threshold = 3  # Update object position every position_threshold [meters]
angle_threshold = 90  # Update object angle every angle_threshold [degrees]
# Rays computation settings
max_depth = 5
num_samples = 1e4

# Create a UDP socket
udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
# ... and bind it to the correct (address, port)
if local_machine:
    udp_socket.bind(("127.0.0.1", 8103))  # use this if ns3 in localhost, chose whatever port you prefer
else:
    udp_socket.bind(("0.0.0.0", 8103))  # if you use an external server with Sionna and ns3 in another machine

# Chaches and other variables
SUMO_live_location_db = {}  # Stores the live location of each vehicle in SUMO
Sionna_location_db = {}  # Stores the location of each vehicle in Sionna, not updated in real-time as too computationally expensive

rays_cache = {}

pathloss_cache = {}
delay_cache = {}
last_pathloss_requested = None


def ManageLocationMessage(message):

    global pathloss_cache
    global rays_cache

    try:
        # Hangle map_update message
        data = message[len("map_update:") :]
        parts = data.split(",")
        car = int(parts[0].replace("veh", ""))

        new_x = float(parts[1])
        new_y = float(parts[2])
        new_z = float(parts[3]) + 1
        new_angle = float(parts[4])

        # Save in SUMO_live_location_db
        SUMO_live_location_db[car] = {"x": new_x, "y": new_y, "z": new_z, "angle": new_angle}

        # Check if the vehicle exists in Sionna_location_db
        if car in Sionna_location_db:
            # Fetch the old position and angle
            old_x = Sionna_location_db[car]["x"]
            old_y = Sionna_location_db[car]["y"]
            old_z = Sionna_location_db[car]["z"]
            old_angle = Sionna_location_db[car]["angle"]

            # Check if the position or angle has changed by more than the thresholds
            position_changed = (
                    abs(new_x - old_x) >= position_threshold
                    or abs(new_y - old_y) >= position_threshold
                    or abs(new_z - old_z) >= position_threshold
            )
            angle_changed = abs(new_angle - old_angle) >= angle_threshold
        else:
            # No previous record, so this is the first update (considered a change)
            position_changed = True
            angle_changed = True

        # If the position or angle has changed, update the dictionary and the scene
        if position_changed or angle_changed:
            # Update Sionna_location_db with the new values
            Sionna_location_db[car] = SUMO_live_location_db[car]
            # Clear the pathloss cache as one of the car's position has changed (must do for vNLOS cases)
            pathloss_cache = {}
            rays_cache = {}
            #print("Pathloss cache cleared.")
            # Print the updated car's information for logging
            if verbose:
                print(f"car_{car} - Position: [{new_x}, {new_y}, {new_z}] - Angle: {new_angle}")
            # Apply changes to the scene
            if scene.get(f"car_{car}"):  # Make sure the object exists in the scene
                from_sionna = scene.get(f"car_{car}")
                from_sionna.position = [new_x, new_y, new_z]
                # Orientation is not changed because of a SIONNA bug (kernel crashes)
                # new_orientation = (new_angle*np.pi/180, 0, 0)
                # from_sionna.orientation = type(from_sionna.orientation)(new_orientation, device=from_sionna.orientation.device)

                if verbose:
                    print(f"Updated car_{car} position in the scene.")
            else:
                print(f"ERROR: no car_{car} in the scene, use Blender to check")

            scene.remove(f"car_{car}_tx_antenna")
            scene.remove(f"car_{car}_rx_antenna")
        return car

    except (IndexError, ValueError) as e:
        print(f"EXCEPTION - Location parsing failed: {e}")
        return None


def matchRaysToCars(paths, Sionna_location_db, tolerance=position_threshold, antenna_displacement=antenna_displacement):
    matched_paths = {}
    targets = paths.targets.numpy()
    sources = paths.sources.numpy()

    # paths_mask_np = paths.mask.numpy()
    path_coefficients_np = paths.a.numpy()
    delays_np = paths.tau.numpy()
    types_np = paths.types.numpy()
    paths_mask_np = paths.mask.numpy()

    '''
    # Currently unused, may be useful for future work: commented for performance
    theta_t_np = paths.theta_t.numpy()
    phi_t_np = paths.phi_t.numpy()
    theta_r_np = paths.theta_r.numpy()
    phi_r_np = paths.phi_r.numpy()
    doppler_np = paths.doppler.numpy()
    '''

    # Pre-adjust car locations with antenna displacement
    adjusted_car_locs = {
        car_id: {"x": car_loc["x"] + antenna_displacement[0], "y": car_loc["y"] + antenna_displacement[1], "z": car_loc["z"] + antenna_displacement[2]}
        for car_id, car_loc in Sionna_location_db.items()
    }
    car_ids = np.array(list(adjusted_car_locs.keys()))
    car_positions = np.array([[loc["x"], loc["y"], loc["z"]] for loc in adjusted_car_locs.values()])

    # Iterate over each source (TX)
    for tx_idx, source in enumerate(sources):
        distances = np.linalg.norm(car_positions - source, axis=1)
        source_within_tolerance = distances <= tolerance

        if np.any(source_within_tolerance):
            min_idx = np.argmin(distances[source_within_tolerance])
            source_closest_car_id = car_ids[source_within_tolerance][min_idx]
            matched_source_car_name = f"car_{source_closest_car_id}"

            if matched_source_car_name not in matched_paths:
                matched_paths[matched_source_car_name] = {}

            # Iterate over targets for the current source (TX)
            for rx_idx, target in enumerate(targets):
                if rx_idx >= paths.mask.shape[1]:
                    continue
                distances = np.linalg.norm(car_positions - target, axis=1)
                target_within_tolerance = distances <= tolerance

                if np.any(target_within_tolerance):
                    min_idx = np.argmin(distances[target_within_tolerance])
                    target_closest_car_id = car_ids[target_within_tolerance][min_idx]
                    matched_target_car_name = f"car_{target_closest_car_id}"

                    if matched_target_car_name not in matched_paths[matched_source_car_name]:
                        matched_paths[matched_source_car_name][matched_target_car_name] = {
                            # 'paths_mask': [],
                            'path_coefficients': [],
                            'delays': [],
                            # 'angles_of_departure': {'zenith': [], 'azimuth': []},
                            # 'angles_of_arrival': {'zenith': [], 'azimuth': []},
                            # 'doppler': [],
                            'is_los': []
                        }

                    # Populate path data
                    try:
                        matched_paths[matched_source_car_name][matched_target_car_name]['path_coefficients'].append(path_coefficients_np[0, rx_idx, 0, tx_idx, 0, ...])
                        matched_paths[matched_source_car_name][matched_target_car_name]['delays'].append(delays_np[0, rx_idx, tx_idx, ...])

                        '''
                        # Currently unused, may be useful for future work: commented for performance                        
                        matched_paths[matched_source_car_name][matched_target_car_name]['angles_of_departure']['zenith'].append(theta_t_np[0, rx_idx, tx_idx, ...])
                        matched_paths[matched_source_car_name][matched_target_car_name]['angles_of_departure']['azimuth'].append(phi_t_np[0, rx_idx, tx_idx, ...])
                        matched_paths[matched_source_car_name][matched_target_car_name]['angles_of_arrival']['zenith'].append(theta_r_np[0, rx_idx, tx_idx, ...])
                        matched_paths[matched_source_car_name][matched_target_car_name]['angles_of_arrival']['azimuth'].append(phi_r_np[0, rx_idx, tx_idx, ...])
                        matched_paths[matched_source_car_name][matched_target_car_name]['doppler'].append(doppler_np[0, rx_idx, tx_idx, ...])
                        '''

                        # Extract LoS determination
                        valid_paths_mask = paths_mask_np[0, rx_idx, tx_idx, :]
                        valid_path_indices = np.where(valid_paths_mask)[0]
                        valid_path_types = types_np[0][valid_path_indices]
                        # Check if any path is LoS
                        is_los = np.any(valid_path_types == Paths.LOS)
                        matched_paths[matched_source_car_name][matched_target_car_name]['is_los'].append(bool(is_los))

                    except (IndexError, tf.errors.InvalidArgumentError) as e:
                        print(f"Error encountered for source {tx_idx}, target {rx_idx}: {e}")
                        continue
                else:
                    if verbose:
                        print(f"Warning - No car within tolerance for target {rx_idx} (for source {tx_idx})")
        else:
            if verbose:
                print(f"Warning - No car within tolerance for source {tx_idx}")

    return matched_paths


def computeRays():
    t = time.time()

    global rays_cache
    global array

    scene.tx_array = array
    scene.rx_array = array

    # Ensure every car in the simulation has antennas (one for TX and one for RX)
    for car_id in Sionna_location_db:
        tx_antenna_name = f"car_{car_id}_tx_antenna"
        rx_antenna_name = f"car_{car_id}_rx_antenna"
        car_position = np.array([Sionna_location_db[car_id]['x'], Sionna_location_db[car_id]['y'], Sionna_location_db[car_id]['z']])
        tx_position = car_position + np.array(antenna_displacement)
        rx_position = car_position + np.array(antenna_displacement)

        if scene.get(tx_antenna_name) is None:
            scene.add(Transmitter(tx_antenna_name, position=tx_position, orientation=[0, 0, 0]))
            scene.tx_array = scene.tx_array
            if verbose:
                print(f"Added TX antenna for car_{car_id}: {tx_antenna_name}")

        if scene.get(rx_antenna_name) is None:
            scene.add(Receiver(rx_antenna_name, position=rx_position, orientation=[0, 0, 0]))
            scene.rx_array = scene.rx_array
            if verbose:
                print(f"Added RX antenna for car_{car_id}: {rx_antenna_name}")

    # Compute paths
    paths = scene.compute_paths(max_depth=max_depth, num_samples=num_samples, diffraction=True, scattering=True)
    paths.normalize_delays = False  # Do not normalize delays to the first path
    if verbose:
        print(f"Ray tracing took: {(time.time() - t) * 1000} ms")
    t = time.time()
    matched_paths = matchRaysToCars(paths, Sionna_location_db)  # matched_paths[source_name][target_name]
    if verbose:
        print(f"Matching rays to cars took: {(time.time() - t) * 1000} ms")

    # Iterate over sources in matched_paths
    for src_car_id in Sionna_location_db:
        current_source_car_name = f"car_{src_car_id}"
        if current_source_car_name in matched_paths:
            matched_paths_for_source = matched_paths[current_source_car_name]

            # Iterate over targets for the current source
            for trg_car_id in Sionna_location_db:
                current_target_car_name = f"car_{trg_car_id}"
                if current_target_car_name != current_source_car_name:  # Skip case where source == target
                    if current_target_car_name in matched_paths_for_source:
                        if current_source_car_name not in rays_cache:
                            rays_cache[current_source_car_name] = {}
                        # Cache the matched paths for this source-target pair
                        rays_cache[current_source_car_name][current_target_car_name] = matched_paths_for_source[current_target_car_name]
                        if verbose:
                            print(f"Cached paths for source {current_source_car_name} to target {current_target_car_name}")
                    else:
                        # Force an update if the source or target wasn't matched
                        for car_id in Sionna_location_db:
                            car_name = f"car_{car_id}"
                            if scene.get(car_name):
                                from_sionna = scene.get(car_name)
                                new_position = [SUMO_live_location_db[car_id]["x"], SUMO_live_location_db[car_id]["y"], SUMO_live_location_db[car_id]["z"] ]
                                from_sionna.position = new_position
                                # Update Sionna location database with new positions
                                Sionna_location_db[car_id] = { "x": new_position[0], "y": new_position[1], "z": new_position[2], "angle": SUMO_live_location_db[car_id]["angle"] }
                                # Update antenna positions
                                if scene.get(f"{car_name}_tx_antenna"):
                                    scene.get(f"{car_name}_tx_antenna").position = [new_position[0] + antenna_displacement[0], new_position[1] + antenna_displacement[1], new_position[2] + antenna_displacement[2]]
                                    if verbose:
                                        print(f"Forced update for {car_name} and its TX antenna in the scene.")
                                if scene.get(f"{car_name}_rx_antenna"):
                                    scene.get(f"{car_name}_rx_antenna").position = [new_position[0] + antenna_displacement[0], new_position[1] + antenna_displacement[1], new_position[2] + antenna_displacement[2]]
                                    if verbose:
                                        print(f"Forced update for {car_name} and its RX antenna in the scene.")
                            else:
                                print(f"ERROR: no {car_name} in the scene for forced update, use Blender to check")

                        # Re-do matching with updated locations
                        t = time.time()
                        matched_paths = matchRaysToCars(paths, Sionna_location_db)
                        if verbose:
                            print(f"Matching rays to cars (double exec) took: {(time.time() - t)*1000} ms")
                        if current_source_car_name not in rays_cache:
                            rays_cache[current_source_car_name] = {}
                        if current_target_car_name in matched_paths[current_source_car_name]:
                            rays_cache[current_source_car_name][current_target_car_name] = matched_paths[current_source_car_name][current_target_car_name]

    return None


def GetPathloss(car1_id, car2_id):

    global rays_cache

    # Was the requested value already calculated?
    if car1_id not in rays_cache or car2_id not in rays_cache[car1_id]:
        computeRays()

    path_coefficients = rays_cache[car1_id][car2_id]["path_coefficients"]
    sum = np.sum(path_coefficients)
    abs = np.abs(sum)
    square = abs ** 2
    total_cir = square

    # Calculate path loss in dB
    if total_cir > 0:
        path_loss = -10 * np.log10(total_cir)
    else:
        # Handle the case where path loss calculation is not valid
        if verbose:
            print(f"Pathloss calculation failed for {car1_id}-{car2_id}: got infinite value (not enough rays). Returning 300 dB.")
        path_loss = 300  # Assign 300 dB for loss cases

    return path_loss


def ManagePathlossRequest(message):
    try:
        data = message[len("calc_request:"):]
        parts = data.split(",")
        car_a_str = parts[0].replace("veh", "")
        car_b_str = parts[1].replace("veh", "")

        # Getting each car_id, the origin is marked as 0
        car_a_id = "origin" if car_a_str == "0" else f"car_{int(car_a_str)}" if car_a_str else "origin"
        car_b_id = "origin" if car_b_str == "0" else f"car_{int(car_b_str)}" if car_b_str else "origin"

        if car_a_id == "origin" or car_b_id == "origin":
            # If any, ignoring pathloss requests from the origin, used for statistical calibration
            pathloss_value = 0
        else:
            t = time.time()
            pathloss_value = GetPathloss(car_a_id, car_b_id)

        return pathloss_value

    except (ValueError, IndexError) as e:
        print(f"EXCEPTION - Error processing pathloss request: {e}")
        return None


def GetDelay(car1_id, car2_id):
    global rays_cache

    # Check and compute rays only if necessary
    if car1_id not in rays_cache or car2_id not in rays_cache[car1_id]:
        computeRays()

    delays = np.abs(rays_cache[car1_id][car2_id]["delays"])
    delays_flat = delays.flatten()

    # Filter positive values
    positive_values = delays_flat[delays_flat >= 0]

    if positive_values.size > 0:
        min_positive_value = np.min(positive_values)
    else:
        min_positive_value = 1e5

    return min_positive_value


def ManageDelayRequest(message):
    try:
        data = message[len("get_delay:"):]
        parts = data.split(",")
        car_a_str = parts[0].replace("veh", "")
        car_b_str = parts[1].replace("veh", "")

        # Getting each car_id, the origin is marked as 0
        car_a_id = "origin" if car_a_str == "0" else f"car_{int(car_a_str)}" if car_a_str else "origin"
        car_b_id = "origin" if car_b_str == "0" else f"car_{int(car_b_str)}" if car_b_str else "origin"

        if car_a_id == "origin" or car_b_id == "origin":
            # If any, ignoring pathloss requests from the origin, used for statistical calibration
            delay = 0
        else:
            delay = GetDelay(car_a_id, car_b_id)

        return delay

    except (ValueError, IndexError) as e:
        print(f"EXCEPTION - Error processing delay request: {e}")
        return None


def ManageLOSRequest(message):
    try:
        data = message[len("are_they_LOS:") :]
        parts = data.split(",")
        car_a_str = parts[0].replace("veh", "")
        car_b_str = parts[1].replace("veh", "")

        # Getting each car_id, the origin is marked as 0
        car_a_id = "origin" if car_a_str == "0" else f"car_{int(car_a_str)}" if car_a_str else "origin"
        car_b_id = "origin" if car_b_str == "0" else f"car_{int(car_b_str)}" if car_b_str else "origin"

        if car_a_id == "origin" or car_b_id == "origin":
            # If any, ignoring pathloss requests from the origin, used for statistical calibration
            los = 0
        else:
            los = rays_cache[car_a_id][car_b_id]["is_los"]

        return los

    except (ValueError, IndexError) as e:
        print(f"EXCEPTION - Error processing LOS request: {e}")
        return None


if __name__ == "__main__":

    # Startpoint
    print("Sionna is now ready to handle messages... waiting")
    freq = scene.frequency / 1e9
    print(f"Sionna is now ready to handle messages for {freq}GHz... waiting")

    while True:
        # Receive data from the socket
        payload, address = udp_socket.recvfrom(1024)
        message = payload.decode()

        if message.startswith("map_update:"):
            updated_car = ManageLocationMessage(message)
            if updated_car is not None:
                response = "UPDATEDveh" + str(updated_car)
                udp_socket.sendto(response.encode(), address)

        if message.startswith("calc_request:"):
            pathloss = ManagePathlossRequest(message)
            if pathloss is not None:
                # Use pathloss + txPower (dBm) for 80211p
                # response = "CALC_DONE:" + str(pathloss + 23)
                response = "CALC_DONE:" + str(pathloss)
                udp_socket.sendto(response.encode(), address)

        if message.startswith("get_delay:"):
            delay = ManageDelayRequest(message)
            if delay is not None:
                response = "DELAY:" + str(delay)
                udp_socket.sendto(response.encode(), address)

        if message.startswith("are_they_LOS:"):
            los = ManageLOSRequest(message)
            if los is not None:
                response = "LOS:" + str(los)
                udp_socket.sendto(response.encode(), address)

        if message.startswith("kill_sionna"):
            print("Killing Sionna")
            udp_socket.close()
            break
