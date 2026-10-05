#include <CodeyRocky.h>

void setup() {
  codey.begin();
}

void loop() {
  codey.update();
  const CrFace faces[] = {FACE_HAPPY, FACE_CRY, FACE_SAD, FACE_ANGRY, FACE_FEAR, FACE_HEART, FACE_ARROW};
  for (CrFace f : faces) {
    codey.display.setFace(f);
    delay(700);
  }
  codey.display.showNumber(42);
  delay(800);
  codey.display.scrollText("ABCDEFGHIJKLMNOPQRSTUVWXYZ 0123456789", 55);
}
