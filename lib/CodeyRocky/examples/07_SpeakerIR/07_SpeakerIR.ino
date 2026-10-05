#include <CodeyRocky.h>

void setup() {
  codey.begin();
  codey.speaker.setVolume(50);
  codey.speaker.setTempo(90);
  codey.display.showText("MUSIC");
  codey.speaker.note("C4", 0.5);
  codey.speaker.note("E4", 0.5);
  codey.speaker.note("G4", 0.5);
  codey.speaker.note("C5", 1);
}

void loop() {
  codey.update();

  if (codey.buttonA.wasPressed()) {
    codey.ir.sendNEC(0x00FF, 0x1A);   // generic TV-style NEC
    codey.display.showText("IR TX");
    codey.speaker.tone(2000, 60);
  }

  if (codey.buttonB.wasPressed()) {
    codey.ir.sendText("HI");
    codey.display.showText("HI");
  }

  if (codey.ir.available()) {
    codey.display.showNumber(codey.ir.lastCommand());
    codey.speaker.tone(1200, 40);
  }
}
