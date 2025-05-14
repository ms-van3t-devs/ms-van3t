import time
import os
import tensorflow as tf
import numpy as np
import socket
from sionna.rt import load_scene, PlanarArray, Transmitter, Receiver, PathSolver
from scipy.spatial import cKDTree
import subprocess, signal
import argparse


def manage_location_message(message, sionna_structure):
    t = time.time()
    try:
        # LOC_UPDATE handling
        if sionna_structure["verbose"]:
            print(f"LOC_UPDATE message to handle: {message}")
        
        data = message[len("LOC_UPDATE:"):]
        parts = data.split(",")
        car = int(parts[0].replace("veh", ""))

        new_x = float(parts[1])
        new_y = float(parts[2])
        new_z = float(parts[3]) + 0
        
        new_angle = float(parts[4])

        new_v_x = float(parts[5])
        new_v_y = float(parts[6])
        new_v_z = float(parts[7])

        sionna_structure["SUMO_live_location_db"][car] = {"x": new_x, "y": new_y, "z": new_z, "angle": new_angle, "v_x": new_v_x, "v_y": new_v_y, "v_z": new_v_z}

        # 0 - Is vehicle is sionna_location_db?
        if car in sionna_structure["sionna_location_db"]:
            # Fetch the old values
            old_x = sionna_structure["sionna_location_db"][car]["x"]
            old_y = sionna_structure["sionna_location_db"][car]["y"]
            old_z = sionna_structure["sionna_location_db"][car]["z"]
            old_angle = sionna_structure["sionna_location_db"][car]["angle"]

            if sionna_structure["verbose"]:
                print(f"Found in scenario database - Old position: [{old_x}, {old_y}, {old_z}] - Old angle: {old_angle}")

            # Check if the position or angle has changed by more than the thresholds
            position_changed = (
                    abs(new_x - old_x) >= sionna_structure["position_threshold"]
                    or abs(new_y - old_y) >= sionna_structure["position_threshold"]
                    or abs(new_z - old_z) >= sionna_structure["position_threshold"]
            )
            angle_changed = abs(new_angle - old_angle) >= sionna_structure["angle_threshold"]

            if sionna_structure["verbose"] and (position_changed or angle_changed):
                print(f"Update needed for car_{car} - Position changed: {position_changed} - Angle changed: {angle_changed}")
        
        else:
            # First update ever
            if sionna_structure["verbose"]: 
                print(f"First update ever for car_{car} - No previous values found. Forcing update...")
                print(f"New position requested: [{new_x}, {new_y}, {new_z}] - Angle: {new_angle}")
            
            position_changed = True
            angle_changed = True
        
        # 1 - If needed, update Sionna scenario
        if position_changed or angle_changed:
            sionna_structure["sionna_location_db"][car] = sionna_structure["SUMO_live_location_db"][car]
            # Clear caches upon scenario update
            sionna_structure["path_loss_cache"] = {}
            sionna_structure["rays_cache"] = {}
            if sionna_structure["verbose"]:
                print("Pathloss and rays caches cleared.") 

            # Apply change to the scene
            if sionna_structure["scene"].get(f"car_{car}"):
                from_sionna = sionna_structure["scene"].get(f"car_{car}")
            
                new_orientation = ((360 - new_angle) % 360 + 90)*np.pi/180

                from_sionna.position = [new_x, new_y, new_z]
                from_sionna.orientation = [new_orientation, 0, 0]
                from_sionna.velocity = [new_v_x, new_v_y, new_v_z]
                
                if sionna_structure["verbose"]: 
                    print(f"Updated car_{car} position in the scene.")
            else:
                print(f"ERROR: no car_{car} in the scene, use Blender to check")

            sionna_structure["scene"].remove(f"car_{car}_tx_antenna")
            sionna_structure["scene"].remove(f"car_{car}_rx_antenna")
            if sionna_structure["verbose"]:
                print(f"Removed antennas for car_{car} from the scene.")
        
        if sionna_structure["time_checker"]:
            print(f"Location update took: {(time.time() - t) * 1000} ms")
        return car

    except (IndexError, ValueError) as e:
        print(f"EXCEPTION - Location parsing failed: {e}")
        return None

