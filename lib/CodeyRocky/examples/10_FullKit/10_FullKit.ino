#include <CodeyRocky.h>

void setup() {
  Serial.begin(115200);
  codey.begin();
  codey.speaker.setVolume(35);
  codey.display.scrollText("CODEY ROCKY", 60);
  codey.display.setFace(FACE_HAPPY);
}

void loop() {
  codey.update();

  if (codey.gyro.isShaking()) {
    codey.display.playAnimation(FACE_FEAR, 1);
    codey.speaker.tone(150, 80);
  }

  if (codey.gyro.isTilted(TILT_LEFT)) codey.rocky.turnLeft(30);
  else if (codey.gyro.isTilted(TILT_RIGHT)) codey.rocky.turnRight(30);
  else if (codey.buttonA.isPressed()) codey.rocky.forward(45);
  else if (codey.buttonB.isPressed()) codey.rocky.backward(45);
  else codey.rocky.stop();

  if (codey.rocky.isConnected() && codey.rocky.isObstacleAhead()) {
    codey.rocky.stop();
    codey.display.setFace(FACE_ANGRY);
  }

  int light = codey.light.read();
  codey.led.setRGB(0, light, 40);
  delay(25);
}
