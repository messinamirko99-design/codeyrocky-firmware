# CodeyRocky — datasheet libreria PlatformIO

Firmware C++ per **Makeblock Codey Rocky** (ESP32). Sostituisce il firmware mBlock.
Hardware e protocollo Neurons ricavati da
[FFtust/micropython_esp32_local](https://github.com/FFtust/micropython_esp32_local).

La matrice ufficiale è **16 colonne × 8 righe**, non 16×4.

## 1. Flash in 4 passi

1. Installa [VS Code](https://code.visualstudio.com/) + estensione **PlatformIO**.
2. Scompatta lo ZIP e apri la cartella `codeyrocky-firmware`.
3. Collega Codey via USB. Accendi. Se l’upload fallisce: tieni premuto **C**, tocca power, rilascia C, poi Upload.
4. PlatformIO → **Upload**. Il firmware mBlock viene sovrascritto.

Per tornare a mBlock: da mBlock 5, Impostazioni → Aggiorna firmware.

`GPIO15` viene messo HIGH da `codey.begin()`: senza questo Codey si spegne.

## 2. Hello

```cpp
#include <CodeyRocky.h>

void setup() {
  codey.begin();
  codey.display.showText("CIAO");
}

void loop() {
  codey.update();   // obbligatorio: bottoni, gyro, IR, UART Rocky
}
```

## 3. Pinout (dal source originale)

| Funzione | GPIO | Note |
|---|---|---|
| Power hold | 15 | Output HIGH = acceso |
| Power key | 34 | Active LOW, hold 2s = off |
| Button A | 2 | Active HIGH |
| Button B | 12 | Active HIGH |
| Button C | 0 | Active LOW (boot strap). V1.07: GPIO27 |
| RGB R/G/B | 14 / 21 / 4 | PWM LEDC. **Rosso invertito** nel hardware |
| Matrix SDA/SCL | 22 / 23 | TM1640 bitbang, 16×8 |
| IMU SDA/SCL | 19 / 18 | I2C 0x68, WHO_AM_I 0x98 |
| Light | 35 | ADC 0..100 |
| Sound | 36 | ADC 0..100 |
| Knob | 39 | ADC 0..100 |
| Battery | 33 | ~3.4–4.2 V, fattore 0.001934 |
| HW version | 32 | ADC |
| Speaker | 25 | DAC/PWM |
| IR TX / RX | 5 / 26 | NEC 38 kHz (non V1.07). V1.07: scambiati |
| Rocky UART TX/RX | 17 / 16 | 115200 8N1 |

## 4. Display 16×8

Coordinate: `x = 0..15` (sinistra→destra), `y = 0..7` (alto→basso).
`setColumns()` usa 16 byte, un byte per colonna, **bit0 = riga in alto**.
Stesso packing del face hex mBlock (la libreria fa il bit-reverse verso il TM1640).

```cpp
codey.display.setPixel(3, 2, true);
codey.display.getPixel(3, 2);
codey.display.clear();
codey.display.fill(true);
codey.display.setBrightness(8);          // 0..8
codey.display.showText("HELLO");
codey.display.showNumber(42);
codey.display.scrollText("CODEY", 80);
codey.display.showChar('A', 5);
codey.display.setFace(FACE_HAPPY);       // CRY SAD ANGRY FEAR HEART ARROW
codey.display.playAnimation(FACE_HAPPY, 3);
codey.display.setImageHex("000C1E3E7C3E1E0C0C1E3E7C3E1E0C00");

uint8_t cols[16] = { /* 16 colonne */ };
codey.display.setColumns(cols);
```

Font: ASCII 6×8 dal firmware originale + numeri stretti 3×8.

## 5. Sensori Codey

```cpp
codey.light.read();          // 0..100
codey.sound.read();
codey.knob.read();
codey.battery.percent();     // 0..100
codey.battery.voltage();     // volt

codey.gyro.pitch();          // gradi
codey.gyro.roll();
codey.gyro.yaw();            // integrale, deriva
codey.gyro.accelX/Y/Z();     // m/s^2
codey.gyro.gyroX/Y/Z();      // deg/s
codey.gyro.isShaking();
codey.gyro.isTilted(TILT_FORWARD);
codey.gyro.calibrate();      // fermo in piano

codey.buttonA.isPressed();
codey.buttonA.wasPressed();
codey.buttonB / buttonC
codey.led.setRGB(r,g,b);     // 0..255 (rosso hardware-invertito, già corretto)
codey.led.off();
codey.powerOff();
codey.hardwareVersion();
```

## 6. Rocky (base)

Protocollo Neurons su UART1: `F0 id 63 10 cmd … checksum F7`.

```cpp
if (!codey.rocky.isConnected()) { /* Codey non è sulla base */ }

codey.rocky.forward(50);           // velocità 0..100, infinito
codey.rocky.forward(50, 1000);     // 1 secondo poi stop
codey.rocky.backward(40, 500);
codey.rocky.turnLeft(40, 400);
codey.rocky.turnRight(40, 400);
codey.rocky.drive(30, -30);        // sinistra, destra −100..100
codey.rocky.stop();

codey.rocky.setLight(255, 0, 0);   // LED RGB del color sensor
codey.rocky.isObstacleAhead();     // IR davanti (sensore in avanti)
codey.rocky.color();               // CrColor
codey.rocky.colorName();           // "red", "green", …
codey.rocky.isColor(COLOR_RED);
codey.rocky.red()/green()/blue();
codey.rocky.grey();
codey.rocky.lightness();
codey.rocky.lightReflect();
codey.rocky.irReflect();           // linea / caduta (sensore in giù)
codey.rocky.motorCurrentLeft();
```

Comandi Neurons Rocky (class 0x63 subtype 0x10):

| cmd | funzione |
|---|---|
| 0x01 | RGB |
| 0x02 | colore classificato |
| 0x03 | luce ambiente |
| 0x04 | grigio |
| 0x05 | riflettanza visibile |
| 0x06 | riflettanza IR |
| 0x08 | ostacolo |
| 0x09 | set RGB |
| 0x0A | corrente motori |
| 0x0C–0x14 | stop / fwd / back / dx / sx / drive / diff / singolo |

## 7. Speaker e IR

```cpp
codey.speaker.setVolume(70);       // 0..100
codey.speaker.tone(440, 200);      // Hz, ms
codey.speaker.note("C4", 1);       // nome nota, beats
codey.speaker.setTempo(120);
codey.speaker.rest(0.5);
codey.speaker.stop();

codey.ir.sendNEC(0x00FF, 0x1A);
codey.ir.sendText("CIAO");
if (codey.ir.available()) {
  uint16_t c = codey.ir.lastCommand();
}
```

## 8. Esempi

| cartella | cosa fa |
|---|---|
| `01_HelloMatrix` | testo + scroll |
| `02_CustomPixels` | pixel e bitmap 16×8 |
| `03_FontsAndFaces` | font e facce |
| `04_AllSensors` | tutti i sensori Codey |
| `05_RockyMove` | motori |
| `06_RockyColorIR` | colore + ostacolo |
| `07_SpeakerIR` | note + NEC |
| `08_AvoidObstacle` | evita ostacoli |
| `09_LineFollow` | segue linea |
| `10_FullKit` | gyro + motori + LED |

Copia il `.ino` in `src/main.cpp` (rinomina in `.cpp` oppure lascia che PlatformIO prenda `src/`).

## 9. Licenza

Driver originali Makeblock: GPL-2.0. Questa libreria è GPL-2.0-or-later.