def match_rays_to_cars(paths, sionna_structure):
    t = time.time()
    matched_paths = {}
    
    # Extract and transpose source and target positions
    targets = paths._tgt_positions.numpy().T
    sources = paths._src_positions.numpy().T 

    # Path parameters
    a_real, a_imag = paths.a
    path_coefficients = a_real.numpy() + 1j * a_imag.numpy()
    delays = paths.tau.numpy()
    interactions = paths.interactions.numpy()
    valid = paths.valid.numpy()

    # Adjust car positions for antenna displacement
    adjusted_car_locs = {
        car_id: {
            "x": car_loc["x"] + sionna_structure["antenna_displacement"][0],
            "y": car_loc["y"] + sionna_structure["antenna_displacement"][1],
            "z": car_loc["z"] + sionna_structure["antenna_displacement"][2],
        }
        for car_id, car_loc in sionna_structure["sionna_location_db"].items()
    }

    car_ids = list(adjusted_car_locs.keys())
    car_positions = np.array([[v["x"], v["y"], v["z"]] for v in adjusted_car_locs.values()])
    car_tree = cKDTree(car_positions)

    # Match sources and targets
    source_dists, source_indices = car_tree.query(sources, distance_upper_bound=sionna_structure["position_threshold"])
    target_dists, target_indices = car_tree.query(targets, distance_upper_bound=sionna_structure["position_threshold"])

    for tx_idx, src_idx in enumerate(source_indices):
        if src_idx == len(car_ids):
            if sionna_structure["verbose"]:
                print(f"Warning - No car within tolerance for source {tx_idx}")
            continue

        matched_source_car_name = f"car_{car_ids[src_idx]}"
        if matched_source_car_name not in matched_paths:
            matched_paths[matched_source_car_name] = {}

        for rx_idx, tgt_idx in enumerate(target_indices):
            if tgt_idx == len(car_ids):
                if sionna_structure["verbose"]:
                    print(f"Warning - No car within tolerance for target {rx_idx} (for source {tx_idx})")
                continue

            matched_target_car_name = f"car_{car_ids[tgt_idx]}"
            if matched_target_car_name not in matched_paths[matched_source_car_name]:
                matched_paths[matched_source_car_name][matched_target_car_name] = {
                    'path_coefficients': [],
                    'delays': [],
                    'is_los': []
                }

            try:
                coeff = path_coefficients[rx_idx, 0, tx_idx, 0, :]
                delay = delays[rx_idx, tx_idx, :]
                valid_mask = valid[rx_idx, tx_idx, :].astype(bool)
                interaction_types = interactions[:, rx_idx, tx_idx, :]  # shape: (5, 12)
                interaction_types_masked = interaction_types[:, valid_mask]  # shape: (5, <=12)
                is_los = np.any(interaction_types_masked == 0)

                matched_paths[matched_source_car_name][matched_target_car_name]['path_coefficients'].append(coeff)
                matched_paths[matched_source_car_name][matched_target_car_name]['delays'].append(delay)
                matched_paths[matched_source_car_name][matched_target_car_name]['is_los'].append(bool(is_los))

            except Exception as e:
                print(f"Error encountered for source {tx_idx}, target {rx_idx}: {e}")
                continue
    print(f"Matching took: {(time.time() - t) * 1000} ms")
    return matched_paths

