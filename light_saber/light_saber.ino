#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// === PIN DEFINITIONS ===
const int INT_LED_PIN = 8;       // onboard LED
const int BUTTON_PIN = 0;        // button switch to GND
const int BUTTON_LED_PIN = 7;    // LED in button
const int BUZZER_PIN = 10;       // buzzer
#define LED_PIN   2              // SK6812 data pin
#define LED_COUNT 40

// === LED STRIP ===
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_RGBW + NEO_KHZ800);

// === MPU6050 ===
Adafruit_MPU6050 mpu;

// === STATE VARIABLES ===
bool isOn = false;
bool buttonLedState = false;
int lastButtonState = HIGH;

// Motion detection thresholds
const float MOTION_THRESHOLD = 15.0;  // m/s² (adjust sensitivity)
unsigned long lastMotionTime = 0;
const unsigned long MOTION_COOLDOWN = 500;  // ms between motion triggers

// === SETUP ===
void setup() {
  Serial.begin(115200);
  
  // GPIO setup
  pinMode(INT_LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUTTON_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  
  // LED strip
  strip.begin();
  strip.show();
  
  // I2C for MPU6050 (SDA=GPIO4, SCL=GPIO5)
  Wire.begin(4, 5);
  
  // Initialize MPU6050
  if (!mpu.begin()) {
    Serial.println("MPU6050 not found!");
    // Flash onboard LED rapidly to indicate error
    for (int i = 0; i < 10; i++) {
      digitalWrite(INT_LED_PIN, HIGH);
      delay(100);
      digitalWrite(INT_LED_PIN, LOW);
      delay(100);
    }
  } else {
    Serial.println("MPU6050 ready!");
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  }
}

// === MAIN LOOP ===
void loop() {
  // --- Onboard LED blink ---
  digitalWrite(INT_LED_PIN, HIGH);
  delay(200);
  digitalWrite(INT_LED_PIN, LOW);
  delay(200);
  
  // --- Button handling ---
  int buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == LOW && lastButtonState == HIGH) {
    handleButtonPress();
    delay(200);  // debounce
  }
  lastButtonState = buttonState;
  
  // --- Motion detection (only when strip is ON) ---
  if (isOn) {
    checkMotion();
  }
}

// === BUTTON PRESS HANDLER ===
void handleButtonPress() {
  // Toggle button LED
  buttonLedState = !buttonLedState;
  digitalWrite(BUTTON_LED_PIN, buttonLedState ? HIGH : LOW);
  
  // Toggle strip
  isOn = !isOn;
  if (isOn) {
    strip.fill(strip.Color(0, 0, 0, 15));  // soft white
    strip.show();
    beep(100);  // short beep on activation
  } else {
    strip.clear();
    strip.show();
    beep(50);   // shorter beep on deactivation
  }
}

// === MOTION DETECTION ===
void checkMotion() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  
  // Calculate total acceleration magnitude
  float totalAccel = sqrt(a.acceleration.x * a.acceleration.x +
                          a.acceleration.y * a.acceleration.y +
                          a.acceleration.z * a.acceleration.z);
  
  // Remove gravity (9.8 m/s²)
  float motion = abs(totalAccel - 9.8);
  
  // Check if motion exceeds threshold
  if (motion > MOTION_THRESHOLD && 
      (millis() - lastMotionTime) > MOTION_COOLDOWN) {
    
    Serial.print("Motion detected: ");
    Serial.println(motion);
    
    // Visual feedback: flash rainbow
    rainbowPulse();
    
    // Audio feedback
    beep(200);
    
    lastMotionTime = millis();
  }
}

// === LED EFFECTS ===
void rainbowPulse() {
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, strip.ColorHSV(i * 21845, 255, 100));
  }
  strip.show();
  delay(300);
  strip.fill(strip.Color(0, 0, 0, 15));  // back to white
  strip.show();
}

// === BUZZER FUNCTIONS ===
void beep(int duration) {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(duration);
  digitalWrite(BUZZER_PIN, LOW);
}

// For passive buzzer (tone generation):
void playTone(int frequency, int duration) {
  tone(BUZZER_PIN, frequency, duration);
  delay(duration);
  noTone(BUZZER_PIN);
}