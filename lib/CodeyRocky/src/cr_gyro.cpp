#include "CodeyRocky.h"
#include <Wire.h>
#include <math.h>

bool CrGyro::_writeReg(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(CR_IMU_ADDR);
  Wire.write(reg);
  Wire.write(val);
  return Wire.endTransmission() == 0;
}

bool CrGyro::_readBytes(uint8_t reg, uint8_t *buf, uint8_t n) {
  Wire.beginTransmission(CR_IMU_ADDR);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;
  uint8_t got = Wire.requestFrom((int)CR_IMU_ADDR, (int)n);
  if (got != n) return false;
  for (uint8_t i = 0; i < n; i++) buf[i] = Wire.read();
  return true;
}

bool CrGyro::begin() {
  _ok = false;
  _ax = _ay = _az = _gx = _gy = _gz = _pitch = _roll = _yaw = _temp = 0;
  _offX = _offY = _offZ = 0;
  _lastUs = micros();
  Wire.begin(CR_PIN_IMU_SDA, CR_PIN_IMU_SCL);
  Wire.setClock(100000);
  delay(50);
  uint8_t id = 0;
  for (int i = 0; i < 8; i++) {
    if (_readBytes(0x75, &id, 1)) break;
    delay(20);
  }
  _who = id;
  if (id != 0x98 && id != 0x68 && id != 0x70 && id != 0x71 && id != 0x73) {
    return false;
  }
  _writeReg(0x6B, 0x00); // wake
  delay(50);
  _writeReg(0x1A, 0x01); // DLPF
  delay(20);
  _writeReg(0x1B, 0x08); // gyro ±500 dps
  delay(20);
  _writeReg(0x19, 19);   // 50 Hz sample
  delay(20);
  _ok = true;
  calibrate();
  return true;
}

void CrGyro::calibrate() {
  if (!_ok) return;
  long sx = 0, sy = 0, sz = 0;
  uint8_t raw[14];
  int n = 0;
  for (int i = 0; i < 80; i++) {
    if (_readBytes(0x3B, raw, 14)) {
      if (i >= 10) {
        sx += (int16_t)((raw[8] << 8) | raw[9]);
        sy += (int16_t)((raw[10] << 8) | raw[11]);
        sz += (int16_t)((raw[12] << 8) | raw[13]);
        n++;
      }
    }
    delay(8);
  }
  if (n) {
    _offX = sx / (float)n;
    _offY = sy / (float)n;
    _offZ = sz / (float)n;
  }
}

void CrGyro::update() {
  if (!_ok) return;
  uint8_t raw[14];
  if (!_readBytes(0x3B, raw, 14)) return;
  int16_t ax = (int16_t)((raw[0] << 8) | raw[1]);
  int16_t ay = (int16_t)((raw[2] << 8) | raw[3]);
  int16_t az = (int16_t)((raw[4] << 8) | raw[5]);
  int16_t t  = (int16_t)((raw[6] << 8) | raw[7]);
  int16_t gx = (int16_t)((raw[8] << 8) | raw[9]);
  int16_t gy = (int16_t)((raw[10] << 8) | raw[11]);
  int16_t gz = (int16_t)((raw[12] << 8) | raw[13]);

  _ax = ax / 16384.0f * 9.81f;
  _ay = ay / 16384.0f * 9.81f;
  _az = az / 16384.0f * 9.81f;
  _temp = t / 340.0f + 36.53f;
  const float sens = 65.5f; // 500 dps
  _gx = (gx - _offX) / sens;
  _gy = (gy - _offY) / sens;
  _gz = (gz - _offZ) / sens;

  uint32_t now = micros();
  float dt = (now - _lastUs) / 1000000.0f;
  if (dt <= 0 || dt > 0.2f) dt = 0.02f;
  _lastUs = now;

  float accPitch = atan2f(-_ax, sqrtf(_ay * _ay + _az * _az)) * 57.2958f;
  float accRoll  = atan2f(_ay, _az) * 57.2958f;
  _pitch = 0.96f * (_pitch + _gy * dt) + 0.04f * accPitch;
  _roll  = 0.96f * (_roll + _gx * dt) + 0.04f * accRoll;
  _yaw  += _gz * dt;
  if (_yaw < 0) _yaw += 360;
  if (_yaw >= 360) _yaw -= 360;
}

bool CrGyro::isShaking() const {
  float mag = sqrtf(_gx * _gx + _gy * _gy + _gz * _gz);
  return mag > 80.0f;
}

bool CrGyro::isTilted(CrTilt dir, float threshold) const {
  switch (dir) {
    case TILT_FORWARD: return _pitch > threshold;
    case TILT_BACKWARD: return _pitch < -threshold;
    case TILT_LEFT: return _roll > threshold;
    case TILT_RIGHT: return _roll < -threshold;
  }
  return false;
}
