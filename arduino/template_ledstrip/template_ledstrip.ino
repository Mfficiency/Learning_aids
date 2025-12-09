#include <Adafruit_NeoPixel.h>
// Include your MPU library here
// #include <Wire.h>
// #include <MPU6050.h>   // Or MPU9250, etc

// ------------------------------------------------------
// CONFIGURATION VARIABLES - STUDENTS CAN TUNE THESE
// ------------------------------------------------------

// LED strip
#define LED_PIN        2
#define LED_COUNT      60      // 60 LEDs total
#define FRONT_LEDS     30      // 0..29  in the front
#define BACK_LEDS      30      // 30..59 in the back
#define LED_BRIGHTNESS 40      // 0..255

// Onboard LED
const int INT_LED_PIN = 8;

// Button input, button wired to GND
const int BUTTON_PIN = 0;

// LED built inside the button
const int BUTTON_LED_PIN = 7;

// Buzzer pin
const int BUZZER_PIN = 9;

// Button timing (in milliseconds)
const unsigned long DEBOUNCE_MS      = 30;
const unsigned long LONG_PRESS_MS    = 800;
const unsigned long DOUBLE_CLICK_MS  = 400;

// Acceleration threshold for "high acceleration" alarm
const float ACCEL_THRESHOLD_HIGH = 2.0;  // g units, students can change

// ------------------------------------------------------
// GLOBAL OBJECTS
// ------------------------------------------------------

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_RGBW + NEO_KHZ800);

// Example: different color palettes
uint32_t currentColor = 0;     // students can implement color cycling

// Power and mode
bool isOn = false;

// Modes for the strip
enum Mode {
  MODE_SPIRIT_LEVEL,
  MODE_COMPASS,
  MODE_ACCELERATION
};
Mode currentMode = MODE_SPIRIT_LEVEL;

// Button state for single, double and long press
int lastButtonReading = HIGH;       // raw reading from pin
int stableButtonState = HIGH;       // debounced state

unsigned long lastDebounceTime = 0;
unsigned long buttonDownTime = 0;
unsigned long lastClickTime = 0;
bool buttonIsPressed = false;
bool clickPending = false;

// Button LED state
bool buttonLedState = false;

// ------------------------------------------------------
// FORWARD DECLARATIONS OF PLACEHOLDER FUNCTIONS
// ------------------------------------------------------

// Students fill these with MPU code
void setupMPU();
void readMPU();                      // read sensor and store in globals
float getPitch();                    // return pitch angle
float getRoll();                     // return roll angle
float getHeading();                  // return compass heading (0..360)
float getAccelMagnitude();           // return acceleration magnitude

// Display functions
void showSpiritLevel();              // use pitch and roll
void showCompass();                  // use heading and show "north"
void showAcceleration();             // optional visualisation

// Buzzer sounds
void playStartupSound();
void playPowerOffSound();
void playHighAccelSound();

// Helpers for LED control
void showAll(uint32_t color);
void clearStrip();
uint32_t nextColor(uint32_t current); // double press switches colors

// Button handling
void updateButton();
void onSingleClick();               // toggle function / mode
void onDoubleClick();               // switch color palette
void onLongPress();                 // power off

// High acceleration detector
void checkHighAcceleration();

// ------------------------------------------------------
// SETUP
// ------------------------------------------------------

