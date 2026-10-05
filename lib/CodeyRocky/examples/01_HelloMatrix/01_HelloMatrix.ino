#include <CodeyRocky.h>

void setup() {
  codey.begin();
  codey.display.setBrightness(8);
  codey.display.showText("HELLO");
  delay(1200);
  codey.display.scrollText("CODEY ROCKY", 70);
  codey.display.setFace(FACE_HAPPY);
}

void loop() {
  codey.update();
}
