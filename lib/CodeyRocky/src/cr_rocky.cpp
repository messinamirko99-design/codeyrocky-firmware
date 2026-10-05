#include "CodeyRocky.h"

// Makeblock Neurons (Firmata-style) protocol on UART1.
// Frame: 0xF0  dev  0x63  0x10  cmd  [params]  checksum  0xF7
// checksum = (sum of bytes from device_id through last param) & 0x7F

#define SYSEX_START 0xF0
#define SYSEX_END   0xF7
#define CTL_ASSIGN  0x10
#define CLASS_SENSOR 0x63
#define BLOCK_ROCKY  0x10

enum {
  RK_RGB = 0x01,
  RK_COLOR = 0x02,
  RK_LIGHT = 0x03,
  RK_GREY = 0x04,
  RK_LREF = 0x05,
  RK_IREF = 0x06,
  RK_INGREY = 0x07,
  RK_BARRIER = 0x08,
  RK_SETRGB = 0x09,
  RK_CURRENT = 0x0A,
  RK_STOP = 0x0C,
  RK_FWD = 0x0D,
  RK_BACK = 0x0E,
  RK_RIGHT = 0x0F,
  RK_LEFT = 0x10,
  RK_DRIVE = 0x11,
  RK_RDIF = 0x12,
  RK_LDIF = 0x13,
  RK_SINGLE = 0x14
};

static const char *kColorName[] = {
  "white", "pink", "red", "orange", "yellow",
  "green", "cyan", "blue", "purple", "black", "gold"
};

uint8_t CrRocky::_checksum(const uint8_t *d, uint8_t from, uint8_t to) {
  uint16_t s = 0;
  for (uint8_t i = from; i < to; i++) s += d[i];
  return (uint8_t)(s & 0x7F);
}

void CrRocky::_send(uint8_t cmd, const uint8_t *params, uint8_t nParams) {
  uint8_t pkt[32];
  uint8_t n = 0;
  pkt[n++] = SYSEX_START;
  pkt[n++] = _id;
  pkt[n++] = CLASS_SENSOR;
  pkt[n++] = BLOCK_ROCKY;
  pkt[n++] = cmd;
  for (uint8_t i = 0; i < nParams; i++) {
    pkt[n++] = params[i] & 0x7F;
  }
  pkt[n] = _checksum(pkt, 1, n);
  n++;
  pkt[n++] = SYSEX_END;
  Serial1.write(pkt, n);
}

void CrRocky::assignId() {
  uint8_t pkt[8];
  uint8_t n = 0;
  pkt[n++] = SYSEX_START;
  pkt[n++] = 0xFF;
  pkt[n++] = CTL_ASSIGN;
  pkt[n++] = 0x00;
  pkt[n] = _checksum(pkt, 1, n);
  n++;
  pkt[n++] = SYSEX_END;
  Serial1.write(pkt, n);
}

bool CrRocky::begin() {
  _id = 1;
  _connected = false;
  _r = _g = _b = 0;
  _light = _grey = _lref = _iref = _barrier = 0;
  _curL = _curR = 0;
  _color = COLOR_UNKNOWN;
  _rxLen = 0;
  _lastSeen = 0;
  _moveUntil = 0;
  Serial1.begin(CR_ROCKY_BAUD, SERIAL_8N1, CR_PIN_ROCKY_RX, CR_PIN_ROCKY_TX);
  delay(30);
  assignId();
  delay(80);
  assignId();
  stop();
  setLight(0, 0, 0);
  uint32_t t0 = millis();
  while (millis() - t0 < 250) {
    update();
    if (_connected) break;
    delay(5);
  }
  return _connected;
}

void CrRocky::_feed(uint8_t b) {
  if (b == SYSEX_START) {
    _rxLen = 0;
    _rx[_rxLen++] = b;
    return;
  }
  if (_rxLen == 0) return;
  if (_rxLen < sizeof(_rx)) _rx[_rxLen++] = b;
  if (b == SYSEX_END) {
    _handleFrame(_rx, _rxLen);
    _rxLen = 0;
  }
}

void CrRocky::_handleFrame(const uint8_t *f, uint8_t len) {
  if (len < 6) return;
  if (f[0] != SYSEX_START || f[len - 1] != SYSEX_END) return;
  uint8_t cs = _checksum(f, 1, len - 2);
  if (cs != f[len - 2]) return;
  _lastSeen = millis();
  _connected = true;
  if (f[2] != CLASS_SENSOR || f[3] != BLOCK_ROCKY) return;
  uint8_t cmd = f[4];
  const uint8_t *p = f + 5;
  uint8_t plen = (len > 7) ? (uint8_t)(len - 7) : 0;
  auto u8 = [&](uint8_t i) -> int { return (i < plen) ? p[i] : 0; };
  auto u16 = [&](uint8_t i) -> int {
    if (i + 1 >= plen) return 0;
    return (p[i] & 0x7F) | ((p[i + 1] & 0x7F) << 7);
  };
  switch (cmd) {
    case RK_RGB:
      _r = u16(0);
      _g = u16(2);
      _b = u16(4);
      break;
    case RK_COLOR:
      _color = (CrColor)u8(0);
      break;
    case RK_LIGHT: _light = u8(0); break;
    case RK_GREY: _grey = u8(0); break;
    case RK_LREF: _lref = u8(0); break;
    case RK_IREF: _iref = u8(0); break;
    case RK_BARRIER: _barrier = u8(0); break;
    case RK_CURRENT:
      _curL = u16(0);
      _curR = u16(2);
      break;
    default: break;
  }
}