def compute_rays(sionna_structure):
    t = time.time()

    sionna_structure["scene"].tx_array = sionna_structure["planar_array"]
    sionna_structure["scene"].rx_array = sionna_structure["planar_array"]

    # Ensure every car in the simulation has antennas (one for TX and one for RX)
    for car_id in sionna_structure["sionna_location_db"]:
        tx_antenna_name = f"car_{car_id}_tx_antenna"
        rx_antenna_name = f"car_{car_id}_rx_antenna"
        car_position = np.array(
            [sionna_structure["sionna_location_db"][car_id]['x'], sionna_structure["sionna_location_db"][car_id]['y'],
             sionna_structure["sionna_location_db"][car_id]['z']])
        tx_position = car_position + np.array(sionna_structure["antenna_displacement"])
        rx_position = car_position + np.array(sionna_structure["antenna_displacement"])

        if sionna_structure["scene"].get(tx_antenna_name) is None:
            sionna_structure["scene"].add(Transmitter(tx_antenna_name, position=tx_position, orientation=[0, 0, 0]))
            sionna_structure["scene"].tx_array = sionna_structure["scene"].tx_array
            if sionna_structure["verbose"]:
                print(f"Added TX antenna for car_{car_id}: {tx_antenna_name}")

        if sionna_structure["scene"].get(rx_antenna_name) is None:
            sionna_structure["scene"].add(Receiver(rx_antenna_name, position=rx_position, orientation=[0, 0, 0]))
            sionna_structure["scene"].rx_array = sionna_structure["scene"].rx_array
            if sionna_structure["verbose"]:
                print(f"Added RX antenna for car_{car_id}: {rx_antenna_name}")

    # Compute paths
    paths = sionna_structure["path_solver"](scene=sionna_structure["scene"],
                                            max_depth=sionna_structure["max_depth"],
                                            los=sionna_structure["los"],
                                            specular_reflection=sionna_structure["specular_reflection"],
                                            diffuse_reflection=sionna_structure["diffuse_reflection"],
                                            refraction=sionna_structure["refraction"],
                                            synthetic_array=sionna_structure["synthetic_array"],
                                            seed=sionna_structure["seed"])
    paths.normalize_delays = False

    sionna_structure["paths"] = paths

    if sionna_structure["time_checker"]:
        print(f"Ray tracing took: {(time.time() - t) * 1000} ms")
    
    t = time.time()
    matched_paths = match_rays_to_cars(paths, sionna_structure)
    if sionna_structure["time_checker"]:
        print(f"Matching rays to cars took: {(time.time() - t) * 1000} ms")

    # Iterate over sources in matched_paths
    for src_car_id in sionna_structure["sionna_location_db"]:
        current_source_car_name = f"car_{src_car_id}"
        if current_source_car_name in matched_paths:
            matched_paths_for_source = matched_paths[current_source_car_name]

            # Iterate over targets for the current source
            for trg_car_id in sionna_structure["sionna_location_db"]:
                current_target_car_name = f"car_{trg_car_id}"
                if current_target_car_name != current_source_car_name:  # Skip case where source == target
                    if current_target_car_name in matched_paths_for_source:
                        if current_source_car_name not in sionna_structure["rays_cache"]:
                            sionna_structure["rays_cache"][current_source_car_name] = {}
                        # Cache the matched paths for this source-target pair
                        sionna_structure["rays_cache"][current_source_car_name][current_target_car_name] = \
                            matched_paths_for_source[current_target_car_name]
                        if sionna_structure["verbose"]:
                            print(
                                f"Cached paths for source {current_source_car_name} to target {current_target_car_name}")
                    else:
                        # Force an update if the source or target wasn't matched
                        for car_id in sionna_structure["sionna_location_db"]:
                            car_name = f"car_{car_id}"
                            if sionna_structure["scene"].get(car_name):
                                from_sionna = sionna_structure["scene"].get(car_name)
                                new_position = [sionna_structure["SUMO_live_location_db"][car_id]["x"],
                                                sionna_structure["SUMO_live_location_db"][car_id]["y"],
                                                sionna_structure["SUMO_live_location_db"][car_id]["z"]]
                                from_sionna.position = new_position
                                # Update Sionna location database with new positions
                                sionna_structure["sionna_location_db"][car_id] = {"x": new_position[0],
                                                                                  "y": new_position[1],
                                                                                  "z": new_position[2], "angle":
                                                                                      sionna_structure[
                                                                                          "SUMO_live_location_db"][
                                                                                          car_id]["angle"]}
                                # Update antenna positions
                                if sionna_structure["scene"].get(f"{car_name}_tx_antenna"):
                                    sionna_structure["scene"].get(f"{car_name}_tx_antenna").position = \
                                        [new_position[0] + sionna_structure["antenna_displacement"][0],
                                         new_position[1] + sionna_structure["antenna_displacement"][1],
                                         new_position[2] + sionna_structure["antenna_displacement"][2]]
                                    if sionna_structure["verbose"]:
                                        print(f"Forced update for {car_name} and its TX antenna in the scene.")
                                if sionna_structure["scene"].get(f"{car_name}_rx_antenna"):
                                    sionna_structure["scene"].get(f"{car_name}_rx_antenna").position = \
                                        [new_position[0] + sionna_structure["antenna_displacement"][0],
                                         new_position[1] + sionna_structure["antenna_displacement"][1],
                                         new_position[2] + sionna_structure["antenna_displacement"][2]]
                                    if sionna_structure["verbose"]:
                                        print(f"Forced update for {car_name} and its RX antenna in the scene.")
                            else:
                                print(f"ERROR: no {car_name} in the scene for forced update, use Blender to check")

                        # Re-do matching with updated locations
                        t = time.time()
                        matched_paths = match_rays_to_cars(paths, sionna_structure)
                        if sionna_structure["time_checker"]:
                            print(f"Matching rays to cars (double exec) took: {(time.time() - t) * 1000} ms")
                        if current_source_car_name not in sionna_structure["rays_cache"]:
                            sionna_structure["rays_cache"][current_source_car_name] = {}
                        if current_target_car_name in matched_paths[current_source_car_name]:
                            sionna_structure["rays_cache"][current_source_car_name][current_target_car_name] = \
                                matched_paths[current_source_car_name][current_target_car_name]

    return None

