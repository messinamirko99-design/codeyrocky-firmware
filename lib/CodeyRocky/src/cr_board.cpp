#include "CodeyRocky.h"

#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
#define CR_LEDC3 1
#else
#define CR_LEDC3 0
#endif

void CrButton::begin(uint8_t pin, bool activeHigh) {
  _pin = pin;
  _activeHigh = activeHigh;
  _pressed = _prev = _rose = _fell = false;
  _lastMs = 0;
  pinMode(_pin, INPUT);
}

void CrButton::update() {
  bool raw = digitalRead(_pin);
  bool down = _activeHigh ? raw : !raw;
  uint32_t now = millis();
  if (down != _prev && (now - _lastMs) > 25) {
    _lastMs = now;
    _prev = down;
    _pressed = down;
    if (down) _rose = true;
    else _fell = true;
  } else {
    _pressed = _prev;
  }
}

bool CrButton::wasPressed() {
  bool v = _rose;
  _rose = false;
  return v;
}

bool CrButton::wasReleased() {
  bool v = _fell;
  _fell = false;
  return v;
}

void CrRgb::begin() {
#if CR_LEDC3
  ledcAttach(CR_PIN_LED_R, 5000, 8);
  ledcAttach(CR_PIN_LED_G, 5000, 8);
  ledcAttach(CR_PIN_LED_B, 5000, 8);
#else
  ledcSetup(0, 5000, 8);
  ledcSetup(1, 5000, 8);
  ledcSetup(2, 5000, 8);
  ledcAttachPin(CR_PIN_LED_R, 0);
  ledcAttachPin(CR_PIN_LED_G, 1);
  ledcAttachPin(CR_PIN_LED_B, 2);
#endif
  off();
}

void CrRgb::_write(uint8_t pin, uint8_t value) {
#if CR_LEDC3
  ledcWrite(pin, value);
#else
  uint8_t ch = (pin == CR_PIN_LED_R) ? 0 : (pin == CR_PIN_LED_G) ? 1 : 2;
  ledcWrite(ch, value);
#endif
}

void CrRgb::setRGB(uint8_t r, uint8_t g, uint8_t b) {
  // Stock firmware inverts the red channel (GPIO14). Green/blue are normal.
  _write(CR_PIN_LED_R, (uint8_t)(255 - r));
  _write(CR_PIN_LED_G, g);
  _write(CR_PIN_LED_B, b);
}

void CrRgb::setColor(uint32_t rgb) {
  setRGB((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
}

void CrRgb::off() { setRGB(0, 0, 0); }

void CrAnalog::begin(uint8_t pin) {
  _pin = pin;
  analogReadResolution(12);
  analogSetPinAttenuation(_pin, ADC_11db);
}

int CrAnalog::raw() { return analogRead(_pin); }

int CrAnalog::read() {
  int v = map(raw(), 0, 4095, 0, 100);
  if (v < 0) v = 0;
  if (v > 100) v = 100;
  return v;
}

void CrBattery::begin() {
  analogReadResolution(12);
  analogSetPinAttenuation(CR_PIN_BATTERY, ADC_11db);
}

float CrBattery::voltage() {
  // From firmware: ADC_VALUE_TO_VOL = 0.001934  ((1/4096)*1.1*3.6*2)
  return analogRead(CR_PIN_BATTERY) * 0.001934f;
}

int CrBattery::percent() {
  float v = voltage();
  if (v <= 3.40f) return 0;
  if (v >= 4.15f) return 100;
  return (int)((v - 3.40f) / (4.15f - 3.40f) * 100.0f);
}
