import json
import pandas as pd
from decoded_messages import DecodedMessage
from math import sin, cos, radians

SPEED_THRESHOLD = 15  # m/s
AGE_THRESHOLD = 20  # ms


def filter_by_start_time(data, start_time):
    start_time_micseconds = start_time
    assert start_time_micseconds < data[-1]["timestamp"], "The start time is greater than the last timestamp in the file"
    return list(filter(lambda x: x["timestamp"] >= start_time_micseconds, data))


if __name__ == "__main__":
    filename = "/home/diego/ms-van3t-devs/ms-van3t/ns-3-dev/src/gps-tc/examples/GPS-Traces-Sample/A4_andata.json"
    start_time = None
    decoder = DecodedMessage()
    f = open(filename, "r")
    data = json.load(f)
    f.close()

    df = pd.DataFrame(columns=["agent_id", "timeStamp_posix", "latitude_deg", "longitude_deg", "speed_ms", "heading_deg", "accel_ms2"])

    if start_time:
        data = filter_by_start_time(data, start_time)

    last_update_pos = None
    agent_id = 1
    i = 0
    last_time = 0
    lat = None
    lon = None
    heading = None
    speed = None
    last_speed = None
    acc = None
    for d in data:
        delta_time = (d["timestamp"] - last_time) / 1e6
        last_time = d["timestamp"]
        message_type = d["type"]
        if message_type == "Unknown":
            continue
        content = d["data"]
        if message_type == "UBX":
            content = bytes.fromhex(content)
        else:
            # For the NMEA messages we need to encode the content for the serial emulator and decode it for the GUI (to obtain a string)
            content = content.encode()
        tmp_lat, tmp_lon, tmp_heading, tmp_speed = None, None, None, None
        if message_type == "UBX":
            tmp_lat, tmp_lon, tmp_heading, tmp_speed = decoder.extract_data(content, message_type)
        else:
            tmp_lat, tmp_lon, tmp_heading, tmp_speed = decoder.extract_data(content.decode(), message_type)
        if tmp_lat and tmp_lon:
            last_update_pos = d["timestamp"]
        if tmp_lat:
            lat = tmp_lat
        if tmp_lon:
            lon = tmp_lon
        if tmp_heading:
            heading = tmp_heading
        if tmp_speed:
            speed = tmp_speed
            acc = (speed - last_speed) / delta_time

        if not lat and not lon and not heading and not speed:
            continue

        interpolated = False
        if (speed and speed > SPEED_THRESHOLD) and last_update_pos and (d["timestamp"] - last_update_pos) / 1e3 > AGE_THRESHOLD:
            interpolated = True
            pos_age = d["timestamp"] - last_update_pos
            pos_age /= 1e3
            interp_points = pos_age / AGE_THRESHOLD
            for j in range(1, interp_points):
                t = last_update_pos + j * (AGE_THRESHOLD * 1e3)
                delta_x = speed * t * sin(heading)
                delta_y = speed * t * cos(heading)
                new_lat = lat + (delta_y / 111320)
                new_lon = lon + (delta_x / (111320 * cos(radians(lat))))
                lat = new_lat
                lon = new_lon
                df.iloc[i] = [agent_id, t, lat, lon, speed, heading, acc]
                last_update_pos = t
                i += 1

        last_speed = speed if speed else last_speed
        if (lat and lon and heading and speed) and not interpolated:
            df.iloc[i] = [agent_id, d["timestamp"], lat, lon, speed, heading, acc]
            i += 1