def get_path_loss(car1_id, car2_id, sionna_structure):
    t = time.time()
    # Was the requested value already calculated?
    if car1_id not in sionna_structure["rays_cache"] or car2_id not in sionna_structure["rays_cache"][car1_id]:
        if sionna_structure["verbose"]:
            print(f"Pathloss calculation requested for {car1_id}-{car2_id}: rays not computed yet.")
        compute_rays(sionna_structure)
    
    if sionna_structure["verbose"]:
        print(f"Pathloss calculation requested for {car1_id}-{car2_id}: rays retreived from cache.")
    
    path_coefficients = sionna_structure["rays_cache"][car1_id][car2_id]["path_coefficients"]

    total_cir = 0
    if len(path_coefficients) > 0:
        # Uncoherent paths summation
        sum_coeffs = np.sum(path_coefficients)
        abs_coeffs = np.abs(sum_coeffs)
        square = abs_coeffs ** 2
        total_cir = square

    # Calculate path loss in dB
    if total_cir > 0:
        path_loss = -10 * np.log10(total_cir)
    else:
        # Handle the case where path loss calculation is not valid
        if sionna_structure["verbose"]:
            print(
                f"Pathloss calculation failed for {car1_id}-{car2_id}: got infinite value (not enough rays). Returning 300 dB.")
        path_loss = 300  # Assign 300 dB for loss cases

    if sionna_structure["time_checker"]:
        print(f"Pathloss calculation took: {(time.time() - t) * 1000} ms")
    return path_loss