void setup() {
  pinMode(INT_LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUTTON_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  strip.begin();
  strip.setBrightness(LED_BRIGHTNESS);
  strip.show();   // strip off

  // Serial for debugging
  Serial.begin(115200);

  // TODO: students call and implement MPU setup
  setupMPU();

  // On startup, play a sound
  playStartupSound();

  // Example: initial color
  currentColor = strip.Color(0, 0, 0, 40);   // soft white
}

// ------------------------------------------------------
// MAIN LOOP
// ------------------------------------------------------

void loop() {
  // Blinking onboard LED so we see code is alive
  digitalWrite(INT_LED_PIN, HIGH);
  delay(100);
  digitalWrite(INT_LED_PIN, LOW);
  delay(100);

  // Handle button with single, double and long press
  updateButton();

  // Read MPU values (students implement this)
  readMPU();

  // Only update LEDs if power is on
  if (isOn) {
    switch (currentMode) {
      case MODE_SPIRIT_LEVEL:
        showSpiritLevel();
        break;
      case MODE_COMPASS:
        showCompass();
        break;
      case MODE_ACCELERATION:
        showAcceleration();
        break;
    }

    // Check for high acceleration event
    checkHighAcceleration();
  } else {
    clearStrip();
  }
}

// ------------------------------------------------------
// BUTTON HANDLING
// ------------------------------------------------------

// Debounce + click type detection
void updateButton() {
  int reading = digitalRead(BUTTON_PIN);
  unsigned long now = millis();

  // Debounce logic
  if (reading != lastButtonReading) {
    lastDebounceTime = now;
  }

  if ((now - lastDebounceTime) > DEBOUNCE_MS) {
    if (reading != stableButtonState) {
      stableButtonState = reading;

      // Button just went down (HIGH to LOW, since pullup)
      if (stableButtonState == LOW) {
        buttonDownTime = now;
        buttonIsPressed = true;
      }
      // Button just released (LOW to HIGH)
      else {
        buttonIsPressed = false;
        unsigned long pressDuration = now - buttonDownTime;

        if (pressDuration >= LONG_PRESS_MS) {
          onLongPress();
        } else {
          // Short press: possible single or double click
          if (clickPending && (now - lastClickTime) < DOUBLE_CLICK_MS) {
            // Double click detected
            clickPending = false;
            onDoubleClick();
          } else {
            // First click, wait to see if double click happens
            clickPending = true;
            lastClickTime = now;
          }
        }
      }
    }
  }

  // If we had a click and the double click timeout expired, treat as single
  if (clickPending && (now - lastClickTime) >= DOUBLE_CLICK_MS) {
    clickPending = false;
    onSingleClick();
  }

  lastButtonReading = reading;
}

// ------------------------------------------------------
// BUTTON CALLBACKS - STUDENTS FILL THESE
// ------------------------------------------------------

void onSingleClick() {
  // TODO: toggle between different functions or modes
  // For example:
  // currentMode = (Mode)((currentMode + 1) % 3);

  // Toggle button LED as visual feedback
  buttonLedState = !buttonLedState;
  digitalWrite(BUTTON_LED_PIN, buttonLedState ? HIGH : LOW);

  Serial.println("Single click");
}

void onDoubleClick() {
  // TODO: implement color switching
  // Example:
  // currentColor = nextColor(currentColor);

  Serial.println("Double click");
}

void onLongPress() {
  // TODO: implement power off behavior
  // isOn = false;
  // playPowerOffSound();
  // clearStrip();

  Serial.println("Long press");
}

// ------------------------------------------------------
// MPU PLACEHOLDERS - STUDENTS IMPLEMENT
// ------------------------------------------------------

void setupMPU() {
  // TODO: setup MPU (Wire.begin, sensor init, etc)
  // Example:
  // Wire.begin();
  // mpu.initialize();
  Serial.println("MPU setup placeholder");
}

void readMPU() {
  // TODO: read sensor data and store in global variables
  // for pitch, roll, heading, acceleration, etc.
}

float getPitch() {
  // TODO: calculate and return pitch angle in degrees
  return 0.0;
}

float getRoll() {
  // TODO: calculate and return roll angle in degrees
  return 0.0;
}

float getHeading() {
  // TODO: calculate and return compass heading (0..360)
  return 0.0;
}

float getAccelMagnitude() {
  // TODO: calculate and return acceleration magnitude in g units
  return 0.0;
}

// ------------------------------------------------------
// DISPLAY FUNCTIONS - STUDENTS IMPLEMENT THE LOGIC
// ------------------------------------------------------

void showSpiritLevel() {
  // TODO: use pitch and roll to decide which LEDs turn on
  // Example idea:
  // float pitch = getPitch();
  // float roll  = getRoll();
  // Map these angles to LED positions on front and back.

  strip.clear();

  // Placeholder: fill front with currentColor
  for (int i = 0; i < FRONT_LEDS; i++) {
    strip.setPixelColor(i, currentColor);
  }

  strip.show();
}

void showCompass() {
  // TODO: use heading to light up the LED that points north
  // Example idea:
  // float heading = getHeading();           // 0..360
  // int ledIndex = map(heading, 0, 360, 0, LED_COUNT);
  // strip.clear();
  // strip.setPixelColor(ledIndex, currentColor);

  strip.clear();

  // Placeholder: light a single LED in the front
  strip.setPixelColor(0, currentColor);

  strip.show();
}

void showAcceleration() {
  // TODO: visualise acceleration (for example brightness or number of lit LEDs)
  float accel = getAccelMagnitude();

  strip.clear();

  // Placeholder: simple bar graph using accel value
  int numLeds = map((int)(accel * 10), 0, 30, 0, LED_COUNT);
  if (numLeds < 0) numLeds = 0;
  if (numLeds > LED_COUNT) numLeds = LED_COUNT;

  for (int i = 0; i < numLeds; i++) {
    strip.setPixelColor(i, currentColor);
  }

  strip.show();
}

// ------------------------------------------------------
// HIGH ACCELERATION CHECK
// ------------------------------------------------------

void checkHighAcceleration() {
  float accel = getAccelMagnitude();

  if (accel >= ACCEL_THRESHOLD_HIGH) {
    // TODO: maybe flash LEDs as well
    playHighAccelSound();
    Serial.println("High acceleration detected");
  }
}

// ------------------------------------------------------
// BUZZER SOUND PLACEHOLDER FUNCTIONS
// ------------------------------------------------------

void playStartupSound() {
  // TODO: design a nice startup tune
  // Example:
  // tone(BUZZER_PIN, 1000, 100);
  // delay(150);
  // tone(BUZZER_PIN, 1500, 100);
  // delay(150);
  // noTone(BUZZER_PIN);
}

void playPowerOffSound() {
  // TODO: design a power off sound
  // Example:
  // tone(BUZZER_PIN, 500, 200);
  // delay(200);
  // noTone(BUZZER_PIN);
}

void playHighAccelSound() {
  // TODO: design a warning sound for high acceleration
  // Example:
  // tone(BUZZER_PIN, 2000, 100);
  // delay(100);
  // noTone(BUZZER_PIN);
}

// ------------------------------------------------------
// LED HELPERS
// ------------------------------------------------------

void showAll(uint32_t color) {
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, color);
  }
  strip.show();
}

void clearStr
