# CodeyRocky — firmware libero per Makeblock Codey Rocky

[![CI](https://github.com/messinamirko99-design/codeyrocky-firmware/actions/workflows/build.yml/badge.svg)](https://github.com/messinamirko99-design/codeyrocky-firmware/actions/workflows/build.yml)
[![License: GPL-2.0-or-later](https://img.shields.io/badge/License-GPL--2.0--or--later-blue.svg)](LICENSE)
[![PlatformIO](https://img.shields.io/badge/PlatformIO-ESP32-orange.svg)](https://platformio.org/)

Libreria C++ (Arduino / PlatformIO) per programmare **Makeblock Codey Rocky**
in C++ puro, senza mBlock: flashi il *tuo* firmware sul robot.

> Hardware e protocollo Neurons ricavati da
> [FFtust/micropython_esp32_local](https://github.com/FFtust/micropython_esp32_local) (GPL-2.0).

## Cosa supporta

| Componente | Classe | Note |
|---|---|---|
| Matrice LED 16×8 | `codey.display` | pixel, font ASCII, facce, scroll, bitmap mBlock |
| Tasti A / B / C | `codey.buttonA/B/C` | pressione, rilascio, debouncing |
| LED RGB | `codey.led` | 0–255 per canale (rosso invertito in hardware, già corretto) |
| Luce / suono / manopola | `codey.light/sound/knob` | 0–100 |
| IMU (gyro + accel) | `codey.gyro` | pitch/roll/yaw, shake, tilt, calibrazione |
| Batteria | `codey.battery` | volt e percentuale |
| Speaker | `codey.speaker` | toni, note (C4, F#5…), tempo |
| IR NEC 38 kHz | `codey.ir` | invio/ricezione frame e testo |
| Base Rocky | `codey.rocky` | motori, colore, ostacolo, linea, corrente motori, LED |

## Flash in 4 passi

1. Installa [VS Code](https://code.visualstudio.com/) + l'estensione **PlatformIO**.
2. Clona questo repo e aprilo in VS Code.
3. Collega Codey via USB e accendilo.
   Se l'upload fallisce: tieni premuto **C**, tocca power, rilascia C, poi Upload
   (GPIO0, tasto C, è lo strap di boot dell'ESP32).
4. PlatformIO → **Upload**. Il firmware mBlock viene sovrascritto.

Per tornare al firmware originale: da mBlock 5 → Impostazioni → *Aggiorna firmware*.

⚠️ `GPIO15` viene portato HIGH da `codey.begin()`: senza questa riga Codey si spegne subito.

## Hello world

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

## Esempi

| Cartella (`lib/CodeyRocky/examples/`) | Cosa fa |
|---|---|
| `01_HelloMatrix` | testo statico + scroll |
| `02_CustomPixels` | pixel singoli e bitmap 16×8 |
| `03_FontsAndFaces` | font e facce |
| `04_AllSensors` | tutti i sensori Codey |
| `05_RockyMove` | motori |
| `06_RockyColorIR` | sensore colore + ostacolo |
| `07_SpeakerIR` | note + infrarossi NEC |
| `08_AvoidObstacle` | evita ostacoli |
| `09_LineFollow` | segue linea |
| `10_FullKit` | gyro + motori + LED |

Per provarne uno: copia il `.ino` in `src/main.cpp`.

## Documentazione completa

API, pinout completo, protocollo Neurons della base Rocky e note hardware:
vedi [**DATASHEET.md**](DATASHEET.md).

## Struttura del repo

```
├── platformio.ini          # configurazione build (esp32dev, arduino)
├── src/main.cpp            # sketch principale
├── lib/CodeyRocky/         # la libreria (usabile anche standalone)
│   ├── src/                # sorgenti C++
│   ├── examples/           # 10 esempi
│   ├── library.json        # metadati PlatformIO
│   └── library.properties  # metadati Arduino IDE
├── DATASHEET.md            # datasheet: API, pinout, protocollo
└── .github/workflows/      # CI: build PlatformIO ad ogni push
```

## Licenza

[GPL-2.0-or-later](LICENSE) — come i driver originali Makeblock.
Firmware originale e fonte del protocollo:
[FFtust/micropython_esp32_local](https://github.com/FFtust/micropython_esp32_local).
