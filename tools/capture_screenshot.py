#!/usr/bin/env python3
import sys
import os
import time
import serial
import serial.tools.list_ports

def find_esp32_port():
    ports = serial.tools.list_ports.comports()
    for p in ports:
        if "usbmodem" in p.device or "USB" in p.description:
            return p.device
    return "/dev/cu.usbmodem101"

def capture(output_filename="screenshot.bmp", port=None):
    if not port:
        port = find_esp32_port()
    
    print(f"Connecting to ESP32 on {port}...")
    try:
        ser = serial.Serial(port, 115200, timeout=4.0)
    except Exception as e:
        print(f"Error opening port {port}: {e}")
        return False

    time.sleep(0.5)
    ser.reset_input_buffer()
    
    print("Requesting on-device screenshot...")
    ser.write(b"SCREENSHOT\n")
    ser.flush()

    # Look for start marker
    start_time = time.time()
    found_start = False
    while time.time() - start_time < 3.0:
        line = ser.readline()
        if b"===SCREENSHOT_START===" in line:
            found_start = True
            break

    if not found_start:
        print("Failed to receive screenshot start marker.")
        ser.close()
        return False

    print("Receiving BMP stream from ESP32...")
    bmp_size = 54 + (368 * 448 * 3) # 494646 bytes
    raw_data = bytearray()
    
    while len(raw_data) < bmp_size:
        chunk = ser.read(min(4096, bmp_size - len(raw_data)))
        if not chunk:
            break
        raw_data.extend(chunk)

    ser.close()

    if len(raw_data) < bmp_size:
        print(f"Incomplete screenshot data: received {len(raw_data)}/{bmp_size} bytes")
        return False

    os.makedirs(os.path.dirname(os.path.abspath(output_filename)), exist_ok=True)
    with open(output_filename, "wb") as f:
        f.write(raw_data)

    print(f"✅ On-device screenshot saved successfully to: {output_filename} ({len(raw_data)} bytes)")
    return True

if __name__ == "__main__":
    out_file = sys.argv[1] if len(sys.argv) > 1 else "assets/screenshot.bmp"
    capture(out_file)
