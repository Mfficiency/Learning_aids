#include <Adafruit_NeoPixel.h>
#include <math.h>
// Include your MPU library here
// #include <Wire.h>
// #include <MPU6050.h>

// ------------------------------------------------------
// CONFIG VARIABLES
// ------------------------------------------------------

#define LED_PIN        2
#define LED_COUNT      60        // 60 LEDs total
#define FRONT_LEDS     30        // 0..29 front side
#define BACK_LEDS      30        // 30..59 back side
#define LED_BRIGHTNESS 80        // 0..255

const int BUTTON_PIN      = 0;   // push button to GND
const int BUTTON_LED_PIN  = 7;   // LED in the button (optional)
const int INT_LED_PIN     = 8;   // onboard LED (heartbeat)

// debounce in ms
const unsigned long DEBOUNCE_MS = 40;

// ------------------------------------------------------
// GLOBALS
// ------------------------------------------------------

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_RGBW + NEO_KHZ800);

// flame color palette
const int NUM_COLORS = 4;
uint32_t flameColors[NUM_COLORS];

int currentColorIndex = 0;

// button state
int lastButtonState = HIGH;
int stableButtonState = HIGH;
unsigned long lastDebounceTime = 0;

// simple gyro variable (students should update this from MPU)
float tiltValue = 0.0;   // for example from -90 to +90 degrees

// ------------------------------------------------------
// FORWARD DECLARATIONS
// ------------------------------------------------------

void setupMPU();
void readMPU();         // students fill in and set tiltValue

float getTiltValue();

void updateButton();
void onButtonPressed();

void updateFlame();
uint32_t scaleColor(uint32_t color, float factor);

// ------------------------------------------------------
// SETUP
// ------------------------------------------------------

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUTTON_LED_PIN, OUTPUT);
  pinMode(INT_LED_PIN, OUTPUT);

  strip.begin();
  strip.setBrightness(LED_BRIGHTNESS);
  strip.show();

  Serial.begin(115200);
  delay(500);
  Serial.println("Simple gyro flame test");

  // default flame palette
  // mainly blue, then other colors to cycle through
  flameColors[0] = strip.Color(0, 0, 80, 40);  // blue flame
  flameColors[1] = strip.Color(80, 0, 0, 40);  // red flame
  flameColors[2] = strip.Color(0, 80, 0, 40);  // green flame
  flameColors[3] = strip.Color(80, 20, 0, 40); // orange flame

  currentColorIndex = 0;

  // MPU placeholder
  setupMPU();

  // seed random for flicker
  randomSeed(analogRead(0));
}

// ------------------------------------------------------
// MAIN LOOP
// ------------------------------------------------------

void loop() {
  // heartbeat LED
  digitalWrite(INT_LED_PIN, HIGH);
  delay(50);
  digitalWrite(INT_LED_PIN, LOW);

  updateButton();
  readMPU();       // students fill in
  updateFlame();   // draw the flame effect

  delay(30);       // small delay for animation
}

// ------------------------------------------------------
// BUTTON HANDLING
// ------------------------------------------------------

void updateButton() {
  int reading = digitalRead(BUTTON_PIN);
  unsigned long now = millis();

  if (reading != lastButtonState) {
    lastDebounceTime = now;
  }

  if ((now - lastDebounceTime) > DEBOUNCE_MS) {
    if (reading != stableButtonState) {
      stableButtonState = reading;

      // detect press (HIGH to LOW)
      if (stableButtonState == LOW) {
        onButtonPressed();
      }
    }
  }

  lastButtonState = reading;
}

void onButtonPressed() {
  // cycle color index
  currentColorIndex = (currentColorIndex + 1) % NUM_COLORS;

  // show button LED as simple feedback
  digitalWrite(BUTTON_LED_PIN, currentColorIndex % 2 ? HIGH : LOW);

  Serial.print("Button pressed, color index = ");
  Serial.println(currentColorIndex);
}

// ------------------------------------------------------
// MPU PLACEHOLDERS
// ------------------------------------------------------

void setupMPU() {
  // TODO: students setup MPU here
  // Example:
  // Wire.begin();
  // mpu.initialize();
  Serial.println("MPU setup placeholder");
}

void readMPU() {
  // TODO: students read gyro or accelerometer and set tiltValue
  // For example:
  // tiltValue = someAngleInDegrees;  // between -90 and +90

  // placeholder: simple slow sweep for testing without sensor
  static unsigned long startTime = millis();
  float t = (millis() - startTime) / 1000.0;
  tiltValue = 45.0 * sin(t);  // fake tilt between -45 and +45
}

float getTiltValue() {
  return tiltValue;
}

// ------------------------------------------------------
// FLAME EFFECT
// ------------------------------------------------------

// reacts to tiltValue and draws a "flame" from base (0 and 59) to tip (29 and 30)
void updateFlame() {
  float tilt = getTiltValue();           // expected range about -90..+90
  tilt = constrain(tilt, -90.0, 90.0);

  // map absolute tilt to flame height
  // straight up: high flame
  // flat: low flame
  int minHeight = 5;                     // never completely off
  int maxHeight = FRONT_LEDS;           // 30
  int flameHeight = map((int)abs(tilt), 0, 90, minHeight, maxHeight);

  if (flameHeight < minHeight) flameHeight = minHeight;
  if (flameHeight > FRONT_LEDS) flameHeight = FRONT_LEDS;

  uint32_t baseColor = flameColors[currentColorIndex];

  strip.clear();

  // positionFromBase goes from 0 to 29
  // front index increases from base 0 to tip 29
  // back index decreases from base 59 to tip 30
  for (int pos = 0; pos < FRONT_LEDS; pos++) {
    int frontIndex = pos;
    int backIndex  = LED_COUNT - 1 - pos;  // 59..30

    // if outside current flame height, turn off
    if (pos >= flameHeight) {
      strip.setPixelColor(frontIndex, 0);
      strip.setPixelColor(backIndex, 0);
      continue;
    }

    // brightness factor, brighter near the base
    float baseFactor = (float)(flameHeight - pos) / (float)flameHeight;

    // add some random flicker
    float flicker = random(-10, 15) / 100.0;  // -0.10 to +0.15
    float factor = baseFactor + flicker;
    if (factor < 0.0) factor = 0.0;
    if (factor > 1.0) factor = 1.0;

    uint32_t c = scaleColor(baseColor, factor);
    strip.setPixelColor(frontIndex, c);
    strip.setPixelColor(backIndex, c);
  }

  strip.show();
}

// scale RGBW color by factor 0..1
uint32_t scaleColor(uint32_t color, float factor) {
  if (factor < 0.0) factor = 0.0;
  if (factor > 1.0) factor = 1.0;

  uint8_t r = (color >> 16) & 0xFF;
  uint8_t g = (color >> 8)  & 0xFF;
  uint8_t b =  color        & 0xFF;
  uint8_t w = (color >> 24) & 0xFF;

  r = (uint8_t)(r * factor);
  g = (uint8_t)(g * factor);
  b = (uint8_t)(b * factor);
  w = (uint8_t)(w * factor);

  return Adafruit_NeoPixel::Color(r, g, b, w);
}