def manage_path_loss_request(message, sionna_structure):
    try:
        data = message[len("CALC_REQUEST_PATHGAIN:"):]
        parts = data.split(",")
        car_a_str = parts[0].replace("veh", "")
        car_b_str = parts[1].replace("veh", "")

        # Getting each car_id, the origin is marked as 0 - ns-3 marks a car at [0,0,0] when still outside the simulation
        car_a_id = "origin" if car_a_str == "0" else f"car_{int(car_a_str)}" if car_a_str else "origin"
        car_b_id = "origin" if car_b_str == "0" else f"car_{int(car_b_str)}" if car_b_str else "origin"

        if car_a_id == "origin" or car_b_id == "origin":
            # If any, ignoring path_loss requests from the origin, used for statistical calibration
            path_loss_value = 0
        else:
            t = time.time()
            path_loss_value = get_path_loss(car_a_id, car_b_id, sionna_structure)

        return path_loss_value

    except (ValueError, IndexError) as e:
        print(f"EXCEPTION - Error processing path_loss request: {e}")
        return None

def get_delay(car1_id, car2_id, sionna_structure):
    t = time.time()
    # Check and compute rays only if necessary
    if car1_id not in sionna_structure["rays_cache"] or car2_id not in sionna_structure["rays_cache"][car1_id]:
        compute_rays(sionna_structure)

    delays = np.abs(sionna_structure["rays_cache"][car1_id][car2_id]["delays"])
    delays_flat = delays.flatten()

    # Filter positive values
    positive_values = delays_flat[delays_flat >= 0]

    if positive_values.size > 0:
        min_positive_value = np.min(positive_values)
    else:
        min_positive_value = 1e5

    if sionna_structure["time_checker"]:
        print(f"Delay calculation took: {(time.time() - t) * 1000} ms")
    return min_positive_value

def manage_delay_request(message, sionna_structure):
    try:
        data = message[len("CALC_REQUEST_DELAY:"):]
        parts = data.split(",")
        car_a_str = parts[0].replace("veh", "")
        car_b_str = parts[1].replace("veh", "")

        # Getting each car_id, the origin is marked as 0
        car_a_id = "origin" if car_a_str == "0" else f"car_{int(car_a_str)}" if car_a_str else "origin"
        car_b_id = "origin" if car_b_str == "0" else f"car_{int(car_b_str)}" if car_b_str else "origin"

        if car_a_id == "origin" or car_b_id == "origin":
            # If any, ignoring path_loss requests from the origin, used for statistical calibration
            delay = 0
        else:
            delay = get_delay(car_a_id, car_b_id, sionna_structure)

        return delay

    except (ValueError, IndexError) as e:
        print(f"EXCEPTION - Error processing delay request: {e}")
        return None

def manage_los_request(message, sionna_structure):
    t = time.time()
    try:
        data = message[len("CALC_REQUEST_LOS:"):]
        parts = data.split(",")
        car_a_str = parts[0].replace("veh", "")
        car_b_str = parts[1].replace("veh", "")

        # Getting each car_id, the origin is marked as 0
        car_a_id = "origin" if car_a_str == "0" else f"car_{int(car_a_str)}" if car_a_str else "origin"
        car_b_id = "origin" if car_b_str == "0" else f"car_{int(car_b_str)}" if car_b_str else "origin"

        if car_a_id == "origin" or car_b_id == "origin":
            # If any, ignoring path_loss requests from the origin, used for statistical calibration
            los = 0
        else:
            los = sionna_structure["rays_cache"][car_a_id][car_b_id]["is_los"]

        if sionna_structure["time_checker"]:
            print(f"LOS calculation took: {(time.time() - t) * 1000} ms")
        return los

    except (ValueError, IndexError) as e:
        print(f"EXCEPTION - Error processing LOS request: {e}")
        return None

# Function to kill processes using a specific port
def kill_process_using_port(port, verbose=False):
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

# Configure GPU settings
def configure_gpu(verbose=False, gpus=0):
    if os.getenv("CUDA_VISIBLE_DEVICES") is None:
        gpu_num = gpus
        os.environ["CUDA_VISIBLE_DEVICES"] = f"{gpu_num}"
    os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'

    gpus = tf.config.list_physical_devices('GPU')
    if gpus:
        try:
            for gpu in gpus:
                tf.config.experimental.set_memory_growth(gpu, True)
        except RuntimeError as e:
            print(e)

    tf.get_logger().setLevel('ERROR')
    if verbose:
        print("Configured TensorFlow and GPU settings.")


