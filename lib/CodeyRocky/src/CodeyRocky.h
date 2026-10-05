#pragma once
/*
  CodeyRocky — firmware library for Makeblock Codey Rocky (ESP32).
  Replaces the stock mBlock firmware. Flash with PlatformIO.

  Hardware map and Neurons protocol from
  https://github.com/FFtust/micropython_esp32_local  (GPL-2.0)
*/

#include <Arduino.h>
#include "cr_pins.h"
#include "cr_font.h"

enum CrTilt {
  TILT_FORWARD = 0,
  TILT_BACKWARD,
  TILT_LEFT,
  TILT_RIGHT
};

enum CrColor {
  COLOR_WHITE = 0,
  COLOR_PINK,
  COLOR_RED,
  COLOR_ORANGE,
  COLOR_YELLOW,
  COLOR_GREEN,
  COLOR_CYAN,
  COLOR_BLUE,
  COLOR_PURPLE,
  COLOR_BLACK,
  COLOR_GOLD,
  COLOR_UNKNOWN = 255
};

enum CrFace {
  FACE_HAPPY = 0,
  FACE_CRY,
  FACE_SAD,
  FACE_ANGRY,
  FACE_FEAR,
  FACE_HEART,
  FACE_ARROW
};

class CrMatrix {
public:
  void begin();
  void setBrightness(uint8_t level);           // 0..8
  void clear();
  void show();                                 // push framebuffer
  void setPixel(int x, int y, bool on = true); // x 0..15, y 0..7 (top = 0)
  bool getPixel(int x, int y) const;
  void fill(bool on);
  // 16 column bytes: bit0 = top row. Same packing as mBlock face hex.
  void setColumns(const uint8_t cols[16]);
  void setFace(CrFace face);
  void setFaceRaw(const uint8_t cols[16]);
  // 32 hex chars (16 bytes), mBlock face string. Returns false if invalid.
  bool setImageHex(const char *hex32);
  void invert();
  void showChar(char c, int x = 0);
  void showText(const char *s);                // static, first 16 cols
  void showNumber(int n);
  void scrollText(const char *s, uint16_t msPerCol = 80);
  void playAnimation(CrFace face, uint8_t loops = 3);
  const uint8_t *buffer() const { return _buf; }

private:
  uint8_t _buf[16];
  void _pulseDelay();
  void _writePin(uint8_t pin, bool level);
  void _writeByte(uint8_t data);
  void _writeBuffer();
  uint8_t _bitReverse(uint8_t v);
  const uint8_t *_glyph(char c, uint8_t *width);
};

class CrButton {
public:
  void begin(uint8_t pin, bool activeHigh);
  void update();
  bool isPressed() const { return _pressed; }
  bool wasPressed();
  bool wasReleased();
private:
  uint8_t _pin;
  bool _activeHigh;
  bool _pressed;
  bool _prev;
  bool _rose;
  bool _fell;
  uint32_t _lastMs;
};

class CrRgb {
public:
  void begin();
  void setRGB(uint8_t r, uint8_t g, uint8_t b); // 0..255
  void setColor(uint32_t rgb);
  void off();
private:
  void _write(uint8_t pin, uint8_t value);
};

class CrAnalog {
public:
  void begin(uint8_t pin);
  int raw();                 // 0..4095
  int read();                // 0..100
private:
  uint8_t _pin;
};

class CrBattery {
public:
  void begin();
  float voltage();           // ~3.4 .. 4.2 V
  int percent();             // 0..100
};

class CrGyro {
public:
  bool begin();
  void update();
  void calibrate();          // keep still and level
  float accelX() const { return _ax; }
  float accelY() const { return _ay; }
  float accelZ() const { return _az; }
  float gyroX() const { return _gx; }
  float gyroY() const { return _gy; }
  float gyroZ() const { return _gz; }
  float pitch() const { return _pitch; }
  float roll() const { return _roll; }
  float yaw() const { return _yaw; }
  float temperature() const { return _temp; }
  bool isShaking() const;
  bool isTilted(CrTilt dir, float threshold = 20.0f) const;
  bool ok() const { return _ok; }
private:
  bool _ok;
  float _ax, _ay, _az, _gx, _gy, _gz, _pitch, _roll, _yaw, _temp;
  float _offX, _offY, _offZ;
  uint32_t _lastUs;
  uint8_t _who;
  bool _writeReg(uint8_t reg, uint8_t val);
  bool _readBytes(uint8_t reg, uint8_t *buf, uint8_t n);
};

