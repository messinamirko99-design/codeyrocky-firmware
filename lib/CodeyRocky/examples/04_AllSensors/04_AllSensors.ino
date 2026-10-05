#include <CodeyRocky.h>

void setup() {
  Serial.begin(115200);
  codey.begin();
}

void loop() {
  codey.update();

  int L = codey.light.read();
  int S = codey.sound.read();
  int K = codey.knob.read();
  int B = codey.battery.percent();

  Serial.printf("light=%d sound=%d knob=%d bat=%d%%  ", L, S, K, B);
  Serial.printf("p=%.1f r=%.1f y=%.1f shake=%d\n",
                codey.gyro.pitch(), codey.gyro.roll(), codey.gyro.yaw(),
                codey.gyro.isShaking());

  // Knob selects what the matrix shows.
  if (K < 25) codey.display.showNumber(L);
  else if (K < 50) codey.display.showNumber(S);
  else if (K < 75) codey.display.showNumber(B);
  else codey.display.showNumber((int)codey.gyro.pitch());

  uint8_t c = (uint8_t)map(S, 0, 100, 0, 80);
  codey.led.setRGB(c, 10, 80 - c);

  if (codey.buttonA.isPressed()) codey.display.showText("A");
  if (codey.buttonB.isPressed()) codey.display.showText("B");
  if (codey.buttonC.isPressed()) codey.display.showText("C");

  delay(80);
}
