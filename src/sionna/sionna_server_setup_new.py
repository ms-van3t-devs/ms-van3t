import os
import signal
import time
import socket
import subprocess
import argparse
import numpy as np
import tensorflow as tf
from sionna.rt import load_scene, PlanarArray, Transmitter, Receiver, Paths
from sionna.constants import SPEED_OF_LIGHT


# --- Argument Parsing ---
def parse_arguments():
    parser = argparse.ArgumentParser(description="Sionna Server Script")
    parser.add_argument("--filename", type=str, required=True, help="Path to the scenario file")
    parser.add_argument("--local-machine", action="store_true", help="Run on the local machine")
    parser.add_argument("--verbose", action="store_true", help="Enable verbose output")
    return parser.parse_args()


# --- Utility Functions ---
def setup_environment(gpu_num=2):
    """Configures the TensorFlow environment."""
    if os.getenv("CUDA_VISIBLE_DEVICES") is None:
        os.environ["CUDA_VISIBLE_DEVICES"] = str(gpu_num)
    os.environ["TF_CPP_MIN_LOG_LEVEL"] = "3"
    gpus = tf.config.list_physical_devices("GPU")
    for gpu in gpus:
        try:
            tf.config.experimental.set_memory_growth(gpu, True)
        except RuntimeError as e:
            print(f"GPU setup error: {e}")


def kill_process_using_port(port, verbose=False):
    """Kills any process using the specified port."""
    try:
        result = subprocess.run(["lsof", "-i", f":{port}"], stdout=subprocess.PIPE, text=True)
        for line in result.stdout.splitlines():
            if "LISTEN" in line:
                pid = int(line.split()[1])
                os.kill(pid, signal.SIGKILL)
                if verbose:
                    print(f"Killed process {pid} using port {port}")
    except Exception as e:
        print(f"Error killing process on port {port}: {e}")


def create_udp_socket(local_machine, port=8103):
    """Creates and binds a UDP socket."""
    udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    address = "127.0.0.1" if local_machine else "0.0.0.0"
    udp_socket.bind((address, port))
    return udp_socket


def verbose_log(message, verbose):
    """Logs a message if verbosity is enabled."""
    if verbose:
        print(message)


# --- Scene Setup ---
def setup_scene(file_name, frequency=5.89e9):
    """Loads and configures the Sionna scene."""
    scene = load_scene(file_name)
    scene.frequency = frequency
    scene.synthetic_array = True  # Use a synthetic array for faster computations
    return scene


def configure_antenna(scene, element_spacing, antenna_displacement):
    """Configures antenna settings for the scene."""
    array = PlanarArray(1, 1, element_spacing, element_spacing, "iso", "V")
    return array


# --- Simulation Logic ---
def update_car_positions(scene, car_id, new_position, antenna_displacement, verbose=False):
    """Updates the position of a car and its antennas in the scene."""
    tx_antenna_name = f"car_{car_id}_tx_antenna"
    rx_antenna_name = f"car_{car_id}_rx_antenna"

    if scene.get(tx_antenna_name) is None:
        scene.add(Transmitter(tx_antenna_name, position=new_position + antenna_displacement))
        verbose_log(f"Added TX antenna for car_{car_id}: {tx_antenna_name}", verbose)

    if scene.get(rx_antenna_name) is None:
        scene.add(Receiver(rx_antenna_name, position=new_position + antenna_displacement))
        verbose_log(f"Added RX antenna for car_{car_id}: {rx_antenna_name}", verbose)


def manage_location_message(scene, message, live_db, sionna_db, position_threshold, angle_threshold, verbose=False):
    """Processes a location update message."""
    try:
        car, new_x, new_y, new_z, new_angle = parse_location_message(message)
        new_position = np.array([new_x, new_y, new_z])
        live_db[car] = {"position": new_position, "angle": new_angle}

        if car in sionna_db:
            old_position = sionna_db[car]["position"]
            position_changed = np.linalg.norm(new_position - old_position) >= position_threshold
            angle_changed = abs(new_angle - sionna_db[car]["angle"]) >= angle_threshold
        else:
            position_changed = angle_changed = True

        if position_changed or angle_changed:
            sionna_db[car] = live_db[car]
            if scene.get(f"car_{car}"):
                scene.get(f"car_{car}").position = new_position
                verbose_log(f"Updated car_{car} position in the scene.", verbose)
            else:
                print(f"Error: car_{car} not found in the scene.")
    except Exception as e:
        print(f"Error managing location message: {e}")


def parse_location_message(message):
    """Parses a location message and extracts relevant data."""
    data = message[len("map_update:"):]
    parts = data.split(",")
    car = int(parts[0].replace("veh", ""))
    x, y, z, angle = map(float, parts[1:])
    return car, x, y, z, angle


def compute_rays(scene, sionna_db, max_depth, num_samples, antenna_displacement, verbose=False):
    """Computes and matches rays between cars."""
    t_start = time.time()
    paths = scene.compute_paths(max_depth=max_depth, num_samples=num_samples, diffraction=True, scattering=True)
    verbose_log(f"Ray tracing completed in {(time.time() - t_start) * 1000:.2f} ms", verbose)

    # Match rays to cars (implementation details skipped here)
    matched_paths = match_rays_to_cars(paths, sionna_db, position_threshold=3,
                                       antenna_displacement=antenna_displacement)
    return matched_paths


# --- Main Execution ---
if __name__ == "__main__":
    args = parse_arguments()

    # Environment setup
    setup_environment()
    kill_process_using_port(8103, args.verbose)

    # Scene and antenna configuration
    element_spacing = SPEED_OF_LIGHT / (5.89e9) / 2
    antenna_displacement = np.array([0, 0, 1.5])
    scene = setup_scene(args.filename)
    array = configure_antenna(scene, element_spacing, antenna_displacement)

    # Simulation state
    live_db = {}
    sionna_db = {}
    rays_cache = {}

    # Socket setup
    udp_socket = create_udp_socket(args.local_machine)

    # Start processing messages (example)
    while True:
        try:
            message, _ = udp_socket.recvfrom(1024)
            message = message.decode("utf-8")
            if message.startswith("map_update:"):
                manage_location_message(scene, message, live_db, sionna_db, position_threshold=3, angle_threshold=90,
                                        verbose=args.verbose)
            elif message.startswith("calc_request:"):
                # Handle pathloss calculation request
                pass
        except KeyboardInterrupt:
            print("Exiting...")
            break
