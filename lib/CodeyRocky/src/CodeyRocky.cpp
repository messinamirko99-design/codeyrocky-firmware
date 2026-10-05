#include "CodeyRocky.h"

CodeyRocky codey;

void CodeyRocky::begin(bool powerHold) {
  if (powerHold) {
    pinMode(CR_PIN_POWER_HOLD, OUTPUT);
    digitalWrite(CR_PIN_POWER_HOLD, HIGH);
  }
  pinMode(CR_PIN_POWER_KEY, INPUT);
  analogSetPinAttenuation(CR_PIN_HW_VER, ADC_11db);

  display.begin();
  buttonA.begin(CR_PIN_BTN_A, true);
  buttonB.begin(CR_PIN_BTN_B, true);
  buttonC.begin(CR_PIN_BTN_C, false);
  led.begin();
  light.begin(CR_PIN_LIGHT);
  sound.begin(CR_PIN_SOUND);
  knob.begin(CR_PIN_KNOB);
  battery.begin();
  gyro.begin();
  speaker.begin();
  ir.begin();
  rocky.begin();
}

void CodeyRocky::update() {
  buttonA.update();
  buttonB.update();
  buttonC.update();
  gyro.update();
  speaker.update();
  ir.update();
  rocky.update();

  // Hold power button ~2s to shut down, matching stock firmware.
  static uint32_t holdStart = 0;
  if (digitalRead(CR_PIN_POWER_KEY) == LOW) {
    if (!holdStart) holdStart = millis();
    else if (millis() - holdStart > 2000) {
      powerOff();
    }
  } else {
    holdStart = 0;
  }
}

void CodeyRocky::powerOff() {
  display.clear();
  led.off();
  rocky.stop();
  speaker.stop();
  digitalWrite(CR_PIN_POWER_HOLD, LOW);
  while (true) delay(1000);
}

int CodeyRocky::hardwareVersion() {
  return analogRead(CR_PIN_HW_VER);
}
