# Hardware Schematics & Wiring Guide

This document details the electrical connections, Bill of Materials (BOM), and pin mappings for the **Parkinsonian Tremor Quantifier** physical prototype, as well as the pin mapping differences for the Wokwi simulation environment.

---

## 1. Bill of Materials (BOM)

| Component | Specification | Quantity | Notes |
| :--- | :--- | :---: | :--- |
| **Microcontroller** | ESP32-S3-WROOM-1 DevKit | 1 | Dual-core Xtensa LX7, 3.3V logic |
| **IMU Sensor** | MPU-6050 / MPU-6500 (GY-521) | 1 | 3-axis accelerometer & gyroscope, I2C (`0x68`) |
| **Display** | 0.96" SSD1306 OLED (128×64) | 1 | Monochrome I2C display (`0x3C`) |
| **Indicator LEDs** | 5mm Red & 5mm Green LEDs | 2 | Visual status indicators (Tremor vs. Normal) |
| **Resistors** | 220 Ω (1/4 W) | 2 | Current-limiting resistors for LEDs |
| **Prototyping** | Breadboard & Jumper Wires | 1 set | Male-to-Male / Male-to-Female Dupont wires |
| **Wearable Mount** | Nylon Velcro Wrist Strap | 1 | Secures the IMU module to the wrist/hand |

---

## 2. Physical Prototype Wiring (ESP32-S3-WROOM-1)

Both the **MPU-6050/6500** sensor and the **SSD1306 OLED** display share a single hardware I2C bus configured on **GPIO 8 (SDA)** and **GPIO 9 (SCL)** operating at `100 kHz`.

### Pinout Table

| Peripheral | Module Pin | ESP32-S3 Pin | Electrical / Functional Notes |
| :--- | :--- | :--- | :--- |
| **MPU-6050 / 6500** | `VCC` | `3V3` | 3.3V regulated power supply |
| | `GND` | `GND` | Common system ground |
| | `SDA` | `GPIO 8` | Shared I2C Data line |
| | `SCL` | `GPIO 9` | Shared I2C Clock line |
| | `AD0` | `GND` *(or floating)* | Sets I2C device address to `0x68` |
| **SSD1306 OLED** | `VCC` | `3V3` | 3.3V regulated power supply |
| | `GND` | `GND` | Common system ground |
| | `SDA` | `GPIO 8` | Shared I2C Data line |
| | `SCL` / `SCK` | `GPIO 9` | Shared I2C Clock line (`0x3C`) |
| **Red LED** | Anode (`+`) | `GPIO 4` | Connected via **220 Ω** resistor (Active on `4.0–6.0 Hz`) |
| | Cathode (`-`) | `GND` | Common system ground |
| **Green LED** | Anode (`+`) | `GPIO 5` | Connected via **220 Ω** resistor (Active in Normal state) |
| | Cathode (`-`) | `GND` | Common system ground |

---

### Circuit Block Diagram

```text
                       +-----------------------+
                       |   ESP32-S3-WROOM-1    |
                       |                       |
   +------------------>| 3V3               GND |<------------------+
   |                   |                       |                   |
   |   +-------------->| GPIO 8 (SDA)          |                   |
   |   |   +---------->| GPIO 9 (SCL)          |                   |
   |   |   |           |                       |                   |
   |   |   |           | GPIO 4         GPIO 5 |                   |
   |   |   |           +---+---------------+---+                   |
   |   |   |               |               |                       |
   |   |   |            [220 Ω]         [220 Ω]                    |
   |   |   |               |               |                       |
   |   |   |            (Red LED)     (Green LED)                  |
   |   |   |               |               |                       |
   |   |   |               +-------+-------+-----------------------+
   |   |   |                                                       |
   |   |   +-----------------------+-----------------------+       |
   |   |                           |                       |       |
   |   +-------------------+-------+---------------+       |       |
   |                       |       |               |       |       |
   |                   +---+---+---+---+       +---+---+---+---+   |
   +------------------>|VCC|SDA|SCL|GND|       |VCC|SDA|SCL|GND|<--+
                       +---------------+       +---------------+
                       | MPU-6050/6500 |       | SSD1306 OLED  |
                       | Addr: 0x68    |       | Addr: 0x3C    |
                       +---------------+       +---------------+
```

---

## 3. Design & Electrical Considerations

* **Shared I2C Bus Integrity:** Both I2C breakout boards (GY-521 and SSD1306) include built-in `4.7 kΩ`–`10 kΩ` pull-up resistors to `3.3V`. Running the bus at `100 kHz` ensures signal integrity over breadboard jumper wires without parasitic capacitance distortion.
* **LED Current Limiting:** The `220 Ω` series resistors limit GPIO output current to approximately `(3.3V - 2.0V) / 220 Ω ≈ 6 mA`, protecting the ESP32-S3 I/O drivers while maintaining clear daylight visibility.
* **Mechanical Isolation:** When mounting the IMU onto the wrist strap, the underside solder joints are insulated with Kapton/electrical tape to prevent skin contact or short circuits across the woven nylon fabric.

---

## 4. Wokwi Simulation Pin Mapping Reference

For virtual testing in `code/wokwi_simulation/`, a standard ESP32 board profile is used with default I2C pins:

| Signal | Physical Hardware (`ESP32-S3`) | Wokwi Simulation (`ESP32`) |
| :--- | :--- | :--- |
| **I2C SDA** | `GPIO 8` | `GPIO 21` |
| **I2C SCL** | `GPIO 9` | `GPIO 22` |
| **Red LED** | `GPIO 4` | `GPIO 26` |
| **Green LED** | `GPIO 5` | `GPIO 25` |
