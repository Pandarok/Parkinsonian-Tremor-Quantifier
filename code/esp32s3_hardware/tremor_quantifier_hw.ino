#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <arduinoFFT.h>   // v2.x

// ---------- OLED Display ----------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ---------- ESP32-S3 Pin Definitions ----------
#define SDA_PIN 8
#define SCL_PIN 9
#define RED_LED_PIN 4     // Tremor detected (4.0 - 6.0 Hz)
#define GREEN_LED_PIN 5   // Normal state (outside 4.0 - 6.0 Hz)

// ---------- IMU Sensor (MPU-6050 / MPU-6500 / Clones) ----------
#define MPU_ADDR 0x68
#define REG_CONFIG       0x1A
#define REG_ACCEL_CONFIG 0x1C
#define REG_ACCEL_CFG2   0x1D
#define REG_ACCEL_XOUT_H 0x3B
#define REG_PWR_MGMT_1   0x6B
#define REG_WHO_AM_I     0x75

const float ACCEL_LSB_PER_G = 16384.0;  // ±2g range
const float G_TO_MS2 = 9.80665;

// ---------- FFT Configuration ----------
const uint16_t samples = 128;
const double samplingFrequency = 50.0;
const uint32_t samplePeriodUs = 20000;

// Noise floor threshold (m/s^2): below this value, frequency is set to 0
const double AMP_THRESHOLD = 0.15;

double vReal[samples];
double vImag[samples];
ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, samples, samplingFrequency);

double shownFreq = 0.0;  // Exponentially smoothed frequency for display

bool writeReg(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.write(val);
  return Wire.endTransmission() == 0;
}

int readReg(uint8_t reg) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return -1;
  if (Wire.requestFrom((uint8_t)MPU_ADDR, (uint8_t)1) != 1) return -1;
  return Wire.read();
}

bool readAccel(float &ax, float &ay, float &az) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(REG_ACCEL_XOUT_H);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom((uint8_t)MPU_ADDR, (uint8_t)6) != 6) return false;
  int16_t x = (Wire.read() << 8) | Wire.read();
  int16_t y = (Wire.read() << 8) | Wire.read();
  int16_t z = (Wire.read() << 8) | Wire.read();
  const float k = G_TO_MS2 / ACCEL_LSB_PER_G;
  ax = x * k; ay = y * k; az = z * k;
  return true;
}

void showMessage(const char *l1, const char *l2 = "") {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(l1);
  display.println(l2);
  display.display();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);
  Wire.setTimeOut(50);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;) delay(10);
  }
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  showMessage("Tremor Quantifier", "Init sensor...");

  if (!writeReg(REG_PWR_MGMT_1, 0x00)) {
    Serial.println("Sensor not responding at 0x68");
    showMessage("Sensor error!", "Check wiring");
    while (1) delay(10);
  }
  delay(100);

  int who = readReg(REG_WHO_AM_I);
  Serial.print("WHO_AM_I = 0x");
  Serial.println(who, HEX);

  writeReg(REG_ACCEL_CONFIG, 0x00);  // ±2g range
  writeReg(REG_CONFIG, 0x04);        // DLPF ~21 Hz
  writeReg(REG_ACCEL_CFG2, 0x04);    // DLPF for MPU-6500 compatibility

  float x, y, z;
  if (!readAccel(x, y, z)) {
    Serial.println("Accel read failed");
    showMessage("Sensor error!", "Read failed");
    while (1) delay(10);
  }

  char buf[22];
  snprintf(buf, sizeof(buf), "WHO_AM_I: 0x%02X", who);
  showMessage("Sensor OK", buf);
  delay(1500);
}

void loop() {
  // 1. Collect 128 samples at a strict 50 Hz sampling rate
  uint32_t next = micros();
  for (uint16_t i = 0; i < samples; i++) {
    while ((int32_t)(micros() - next) < 0) { }
    next += samplePeriodUs;

    float ax = 0, ay = 0, az = 0;
    readAccel(ax, ay, az);
    vReal[i] = sqrt(ax * ax + ay * ay + az * az);
    vImag[i] = 0.0;
  }

  // 2. Remove DC offset (gravity component)
  double mean = 0;
  for (uint16_t i = 0; i < samples; i++) mean += vReal[i];
  mean /= samples;
  for (uint16_t i = 0; i < samples; i++) vReal[i] -= mean;

  // 3. Apply Hamming window and compute FFT
  FFT.windowing(FFTWindow::Hamming, FFTDirection::Forward);
  FFT.compute(FFTDirection::Forward);
  FFT.complexToMagnitude();

  // 4. Find dominant frequency peak
  uint16_t k = 1;
  double peakMag = 0;
  for (uint16_t i = 1; i < samples / 2; i++) {
    if (vReal[i] > peakMag) { peakMag = vReal[i]; k = i; }
  }

  // Refine peak frequency using parabolic interpolation
  double delta = 0.0;
  if (k > 1 && k < samples / 2 - 1) {
    double a = vReal[k - 1], b = vReal[k], c = vReal[k + 1];
    double denom = a - 2.0 * b + c;
    if (denom != 0.0) delta = 0.5 * (a - c) / denom;
  }
  double rawFreq = (k + delta) * samplingFrequency / samples;
  double amplitude = 2.0 * peakMag / (samples * 0.54);  // Magnitude used for noise threshold check

  // 5. Zero out noise below threshold; otherwise apply EMA smoothing
  if (amplitude < AMP_THRESHOLD) {
    shownFreq = 0.0;
  } else if (shownFreq == 0.0) {
    shownFreq = rawFreq;
  } else {
    shownFreq = 0.6 * shownFreq + 0.4 * rawFreq;
  }

  // 6. Tremor band (4.0 - 6.0 Hz) -> Red LED, otherwise Green LED
  bool tremorDetected = (shownFreq >= 4.0 && shownFreq <= 6.0);

  digitalWrite(RED_LED_PIN, tremorDetected ? HIGH : LOW);
  digitalWrite(GREEN_LED_PIN, tremorDetected ? LOW : HIGH);

  Serial.printf("Freq: %.2f Hz (raw %.2f, amp %.3f), tremor: %d\n",
                shownFreq, rawFreq, amplitude, tremorDetected);

  // 7. Update OLED display
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Tremor Quantifier");
  display.setCursor(0, 20);
  display.print("Freq: ");
  display.print(shownFreq, 1);
  display.println(" Hz");
  display.setCursor(0, 44);
  display.print(tremorDetected ? "TREMOR DETECTED!" : "Status: Normal");
  display.display();
}
