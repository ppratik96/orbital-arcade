# Orbital Arcade: Rotatetris & RoSnake

[![Board](https://img.shields.io/badge/Board-Waveshare%20ESP32--S3%20AMOLED%201.8%22-cyan.svg)](https://www.waveshare.com/esp32-s3-touch-amoled-1.8.htm)
[![Framework](https://img.shields.io/badge/Platform-App--Pixels-orange.svg)](https://app-pixels.com)
[![Storage](https://img.shields.io/badge/Storage-Flash%20NVS%20(No%20SD%20Required)-blue.svg)](#)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

An arcade suite designed specifically for the **Waveshare ESP32-S3 Touch AMOLED 1.8"** watch board, featuring **gyro-steered motion vectoring** and **touch controls**.

---

## 🎮 Included Games

### 🧊 1. ROTATETRIS
A 4-way rotating gravity puzzle game:
* **4-Way Multi-Well Arena**: 4 playable gravity wells with $3 \times 3$ corner geometry and 7-block wide arcade wells.
* **$3 \times 3$ Solid Central Spawn Core**: Pieces emerge outward from a central cyber chamber that also serves as the visible overflow ceiling.
* **World Rotation (90° Tilt Vectoring)**: Tilt the physical board left/right/up/down to shift the gravitational pull of the entire arena.
* **Grounded Lock Delay**: 600ms grace period with move-reset allows sliding and tucking pieces under deep overhangs.
* **3 Difficulty Tiers**: Easy (900ms), Medium (650ms), Hard (400ms).
* **Cinematic 3, 2, 1 Launch Sequence**: Digital countdown inside the core chamber before spawning.

### 🐍 2. ROSNAKE
A gyro-steered arcade snake game:
* **Real-Time Gyro Steering**: Tilt or rotate the watch in 3D space to steer your snake (Tilt Down = Down, Tilt Up = Up, Rotate Left = Left, Rotate Right = Right).
* **Anti-Reverse Protection**: 180° turns are automatically blocked to prevent accidental self-collision.
* **Touch Assist Gestures**: Swipe across the touchscreen as an alternate steering option.
* **Energy Orbs & Bonus Cores**: Standard gold energy orbs and pulsing magenta bonus cores for high score multipliers.
* **Matching $13 \times 13$ 26px Grid**: Aligned with Rotatetris for visual consistency.

---

## 🕹️ Controls

| Control | Rotatetris | RoSnake |
| :--- | :--- | :--- |
| **Tilt / Rotate Watch** | Shifts gravity well (Down, Left, Up, Right) | Steers snake heading (Down, Left, Up, Right) |
| **Touch Drag / Swipe** | Moves piece laterally / soft drops | Swipes to steer |
| **Physical BOOT Button (GPIO 0)** | Instant Hard Drop | Restart / Menu shortcut |
| **Touch Tap** | Navigates menus & difficulties | Navigates menus & difficulties |

---

## ⚙️ Hardware Specifications & Pinout

| Component | Interface / Pins | Details |
| :--- | :--- | :--- |
| **MCU** | ESP32-S3 Dual Core @ 240MHz | 16MB Flash (QIO), 8MB PSRAM (OPI) |
| **AMOLED Display** | QSPI (CS:12, SCK:11, D0:4, D1:5, D2:6, D3:7) | $368 \times 448$, CO5300 / SH8601 Driver |
| **Touch Sensor** | I2C (SDA:8, SCL:9, INT:21, ADDR:0x15/0x38) | CST820 / FT3168 Capacitive Touch |
| **IMU (Gyro/Accel)** | I2C (SDA:8, SCL:9, ADDR:0x6B) | QMI8658 6-Axis Motion Tracking |
| **Power Management**| I2C (SDA:8, SCL:9, ADDR:0x34) | AXP2101 PMU + TCA9554 IO Expander (0x20) |
| **Storage** | Internal Flash NVS via ESP32 `Preferences` | Persistent High Scores across reboots (No SD needed) |

---

## 🚀 Building & Flashing

### Option A: App-Pixels (Direct Web Flashing)
1. Connect your Waveshare ESP32-S3 AMOLED 1.8" to your computer via USB-C.
2. Open [App-Pixels](https://app-pixels.com) in Google Chrome or Microsoft Edge.
3. Select **Orbital Arcade** and click **Install / Flash**.

### Option B: Arduino IDE
1. Install **ESP32 by Espressif Systems** (v3.0.0 or later) in the Board Manager.
2. Select Board: **ESP32S3 Dev Module**.
3. Settings:
   - **Flash Size**: 16MB (128Mb)
   - **Flash Mode**: QIO 80MHz
   - **PSRAM**: OPI PSRAM
   - **USB CDC On Boot**: Enabled
   - **Partition Scheme**: 16MB Flash (3MB APP / 9MB FAT)
4. Open `orbital-tetris.ino` and click **Upload**.

### Option C: PlatformIO
```bash
pio run --target upload
```

---

## 📄 License
This project is open-source under the [MIT License](LICENSE). Free for community distribution on [App-Pixels](https://app-pixels.com).