void CrRocky::update() {
  while (Serial1.available()) _feed((uint8_t)Serial1.read());
  if (_connected && millis() - _lastSeen > 3000) _connected = false;
  if (_moveUntil && millis() >= _moveUntil) {
    _moveUntil = 0;
    stop();
  }
}

bool CrRocky::_query(uint8_t cmd, uint8_t *out, uint8_t &outLen, uint32_t timeout) {
  _send(cmd, nullptr, 0);
  uint32_t t0 = millis();
  while (millis() - t0 < timeout) {
    while (Serial1.available()) _feed((uint8_t)Serial1.read());
    if (_lastSeen >= t0) {
      outLen = 0;
      (void)out;
      return true;
    }
    delay(2);
  }
  return _connected;
}

void CrRocky::stop() {
  _send(RK_STOP, nullptr, 0);
  _moveUntil = 0;
}

void CrRocky::_timedMove(uint8_t cmd, int speed, int ms) {
  if (speed < 0) speed = 0;
  if (speed > 100) speed = 100;
  uint8_t s = (uint8_t)speed;
  _send(cmd, &s, 1);
  if (ms > 0) {
    _moveUntil = millis() + (uint32_t)ms;
    uint32_t t0 = millis();
    while (millis() - t0 < (uint32_t)ms) {
      update();
      delay(5);
    }
    stop();
  } else {
    _moveUntil = 0;
  }
}

void CrRocky::forward(int speed, int ms) {
  if (speed < 0) { backward(-speed, ms); return; }
  _timedMove(RK_FWD, speed, ms);
}

void CrRocky::backward(int speed, int ms) {
  if (speed < 0) { forward(-speed, ms); return; }
  _timedMove(RK_BACK, speed, ms);
}

void CrRocky::turnLeft(int speed, int ms) {
  if (speed < 0) { turnRight(-speed, ms); return; }
  _timedMove(RK_LEFT, speed, ms);
}

void CrRocky::turnRight(int speed, int ms) {
  if (speed < 0) { turnLeft(-speed, ms); return; }
  _timedMove(RK_RIGHT, speed, ms);
}

void CrRocky::drive(int left, int right) {
  uint8_t p[4];
  uint8_t ld = 0, rd = 1;
  if (left < 0) { ld = 1; left = -left; }
  if (right < 0) { rd = 0; right = -right; }
  if (left > 100) left = 100;
  if (right > 100) right = 100;
  p[0] = ld;
  p[1] = (uint8_t)left;
  p[2] = rd;
  p[3] = (uint8_t)right;
  _send(RK_DRIVE, p, 4);
}

void CrRocky::singleMotor(uint8_t which, int speed) {
  uint8_t dir = 0;
  if (speed < 0) { dir = 1; speed = -speed; }
  if (speed > 100) speed = 100;
  uint8_t p[3] = { which, dir, (uint8_t)speed };
  _send(RK_SINGLE, p, 3);
}

void CrRocky::setLight(uint8_t r, uint8_t g, uint8_t b) {
  // Neurons TYPE_BYTE_8 is 0..127. Map 0..255 → 0..127.
  uint8_t p[3] = { (uint8_t)(r >> 1), (uint8_t)(g >> 1), (uint8_t)(b >> 1) };
  _send(RK_SETRGB, p, 3);
}

void CrRocky::setLightOff() { setLight(0, 0, 0); }

bool CrRocky::requestRgb() {
  uint8_t dummy; uint8_t n = 0;
  return _query(RK_RGB, &dummy, n);
}

CrColor CrRocky::color() {
  uint8_t dummy; uint8_t n = 0;
  _query(RK_COLOR, &dummy, n);
  return _color;
}

const char *CrRocky::colorName() {
  CrColor c = color();
  if (c <= COLOR_GOLD) return kColorName[c];
  return "unknown";
}

bool CrRocky::isColor(CrColor c) { return color() == c; }

int CrRocky::lightness() {
  uint8_t dummy; uint8_t n = 0;
  _query(RK_LIGHT, &dummy, n);
  return _light;
}

int CrRocky::grey() {
  uint8_t dummy; uint8_t n = 0;
  _query(RK_GREY, &dummy, n);
  return _grey;
}

int CrRocky::lightReflect() {
  uint8_t dummy; uint8_t n = 0;
  _query(RK_LREF, &dummy, n);
  return _lref;
}

int CrRocky::irReflect() {
  uint8_t dummy; uint8_t n = 0;
  _query(RK_IREF, &dummy, n);
  return _iref;
}

bool CrRocky::isObstacleAhead() {
  uint8_t dummy; uint8_t n = 0;
  _query(RK_BARRIER, &dummy, n);
  return _barrier == 1;
}

int CrRocky::motorCurrentLeft() {
  uint8_t dummy; uint8_t n = 0;
  _query(RK_CURRENT, &dummy, n);
  return _curL;
}

int CrRocky::motorCurrentRight() {
  uint8_t dummy; uint8_t n = 0;
  _query(RK_CURRENT, &dummy, n);
  return _curR;
}
