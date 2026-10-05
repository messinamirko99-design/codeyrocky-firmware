#include <CodeyRocky.h>

void setup() {
  codey.begin();
  codey.display.showText("ROCKY");
}

void loop() {
  codey.update();
  if (!codey.rocky.isConnected()) {
    codey.display.showText("NO BS");
    delay(200);
    return;
  }

  if (codey.buttonA.wasPressed()) {
    codey.display.showText("FWD");
    codey.rocky.forward(50, 1000);
  }
  if (codey.buttonB.wasPressed()) {
    codey.display.showText("TURN");
    codey.rocky.turnLeft(45, 600);
    codey.rocky.turnRight(45, 600);
  }
  if (codey.buttonC.wasPressed()) {
    codey.rocky.stop();
    codey.display.showText("STOP");
  }

  // Differential drive with the gear knob.
  int k = codey.knob.read();
  int left = map(k, 0, 100, -60, 60);
  if (codey.buttonA.isPressed()) {
    codey.rocky.drive(left, 40);
  }
}