# Main function to manage initialization and variables
def main():
    # Argument parser setup
    parser = argparse.ArgumentParser(description='ns3-rt - Sionna Server Script: use the following options to configure the server. To apply more specific changes, edit the script directly.')
    # Scenario
    parser.add_argument('--path-to-xml-scenario', type=str, default='scenarios/SionnaCircleScenario/scene.xml',
                        help='Path to the .xml file of the scenario (see Sionna documentation for the creation of custom scenarios)')
    parser.add_argument('--frequency', type=float, help='Frequency of the simulation in Hz', default=5.89e9)
    parser.add_argument('--bw', type=float, help='Bandwidth of the simulation in Hz', default=10e6)
    # Integration
    parser.add_argument('--local-machine', action='store_true',
                        help='Flag to indicate if Sionna and ns3-rt are running on the same machine (locally)')
    parser.add_argument('--port', type=int, help='Port for the UDP socket', default=8103)
    # Ray tracing
    parser.add_argument('--position-threshold', type=float, help='Position threshold for ray tracing', default=3)
    parser.add_argument('--angle-threshold', type=float, help='Angle threshold for ray tracing', default=90)
    parser.add_argument('--max-depth', type=int, help='Maximum depth for ray tracing', default=5)
    parser.add_argument('--max-num-paths-per-src', type=int, help='Maximum number of paths per source', default=1e4)
    parser.add_argument('--samples-per-src', type=int, help='Number of samples per source', default=1e4)
    parser.add_argument('--disable-los', action='store_false', help='Flag to exclude LoS paths')
    parser.add_argument('--disable-specular-reflection', action='store_false', help='Flag to exclude specular reflections')
    parser.add_argument('--disable-diffuse-reflection', action='store_false', help='Flag to exclude diffuse reflections')
    parser.add_argument('--disable-refraction', action='store_false', help='Flag to exclude refraction')
    parser.add_argument('--seed', type=int, help='Seed for random number generation', default=42)
    parser.add_argument('--disable-synthetic-array', action='store_false', help='Flag to disable synthetic array approximation')
    # Other
    parser.add_argument('--verbose', action='store_true', help='[DEBUG] Flag for verbose output')
    parser.add_argument('--time-checker', action='store_true', help='[DEBUG] Flag to check time taken for each operation')
    parser.add_argument('--gpu', type=int, help='Number of GPUs, set 0 to use CPU only (refer to TensorFlow and Sionna documentation)', default=2)
    parser.add_argument('--dynamic-objects-name', type=str, help='Name of the dynamic objects; in the Scenario they must be called e.g., car_id, with id=SUMO ID (only number)', default="car")

    args = parser.parse_args()
    # Scenario
    file_name = args.path_to_xml_scenario
    frequency = args.frequency
    bandwidth = args.bw
    # Integration
    local_machine = args.local_machine
    port = args.port
    # Ray tracing
    position_threshold = args.position_threshold
    angle_threshold = args.angle_threshold
    max_depth = args.max_depth
    max_num_paths_per_src = args.max_num_paths_per_src
    samples_per_src = args.samples_per_src
    los = args.disable_los
    specular_reflection = args.disable_specular_reflection
    diffuse_reflection = args.disable_diffuse_reflection
    refraction = args.disable_refraction
    seed = args.seed
    syntetic_array = args.disable_synthetic_array
    # Other
    verbose = args.verbose
    time_checker = args.time_checker
    gpus = args.gpu
    dynamic_objects_name = args.dynamic_objects_name

    kill_process_using_port(port, verbose)
    configure_gpu(verbose, gpus)

    sionna_structure = dict()

    sionna_structure["verbose"] = verbose
    sionna_structure["time_checker"] = time_checker

    # Load scene and configure radio settings
    sionna_structure["scene"] = load_scene(filename=file_name, merge_shapes_exclude_regex=dynamic_objects_name)
    sionna_structure["scene"].frequency = frequency
    sionna_structure["scene"].bandwidth = bandwidth
    
    # Edit here the settings for the antennas
    element_spacing = 2.5
    sionna_structure["planar_array"] = PlanarArray(num_rows=1, num_cols=1, vertical_spacing=element_spacing, horizontal_spacing=element_spacing, pattern="iso", polarization="V")
    sionna_structure["antenna_displacement"] = [0, 0, 1.5] # Antenna position wrt car position. Edit needed if each car uses a different mesh
    
    # Scenario update frequency settings
    sionna_structure["position_threshold"] = position_threshold
    sionna_structure["angle_threshold"] = angle_threshold
    
    # Ray tracing settings
    sionna_structure["path_solver"] = PathSolver()
    sionna_structure["synthetic_array"] = syntetic_array
    sionna_structure["max_depth"] = max_depth
    sionna_structure["max_num_paths_per_src"] = max_num_paths_per_src
    sionna_structure["samples_per_src"] = samples_per_src
    sionna_structure["los"] = los
    sionna_structure["specular_reflection"] = specular_reflection
    sionna_structure["diffuse_reflection"] = diffuse_reflection
    sionna_structure["refraction"] = refraction
    sionna_structure["seed"] = seed

    # Caches - do not edit
    sionna_structure["path_loss_cache"] = {}
    sionna_structure["delay_cache"] = {}
    sionna_structure["last_path_loss_requested"] = None

    # Set up UDP socket
    udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    if local_machine:
        udp_socket.bind(("127.0.0.1", port))  # Local machine configuration
        if verbose:
            print(f"Expecting UDP messages from ns3-rt on localhost:{port}")
    else:
        udp_socket.bind(("0.0.0.0", port))  # External server configuration
        if verbose:
            print(f"Expecting UDP messages from ns3-rt on UDP/{port}")

    # Location databases and caches
    sionna_structure["SUMO_live_location_db"] = {}  # Real-time vehicle locations in SUMO
    sionna_structure["sionna_location_db"] = {}  # Vehicle locations in Sionna
    sionna_structure["rays_cache"] = {}  # Cache for ray information
    sionna_structure["path_loss_cache"] = {}  # Cache for path loss values

    print(f"Setup complete. Working at {frequency / 1e9} GHz, bandwidth {bandwidth / 1e6} MHz.")

    while True:
        # Receive data from the socket
        payload, address = udp_socket.recvfrom(1024)
        message = payload.decode()

        if verbose:
            print(f"Got new message: {message}")

        if message.startswith("LOC_UPDATE:"):
            updated_car = manage_location_message(message, sionna_structure)
            if updated_car is not None:
                response = "LOC_CONFIRM:" + "veh" + str(updated_car)
                udp_socket.sendto(response.encode(), address)

        if message.startswith("CALC_REQUEST_PATHGAIN:"):
            pathloss = manage_path_loss_request(message, sionna_structure)
            if pathloss is not None:
                response = "CALC_DONE_PATHGAIN:" + str(pathloss)
                udp_socket.sendto(response.encode(), address)

        if message.startswith("CALC_REQUEST_DELAY:"):
            delay = manage_delay_request(message, sionna_structure)
            if delay is not None:
                response = "CALC_DONE_DELAY:" + str(delay)
                udp_socket.sendto(response.encode(), address)

        if message.startswith("CALC_REQUEST_LOS:"):
            los = manage_los_request(message, sionna_structure)
            if los is not None:
                response = "CALC_DONE_LOS:" + str(los)
                udp_socket.sendto(response.encode(), address)

        if message.startswith("SHUTDOWN_SIONNA"):
            print("Got SHUTDOWN_SIONNA message. Bye!")
            udp_socket.close()
            break


# Entry point
if __name__ == "__main__":
    main()
