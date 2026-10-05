#include <CodeyRocky.h>

// IR sensor on Rocky must face DOWN. Use a black line on white floor.
// Tune THRESH with the knob (shown on the matrix).

void setup() {
  codey.begin();
  codey.display.showText("LINE");
}

void loop() {
  codey.update();
  if (!codey.rocky.isConnected()) return;

  int thresh = 15 + codey.knob.read() / 2; // 15..65
  int g = codey.rocky.grey();
  int ir = codey.rocky.irReflect();
  int mix = (g + ir) / 2;

  codey.display.showNumber(mix);

  if (mix < thresh) {
    codey.rocky.drive(20, 50);   // line under left → steer right
  } else {
    codey.rocky.drive(50, 20);
  }
  delay(20);
}
