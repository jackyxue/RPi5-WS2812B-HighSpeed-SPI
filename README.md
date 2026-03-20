# Raspberry Pi 5 WS2812B (ARGB) SPI DMA Driver

This project is a high-performance WS2812B LED driver designed specifically for the **Raspberry Pi 5**. [cite_start]By leveraging the SPI bus at **32MHz** and custom bit-encoding, it achieves the precise timing required for WS2812B LEDs while overcoming standard Linux SPI buffer limitations[cite: 1].

---

## 🛠 Hardware Connection Guide

Since the RPi5 GPIO output is 3.3V and WS2812B requires 5V, proper signal integrity is crucial at 32MHz.

| RPi5 Pin | Function | Connection | Note |
| :--- | :--- | :--- | :--- |
| **Pin 19** | SPI MOSI | LED Strip **DI** | Recommended: **33Ω** series resistor to reduce reflection. |
| **Pin 20** | Ground | LED Strip **GND** | Must share common ground with external PSU. |
| **Ext. 5V** | Power | LED Strip **5V** | **DO NOT** power large strips directly from RPi5. |

> **Pro Tip:** For strips over 120 LEDs, using a high-speed level shifter like the **74AHCT125** is highly recommended to boost the 3.3V signal to 5V.

---

## ⚙️ System Configuration

Two critical system-level changes are required for Raspberry Pi OS to support high-speed LED driving.

### 1. Enable SPI and Set Frequency
Edit the configuration file:
```bash
sudo nano /boot/firmware/config.txt
Ensure the following line is present:

Plaintext
dtparam=spi=on
2. Expand SPI Buffer Size (Critical)
At 32MHz with 5 Bytes per bit encoding, the data expands significantly (approx. 40x). Driving a large number of LEDs (e.g., 120+) will fail with "Message too long" if the buffer is not expanded.

Edit the kernel boot command line:

Bash
sudo nano /boot/firmware/cmdline.txt
Add the following parameter to the end of the line (do not create a new line; keep it as one single line of text):

Plaintext
spidev.bufsiz=65536
Note: This expands the buffer to 64KB, sufficient for approximately 500 LEDs.

Reboot to apply changes: sudo reboot

🚀 Building and Running
The project includes a Makefile for easy compilation using g++.

Compile the Program
Bash
make
Run the Demo
Accessing /dev/spidev0.0 requires root privileges:

Bash
sudo ./argb_demo
# OR use the built-in make command
make run
🌈 Integrated Light Effects
Ported from the TI MSPM0G platform, the driver includes several optimized effects:

Fire: Realistic flame simulation using heat-map algorithms.

Aurora: Flowing cyan-green pulses simulating polar lights.

Cyberpulse: High-intensity pulses for a cyberpunk aesthetic.

Soft Comet: Trailing light effect with configurable fade.

Rainbow Cycle: Smooth color cycling across the entire strip.

📊 Data Calculation Reference
Under the 32MHz / 5Bpp configuration, the memory requirement is:
Total Bytes = LED_NUM * 24 bits * 5 Bytes/bit.

10 LEDs: 1,200 Bytes (Works with default settings)

120 LEDs: 14,400 Bytes (Requires cmdline.txt fix) 
sudo nano /boot/firmware/cmdline.txt add "spidev.bufsiz=65536"

450 LEDs: 54,000 Bytes (Requires cmdline.txt fix),
sudo nano /boot/firmware/cmdline.txt add "spidev.bufsiz=65536"