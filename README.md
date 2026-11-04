# Parkinsonian Tremor Quantifier

An embedded system prototype designed to quantify parkinsonian tremors using an **ESP32** microcontroller, an **MPU-6050** accelerometer, and an **OLED display**, powered by **Fast Fourier Transform (FFT)** signal processing.

---

## 📌 Project Overview
Parkinsonian resting tremor typically occurs within the **4–6 Hz** frequency range. This project provides a portable, cost-effective quantification tool that samples acceleration data at a strict rate of **50 Hz**, applies a **Hamming window**, and performs **FFT analysis** to isolate and measure peak tremor frequencies in real time.

### Key Features:
* **Precise Sampling:** Timed data collection at 50 Hz using microsecond intervals (`micros()`).
* **Digital Signal Processing (DSP):** Hamming windowing to reduce spectral leakage and FFT for frequency domain analysis.
* **Visual Feedback:** Real-time display of dominant frequency and status on a 0.96" OLED screen, backed by LED indicators.
* **Virtual Prototyping:** Fully testable in the Wokwi online simulator without physical hardware constraints.

---

## 🛠️ Hardware Requirements
* ESP32 Development Board (DevKit v1)
* MPU-6050 6-axis Accelerometer & Gyroscope Module
* 0.96" SSD1306 I2C OLED Display
* LEDs (Red & Green) and resistors for status indication
* Breadboard and jumper wires

---

## 📂 Repository Structure
* `/code` — Contains the C++ source code (`.ino`) and Wokwi simulation configuration (`diagram.json`).
* `/docs` — Project proposal, analysis, and schematics.

---

## 🚀 Setup & Replication Instructions

### 1. Requirements & Libraries
Ensure you have **Arduino IDE** installed with the ESP32 board package. Install the following libraries via ZIP or Library Manager:
* `Adafruit MPU6050`
* `Adafruit SSD1306`
* `Adafruit GFX Library`
* `arduinoFFT`

### 2. Pinout Configuration (I2C)
| Component Pin | ESP32 Pin |
| :--- | :--- |
| **SDA** (OLED & MPU6050) | GPIO 21 |
| **SCL** (OLED & MPU6050) | GPIO 22 |
| **VCC** | 3.3V |
| **GND** | GND |

### 3. Simulation (Wokwi)
You can test and run this project virtually using [Wokwi](https://wokwi.com/):
1. Create a new ESP32 project.
2. Replace `sketch.ino` with the code from the `/code` folder.
3. Replace `diagram.json` with the layout file provided in this repository.
4. Click **Play** to start the simulation.

---

## 📜 License
This project is licensed under the terms of the [MIT License](LICENSE).
