# Circuit Schematic & Wiring Diagram

## Connection Table

| Component | Pin | ESP32 Board | Cable Color | Note |
| :--- | :--- | :--- | :--- | :--- |
| **OLED SSD1306** | VCC | 3V3 | Red | Power |
| **OLED SSD1306** | GND | GND | Black | Ground |
| **OLED SSD1306** | SCL | GPIO 22 | Yellow | I2C Clock |
| **OLED SSD1306** | SDA | GPIO 21 | Blue | I2C Data |
| **MPU-6050** | VCC | 3V3 | Red | Power |
| **MPU-6050** | GND | GND | Black | Ground |
| **MPU-6050** | SCL | GPIO 22 | Yellow | I2C Clock |
| **MPU-6050** | SDA | GPIO 21 | Blue | I2C Data |
| **Green LED** | Anode (+) | GPIO 25 | Green | Normal Status |
| **Red LED** | Anode (+) | GPIO 26 | Red | Tremor Warning |

---

## I2C Topology
Both the **OLED SSD1306** and **MPU-6050** share the primary I2C bus:
* **SCL:** GPIO 22
* **SDA:** GPIO 21
