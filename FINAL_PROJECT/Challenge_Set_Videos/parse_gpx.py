import csv
import xml.etree.ElementTree as ET
from datetime import datetime
import math
import matplotlib.pyplot as plt
import numpy as np

# Data syncing
VIDEO_START_OFFSET = 33.0
VIDEO_DURATION = 60.0

# I/O
NAME = "Nature_Walk"
INPUT_FILE = f"FINAL_PROJECT/Challenge_Set_Videos/{NAME}/{NAME}.gpx"
OUTPUT_FILE = f"FINAL_PROJECT/Challenge_Set_Videos/{NAME}/{NAME}.csv"
IMG_FILE = f"FINAL_PROJECT/Challenge_Set_Videos/{NAME}/{NAME}.jpg"

NS = "http://www.topografix.com/GPX/1/1"

def parse_time(s): # GPX uses ISO 8601 UTC timestamp
    s = s.strip().replace("Z", "+00:00")
    return datetime.fromisoformat(s)

def haversine_meters(lat1, lon1, lat2, lon2): # Flat earth approx, returns (x, y) in meters
    R = 6_371_000  # Earth radius (m)
    d_lat = math.radians(lat2 - lat1)
    d_lon = math.radians(lon2 - lon1)
    avg_lat = math.radians((lat1 + lat2) / 2)
    x = d_lon * R * math.cos(avg_lat) # east/west
    y = d_lat * R # north/south
    return x, y

def smooth(data, window=5): # moving avg
    kernel = np.ones(window) / window
    padded = np.pad(data, window // 2, mode="edge")
    return np.convolve(padded, kernel, mode="valid")[:len(data)]

def main():
    tree = ET.parse(INPUT_FILE)
    root = tree.getroot()

    # get all trackpoints from file
    points = []
    for trkpt in root.findall(f".//{{{NS}}}trkpt"):
        lat = float(trkpt.attrib["lat"])
        lon = float(trkpt.attrib["lon"])
        time_el = trkpt.find(f"{{{NS}}}time")
        if time_el is None:
            continue
        t = parse_time(time_el.text)
        points.append((t, lat, lon))

    print(f"Found {len(points)} trackpoints.")

    # Compute elapsed seconds and cut to the video window
    t0 = points[0][0]
    timed = [((t - t0).total_seconds(), lat, lon) for t, lat, lon in points]
    print(f"Time range: {timed[0][0]:.1f}s - {timed[-1][0]:.1f}s")
    window_start = VIDEO_START_OFFSET
    window_end = VIDEO_START_OFFSET + VIDEO_DURATION
    windowed = [(s, lat, lon) for s, lat, lon in timed if window_start <= s <= window_end]

    print(f"Kept {len(windowed)} points in window [{window_start}s - {window_end}s].")

    # zero the position
    origin_lat = windowed[0][1]
    origin_lon = windowed[0][2]
    origin_t = windowed[0][0]

    x1, y1 = haversine_meters(origin_lat, origin_lon, windowed[6][1], windowed[6][2])
    initial_angle = math.atan2(x1, y1)  # angle from +Y axis
    print(f"Initial heading: {math.degrees(initial_angle):.1f} deg")

    def rotate(x, y, angle):
        cos_a = math.cos(angle)
        sin_a = math.sin(angle)
        return x * cos_a - y * sin_a, x * sin_a + y * cos_a

    times, xs, ys = [], [], []
    for s, lat, lon in windowed:
        x_m, y_m = haversine_meters(origin_lat, origin_lon, lat, lon)
        x_m, y_m = rotate(x_m, y_m, initial_angle) # rotate so starting pose is facing +Y
        t_rel = s - origin_t   # time relative to video start
        times.append(t_rel)
        xs.append(x_m)
        ys.append(y_m)

    # smooth
    xs = list(smooth(xs, window=5))
    ys = list(smooth(ys, window=5))

    with open(OUTPUT_FILE, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["time_s", "x_m", "y_m"])
        for t, x, y in zip(times, xs, ys):
            writer.writerow([f"{t:.3f}", f"{x:.3f}", f"{y:.3f}"])

    # plot real quick to verify
    plt.figure(figsize=(6, 6))
    plt.plot(xs, ys, marker="o", markersize=3, linewidth=1.5)
    plt.scatter(xs[0],  ys[0],  color="green", zorder=5, s=80, label="Start")
    plt.scatter(xs[-1], ys[-1], color="red",   zorder=5, s=80, label="End")
    plt.xlabel("x (m)")
    plt.ylabel("y (m)")
    plt.title("GPS Ground Truth")
    plt.legend()
    plt.axis("equal")
    plt.grid(True)
    plt.tight_layout()
    plt.savefig(IMG_FILE) # save img
    plt.show()

if __name__ == "__main__":
    main()