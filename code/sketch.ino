#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_SSD1306.h>
#include <arduinoFFT.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Adafruit_MPU6050 mpu;


const uint16_t samples = 128; 

const double samplingFrequency = 50.0; 

double vReal[samples];
double vImag[samples];


ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, samples, samplingFrequency);

const int GREEN_LED_PIN = 25; 
const int RED_LED_PIN = 26;   

void setup() {
  Serial.begin(115200);

  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.print("System Initializing...");
  display.display();

  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    display.setCursor(0, 20);
    display.print("MPU6050 Error!");
    display.display();
    while (1) { delay(10); }
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  delay(1000);
  display.clearDisplay();
}

void loop() {

  for (int i = 0; i < samples; i++) {
    unsigned long microseconds = micros();
    
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    vReal[i] = a.acceleration.z;
    vImag[i] = 0.0; 
    
    
    while (micros() - microseconds < 20000) {

    }
  }

  FFT.windowing(vReal, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD); 
  FFT.compute(vReal, vImag, samples, FFT_FORWARD); 
  FFT.complexToMagnitude(vReal, vImag, samples); 
  
  double peakFrequency = FFT.majorPeak(vReal, samples, samplingFrequency);

  
  bool tremorDetected = false;
  if (peakFrequency >= 4.0 && peakFrequency <= 6.0) {
    tremorDetected = true;
  }


  if (tremorDetected) {
    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(GREEN_LED_PIN, LOW);
  } else {
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, HIGH);
  }


  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Tremor Quantifier");
  
  display.setCursor(0, 20);
  display.print("Freq: ");
  display.print(peakFrequency, 1);
  display.println(" Hz");

  display.setCursor(0, 40);
  if (tremorDetected) {
    display.print("Status: TREMOR DETECTED");
  } else {
    display.print("Status: Normal");
  }
  
  display.display();
}
