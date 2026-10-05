#include "CodeyRocky.h"

static void busyWait() {
  volatile uint32_t i = 400;
  while (i--) { __asm__ __volatile__("nop"); }
}

void CrMatrix::_pulseDelay() { busyWait(); }

void CrMatrix::_writePin(uint8_t pin, bool level) {
  digitalWrite(pin, level ? HIGH : LOW);
  _pulseDelay();
}

uint8_t CrMatrix::_bitReverse(uint8_t data) {
  uint8_t t = 0;
  t |= (data & 0x80) >> 7;
  t |= (data & 0x40) >> 5;
  t |= (data & 0x20) >> 3;
  t |= (data & 0x10) >> 1;
  t |= (data & 0x08) << 1;
  t |= (data & 0x04) << 3;
  t |= (data & 0x02) << 5;
  t |= (data & 0x01) << 7;
  return t;
}

void CrMatrix::_writeByte(uint8_t data) {
  _writePin(CR_PIN_MATRIX_SCL, true);
  _writePin(CR_PIN_MATRIX_SDA, true);
  _writePin(CR_PIN_MATRIX_SDA, false);
  for (uint8_t i = 0; i < 8; i++) {
    _writePin(CR_PIN_MATRIX_SCL, false);
    _writePin(CR_PIN_MATRIX_SDA, data & 0x01);
    _writePin(CR_PIN_MATRIX_SCL, true);
    data >>= 1;
  }
  _writePin(CR_PIN_MATRIX_SCL, false);
  _writePin(CR_PIN_MATRIX_SDA, false);
  _writePin(CR_PIN_MATRIX_SCL, true);
  _writePin(CR_PIN_MATRIX_SDA, true);
}

void CrMatrix::_writeBuffer() {
  uint8_t address = 0xC0;
  _writePin(CR_PIN_MATRIX_SCL, true);
  _writePin(CR_PIN_MATRIX_SDA, true);
  _writeByte(0x40); // auto address increment
  _writePin(CR_PIN_MATRIX_SCL, true);
  _writePin(CR_PIN_MATRIX_SDA, true);
  _writePin(CR_PIN_MATRIX_SDA, false);
  for (uint8_t i = 0; i < 8; i++) {
    _writePin(CR_PIN_MATRIX_SCL, false);
    _writePin(CR_PIN_MATRIX_SDA, address & 0x01);
    _writePin(CR_PIN_MATRIX_SCL, true);
    address >>= 1;
  }
  for (uint8_t k = 0; k < 16; k++) {
    uint8_t t = _buf[k];
    for (uint8_t i = 0; i < 8; i++) {
      _writePin(CR_PIN_MATRIX_SCL, false);
      _writePin(CR_PIN_MATRIX_SDA, t & 0x01);
      _writePin(CR_PIN_MATRIX_SCL, true);
      t >>= 1;
    }
  }
  _writePin(CR_PIN_MATRIX_SCL, false);
  _writePin(CR_PIN_MATRIX_SDA, false);
  _writePin(CR_PIN_MATRIX_SCL, true);
  _writePin(CR_PIN_MATRIX_SDA, true);
}

void CrMatrix::begin() {
  pinMode(CR_PIN_MATRIX_SCL, OUTPUT);
  pinMode(CR_PIN_MATRIX_SDA, OUTPUT);
  digitalWrite(CR_PIN_MATRIX_SCL, HIGH);
  digitalWrite(CR_PIN_MATRIX_SDA, HIGH);
  memset(_buf, 0, 16);
  _writeByte(0x40);
  setBrightness(8);
  show();
}

void CrMatrix::setBrightness(uint8_t level) {
  if (level > 8) level = 8;
  uint8_t b = 0;
  if (level != 0) b = (uint8_t)((level - 1) | 0x08);
  _writeByte(0x80 | b);
}

void CrMatrix::clear() {
  memset(_buf, 0, 16);
  show();
}

void CrMatrix::show() { _writeBuffer(); }

void CrMatrix::fill(bool on) {
  memset(_buf, on ? 0xFF : 0x00, 16);
  show();
}

void CrMatrix::setPixel(int x, int y, bool on) {
  if (x < 0 || x > 15 || y < 0 || y > 7) return;
  if (on) _buf[x] |= (uint8_t)(1 << y);
  else _buf[x] &= (uint8_t)~(1 << y);
  show();
}

bool CrMatrix::getPixel(int x, int y) const {
  if (x < 0 || x > 15 || y < 0 || y > 7) return false;
  return (_buf[x] >> y) & 1;
}

void CrMatrix::setColumns(const uint8_t cols[16]) {
  for (int i = 0; i < 16; i++) _buf[i] = _bitReverse(cols[i]);
  show();
}

void CrMatrix::setFaceRaw(const uint8_t cols[16]) { setColumns(cols); }