class CrSpeaker {
public:
  void begin();
  void setVolume(uint8_t vol);     // 0..100
  uint8_t volume() const { return _vol; }
  void tone(uint16_t freq, uint32_t ms = 0);
  void note(const char *name, float beats = 1);  // "C4", "F#5", or MIDI number
  void rest(float beats = 1);
  void setTempo(uint16_t bpm);
  void stop();
  void update();
private:
  uint8_t _vol;
  uint16_t _tempo;
  uint32_t _until;
  bool _playing;
  void _startTone(uint16_t freq);
};

class CrIr {
public:
  void begin();
  void sendNEC(uint16_t address, uint16_t command);
  void sendText(const char *s);    // NEC packets, one byte each + newline
  bool available();                // true once per received frame, then clears
  uint16_t lastAddress() const { return _addr; }
  uint16_t lastCommand() const { return _cmd; }
  int readText(char *out, int maxLen);
  void update();
private:
  volatile uint32_t _raw;
  volatile bool _have;
  uint16_t _addr, _cmd;
  char _text[48];
  uint8_t _textLen;
  static void IRAM_ATTR _isr();
  static CrIr *_inst;
  void _markSpace(bool mark, uint32_t us);
  bool _decodeNEC(uint32_t v);
};

class CrRocky {
public:
  bool begin();
  void update();
  bool isConnected() const { return _connected; }
  void assignId();

  void stop();
  void forward(int speed, int ms = -1);        // speed 0..100
  void backward(int speed, int ms = -1);
  void turnLeft(int speed, int ms = -1);
  void turnRight(int speed, int ms = -1);
  void drive(int left, int right);             // -100..100 per wheel
  void singleMotor(uint8_t which, int speed);  // which 0=left 1=right

  void setLight(uint8_t r, uint8_t g, uint8_t b); // 0..255
  void setLightOff();

  bool requestRgb();
  int red() const { return _r; }
  int green() const { return _g; }
  int blue() const { return _b; }
  CrColor color();
  const char *colorName();
  bool isColor(CrColor c);
  int lightness();
  int grey();
  int lightReflect();
  int irReflect();
  bool isObstacleAhead();
  int motorCurrentLeft();
  int motorCurrentRight();

private:
  uint8_t _id;
  bool _connected;
  int _r, _g, _b;
  int _light, _grey, _lref, _iref, _barrier;
  int _curL, _curR;
  CrColor _color;
  uint8_t _rx[80];
  uint8_t _rxLen;
  uint32_t _lastSeen;
  uint32_t _moveUntil;

  void _send(uint8_t cmd, const uint8_t *params, uint8_t nParams);
  bool _query(uint8_t cmd, uint8_t *out, uint8_t &outLen, uint32_t timeout = 120);
  void _feed(uint8_t b);
  void _handleFrame(const uint8_t *f, uint8_t len);
  static uint8_t _checksum(const uint8_t *d, uint8_t from, uint8_t to);
  void _timedMove(uint8_t cmd, int speed, int ms);
};

class CodeyRocky {
public:
  CrMatrix display;
  CrButton buttonA, buttonB, buttonC;
  CrRgb led;
  CrAnalog light, sound, knob;
  CrBattery battery;
  CrGyro gyro;
  CrSpeaker speaker;
  CrIr ir;
  CrRocky rocky;

  void begin(bool powerHold = true);
  void update();                   // call every loop
  void powerOff();
  int hardwareVersion();           // raw ADC of GPIO32
};

extern CodeyRocky codey;
