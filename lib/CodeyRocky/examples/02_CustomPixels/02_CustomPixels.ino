#include <CodeyRocky.h>

// 16 columns, 8 rows. Each byte is one column, bit0 = top row.
// Draw a checker, then a custom heart column-map.

void setup() {
  codey.begin();
  codey.display.clear();

  for (int x = 0; x < 16; x++) {
    for (int y = 0; y < 8; y++) {
      codey.display.setPixel(x, y, ((x + y) & 1) == 0);
    }
  }
  delay(800);

  uint8_t face[16] = {
    0x00, 0x0C, 0x1E, 0x3E, 0x7C, 0x3E, 0x1E, 0x0C,
    0x0C, 0x1E, 0x3E, 0x7C, 0x3E, 0x1E, 0x0C, 0x00
  };
  codey.display.setColumns(face);
}

void loop() {
  codey.update();
  static int x = 0;
  codey.display.setPixel(x, 7, false);
  x = (x + 1) & 15;
  codey.display.setPixel(x, 7, true);
  delay(80);
}
