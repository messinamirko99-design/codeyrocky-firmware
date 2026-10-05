#include <CodeyRocky.h>

// Default sketch: face + sensors on the matrix, A/B/C control Rocky.
// Copy any example from lib/CodeyRocky/examples/ over this file to try it.

void setup() {
  codey.begin();
  codey.display.setBrightness(8);
  codey.display.setFace(FACE_HAPPY);
  codey.led.setRGB(0, 40, 20);
  codey.speaker.setVolume(40);
  codey.speaker.note("C5", 0.25);
}

void loop() {
  codey.update();

  if (codey.buttonA.wasPressed()) {
    codey.display.showText("FWD");
    codey.rocky.forward(40, 800);
    codey.display.setFace(FACE_HAPPY);
  }
  if (codey.buttonB.wasPressed()) {
    codey.display.showText("BACK");
    codey.rocky.backward(40, 800);
  }
  if (codey.buttonC.wasPressed()) {
    codey.rocky.stop();
    codey.display.showNumber(codey.battery.percent());
    delay(600);
    codey.display.setFace(FACE_HEART);
  }

  if (codey.rocky.isConnected() && codey.rocky.isObstacleAhead()) {
    codey.speaker.tone(880, 80);
    codey.rocky.turnRight(50, 400);
  }

  delay(20);
}
