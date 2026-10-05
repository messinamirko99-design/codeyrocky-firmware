#include <CodeyRocky.h>

// IR sensor on Rocky must face FORWARD.

void setup() {
  codey.begin();
  codey.display.setFace(FACE_HAPPY);
  codey.rocky.setLight(0, 30, 0);
}

void loop() {
  codey.update();
  if (!codey.rocky.isConnected()) return;

  if (codey.rocky.isObstacleAhead()) {
    codey.display.setFace(FACE_FEAR);
    codey.led.setRGB(80, 0, 0);
    codey.speaker.tone(400, 50);
    codey.rocky.backward(40, 300);
    codey.rocky.turnRight(55, 450);
  } else {
    codey.display.setFace(FACE_HAPPY);
    codey.led.setRGB(0, 40, 0);
    codey.rocky.forward(35);
  }
  delay(30);
}