bool CrMatrix::setImageHex(const char *hex32) {
  if (!hex32) return false;
  uint8_t cols[16];
  for (int i = 0; i < 16; i++) {
    char a = hex32[i * 2];
    char b = hex32[i * 2 + 1];
    if (!a || !b) return false;
    auto nib = [](char c) -> int {
      if (c >= '0' && c <= '9') return c - '0';
      if (c >= 'a' && c <= 'f') return c - 'a' + 10;
      if (c >= 'A' && c <= 'F') return c - 'A' + 10;
      return -1;
    };
    int hi = nib(a), lo = nib(b);
    if (hi < 0 || lo < 0) return false;
    cols[i] = (uint8_t)((hi << 4) | lo);
  }
  setColumns(cols);
  return true;
}

void CrMatrix::setFace(CrFace face) {
  const uint8_t *p = CR_FACE_HAPPY;
  switch (face) {
    case FACE_CRY: p = CR_FACE_CRY; break;
    case FACE_SAD: p = CR_FACE_SAD; break;
    case FACE_ANGRY: p = CR_FACE_ANGRY; break;
    case FACE_FEAR: p = CR_FACE_FEAR; break;
    case FACE_HEART: p = CR_FACE_HEART; break;
    case FACE_ARROW: p = CR_FACE_ARROW; break;
    default: p = CR_FACE_HAPPY; break;
  }
  setColumns(p);
}

void CrMatrix::invert() {
  for (int i = 0; i < 16; i++) _buf[i] = ~_buf[i];
  show();
}

const uint8_t *CrMatrix::_glyph(char c, uint8_t *width) {
  static uint8_t tmp[6];
  for (uint8_t i = 0; i < CR_FONT_LEN; i++) {
    CrGlyph g;
    memcpy_P(&g, &CR_FONT[i], sizeof(g));
    if (g.ch == c) {
      memcpy(tmp, g.col, 6);
      *width = 6;
      return tmp;
    }
  }
  memset(tmp, 0, 6);
  *width = 6;
  return tmp;
}

void CrMatrix::showChar(char c, int x) {
  uint8_t w;
  const uint8_t *g = _glyph(c, &w);
  memset(_buf, 0, 16);
  for (uint8_t i = 0; i < w; i++) {
    int col = x + i;
    if (col >= 0 && col < 16) _buf[col] = _bitReverse(g[i]);
  }
  show();
}

void CrMatrix::showText(const char *s) {
  memset(_buf, 0, 16);
  int x = 0;
  while (*s && x < 16) {
    uint8_t w;
    const uint8_t *g = _glyph(*s++, &w);
    for (uint8_t i = 0; i < w && x < 16; i++, x++) {
      _buf[x] = _bitReverse(g[i]);
    }
  }
  show();
}

void CrMatrix::showNumber(int n) {
  char buf[12];
  snprintf(buf, sizeof(buf), "%d", n);
  memset(_buf, 0, 16);
  int x = 0;
  for (char *p = buf; *p && x < 16; p++) {
    for (uint8_t i = 0; i < CR_NUMFONT_LEN; i++) {
      CrNumGlyph g;
      memcpy_P(&g, &CR_NUMFONT[i], sizeof(g));
      if (g.ch == *p) {
        for (int k = 0; k < 3 && x < 16; k++, x++) _buf[x] = _bitReverse(g.col[k]);
        if (x < 16) _buf[x++] = 0;
        break;
      }
    }
  }
  show();
}

void CrMatrix::scrollText(const char *s, uint16_t msPerCol) {
  uint8_t packed[256];
  int n = 0;
  packed[n++] = 0;
  packed[n++] = 0;
  while (*s && n < 240) {
    uint8_t w;
    const uint8_t *g = _glyph(*s++, &w);
    for (uint8_t i = 0; i < w && n < 240; i++) packed[n++] = _bitReverse(g[i]);
  }
  packed[n++] = 0;
  packed[n++] = 0;
  for (int off = 0; off <= n - 16; off++) {
    memcpy(_buf, packed + off, 16);
    show();
    delay(msPerCol);
  }
}

void CrMatrix::playAnimation(CrFace face, uint8_t loops) {
  const uint8_t (*data)[16] = CR_ANIM_HAPPY_DATA;
  uint8_t frames = 3;
  uint16_t interval = 200;
  switch (face) {
    case FACE_CRY: data = CR_ANIM_CRY_DATA; break;
    case FACE_SAD: data = CR_ANIM_DISPIRITED_DATA; break;
    case FACE_ANGRY: data = CR_ANIM_ANGRY_DATA; break;
    case FACE_FEAR: data = CR_ANIM_FEAR_DATA; break;
    default: data = CR_ANIM_HAPPY_DATA; break;
  }
  for (uint8_t l = 0; l < loops; l++) {
    for (uint8_t f = 0; f < frames; f++) {
      setColumns(data[f]);
      delay(interval);
    }
  }
}
