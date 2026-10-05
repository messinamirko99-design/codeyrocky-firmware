#include "CodeyRocky.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
#define CR_LEDC3 1
#else
#define CR_LEDC3 0
#endif

#define CR_SPK_CH 3
#define CR_IR_CH  4

void CrSpeaker::begin() {
  _vol = 80;
  _tempo = 60;
  _until = 0;
  _playing = false;
#if CR_LEDC3
  ledcAttach(CR_PIN_SPEAKER, 2000, 10);
  ledcWrite(CR_PIN_SPEAKER, 0);
#else
  ledcSetup(CR_SPK_CH, 2000, 10);
  ledcAttachPin(CR_PIN_SPEAKER, CR_SPK_CH);
  ledcWrite(CR_SPK_CH, 0);
#endif
}

void CrSpeaker::setVolume(uint8_t vol) {
  if (vol > 100) vol = 100;
  _vol = vol;
}

void CrSpeaker::_startTone(uint16_t freq) {
  if (freq < 20) {
    stop();
    return;
  }
  uint32_t duty = (uint32_t)_vol * 512 / 100;
  if (duty > 512) duty = 512;
#if CR_LEDC3
  ledcChangeFrequency(CR_PIN_SPEAKER, freq, 10);
  ledcWrite(CR_PIN_SPEAKER, duty);
#else
  ledcSetup(CR_SPK_CH, freq, 10);
  ledcWrite(CR_SPK_CH, duty);
#endif
  _playing = true;
}

void CrSpeaker::tone(uint16_t freq, uint32_t ms) {
  _startTone(freq);
  if (ms == 0) {
    _until = 0;
  } else {
    _until = millis() + ms;
    delay(ms);
    stop();
  }
}

void CrSpeaker::stop() {
#if CR_LEDC3
  ledcWrite(CR_PIN_SPEAKER, 0);
#else
  ledcWrite(CR_SPK_CH, 0);
#endif
  _playing = false;
  _until = 0;
}

void CrSpeaker::update() {
  if (_playing && _until && millis() >= _until) stop();
}

void CrSpeaker::setTempo(uint16_t bpm) {
  if (bpm < 20) bpm = 20;
  if (bpm > 300) bpm = 300;
  _tempo = bpm;
}

void CrSpeaker::rest(float beats) {
  stop();
  delay((uint32_t)(beats * 60000.0f / _tempo));
}

static int noteFreq(const char *name) {
  static const char *names[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
  static const char *flats[] = {"C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B"};
  if (name[0] >= '0' && name[0] <= '9') {
    int midi = atoi(name);
    return (int)(440.0f * powf(2.0f, (midi - 69) / 12.0f));
  }
  int semitone = -1;
  int octOff = 1;
  for (int i = 0; i < 12; i++) {
    size_t n = strlen(names[i]);
    if (strncmp(name, names[i], n) == 0) { semitone = i; octOff = (int)n; break; }
    n = strlen(flats[i]);
    if (strncmp(name, flats[i], n) == 0) { semitone = i; octOff = (int)n; break; }
  }
  if (semitone < 0) return 440;
  int oct = atoi(name + octOff);
  int midi = (oct + 1) * 12 + semitone;
  return (int)(440.0f * powf(2.0f, (midi - 69) / 12.0f));
}

void CrSpeaker::note(const char *name, float beats) {
  uint32_t ms = (uint32_t)(beats * 60000.0f / _tempo);
  tone((uint16_t)noteFreq(name), ms);
}

CrIr *CrIr::_inst = nullptr;
static volatile uint32_t irLastUs = 0;
static volatile uint32_t irBits = 0;
static volatile uint8_t irCount = 0;
static volatile bool irReady = false;
static volatile uint32_t irValue = 0;

void IRAM_ATTR CrIr::_isr() {
  uint32_t now = micros();
  uint32_t dt = now - irLastUs;
  irLastUs = now;
  if (dt > 20000) {
    irCount = 0;
    irBits = 0;
    return;
  }
  if (dt > 1500) {
    irBits = (irBits << 1) | 1;
  } else if (dt > 400) {
    irBits = (irBits << 1);
  } else {
    return;
  }
  irCount++;
  if (irCount >= 32) {
    irValue = irBits;
    irReady = true;
    irCount = 0;
    irBits = 0;
  }
}

void CrIr::begin() {
  _inst = this;
  _have = false;
  _addr = _cmd = 0;
  _textLen = 0;
  pinMode(CR_PIN_IR_RX, INPUT);
  pinMode(CR_PIN_IR_TX, OUTPUT);
  digitalWrite(CR_PIN_IR_TX, LOW);
#if CR_LEDC3
  ledcAttach(CR_PIN_IR_TX, 38000, 8);
  ledcWrite(CR_PIN_IR_TX, 0);
#else
  ledcSetup(CR_IR_CH, 38000, 8);
  ledcAttachPin(CR_PIN_IR_TX, CR_IR_CH);
  ledcWrite(CR_IR_CH, 0);
#endif
  attachInterrupt(digitalPinToInterrupt(CR_PIN_IR_RX), CrIr::_isr, CHANGE);
}

void CrIr::_markSpace(bool mark, uint32_t us) {
#if CR_LEDC3
  ledcWrite(CR_PIN_IR_TX, mark ? 128 : 0);
#else
  ledcWrite(CR_IR_CH, mark ? 128 : 0);
#endif
  delayMicroseconds(us);
}

void CrIr::sendNEC(uint16_t address, uint16_t command) {
  noInterrupts();
  _markSpace(true, 9000);
  _markSpace(false, 4500);
  uint32_t frame = ((uint32_t)address) | ((uint32_t)command << 16);
  for (int i = 0; i < 32; i++) {
    bool one = frame & 1;
    frame >>= 1;
    _markSpace(true, 560);
    _markSpace(false, one ? 1690 : 560);
  }
  _markSpace(true, 560);
  _markSpace(false, 0);
  interrupts();
}

void CrIr::sendText(const char *s) {
  while (*s) {
    sendNEC(0x00, (uint8_t)*s);
    delay(40);
    s++;
  }
  sendNEC(0x00, '\n');
}

bool CrIr::_decodeNEC(uint32_t v) {
  _addr = v & 0xFFFF;
  _cmd = (v >> 16) & 0xFFFF;
  return true;
}

bool CrIr::available() {
  bool v = _have;
  _have = false;
  return v;
}

void CrIr::update() {
  if (irReady) {
    irReady = false;
    uint32_t v = irValue;
    _decodeNEC(v);
    _have = true;
    uint8_t b = _cmd & 0xFF;
    if (_textLen < sizeof(_text) - 1) {
      _text[_textLen++] = (char)b;
      _text[_textLen] = 0;
    }
    if (b == '\n') _textLen = 0;
  }
}

int CrIr::readText(char *out, int maxLen) {
  int n = _textLen;
  if (n >= maxLen) n = maxLen - 1;
  if (n < 0) n = 0;
  memcpy(out, _text, n);
  out[n] = 0;
  _have = false;
  return n;
}
