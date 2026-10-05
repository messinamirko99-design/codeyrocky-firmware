#include <CodeyRocky.h>

void setup() {
  Serial.begin(115200);
  codey.begin();
  codey.rocky.setLight(20, 20, 20);
}

void loop() {
  codey.update();
  if (!codey.rocky.isConnected()) return;

  bool wall = codey.rocky.isObstacleAhead();
  int grey = codey.rocky.grey();
  int ir = codey.rocky.irReflect();
  CrColor c = codey.rocky.color();

  Serial.printf("obs=%d grey=%d ir=%d color=%d rgb=%d,%d,%d\n",
                wall, grey, ir, (int)c,
                codey.rocky.red(), codey.rocky.green(), codey.rocky.blue());

  if (wall) {
    codey.display.showText("WALL");
    codey.led.setRGB(80, 0, 0);
    codey.rocky.setLight(80, 0, 0);
  } else if (codey.rocky.isColor(COLOR_RED)) {
    codey.display.showText("RED");
    codey.led.setRGB(80, 0, 0);
  } else if (codey.rocky.isColor(COLOR_GREEN)) {
    codey.display.showText("GRN");
    codey.led.setRGB(0, 80, 0);
  } else if (codey.rocky.isColor(COLOR_BLUE)) {
    codey.display.showText("BLU");
    codey.led.setRGB(0, 0, 80);
  } else {
    codey.display.showNumber(grey);
    codey.led.setRGB(0, 20, 20);
    codey.rocky.setLight(0, 20, 20);
  }
  delay(60);
}
