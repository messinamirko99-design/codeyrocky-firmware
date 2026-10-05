#pragma once
// Pin map extracted from Makeblock Codey Rocky firmware
// https://github.com/FFtust/micropython_esp32_local

#include <Arduino.h>

// Power — GPIO15 MUST be driven HIGH immediately or the board shuts off.
#define CR_PIN_POWER_HOLD     15
#define CR_PIN_POWER_KEY      34   // ADC, active LOW when pressed

// Buttons: A and B are active HIGH (external pull-down), C is active LOW (pull-up).
#define CR_PIN_BTN_A           2
#define CR_PIN_BTN_B          12
#define CR_PIN_BTN_C           0

// RGB LED (PWM)
#define CR_PIN_LED_R          14
#define CR_PIN_LED_G          21
#define CR_PIN_LED_B           4

// LED matrix TM1640-compatible bitbang (16 columns x 8 rows)
#define CR_PIN_MATRIX_SDA     22
#define CR_PIN_MATRIX_SCL     23

// MPU-compatible 6-axis IMU on I2C
#define CR_PIN_IMU_SDA        19
#define CR_PIN_IMU_SCL        18
#define CR_IMU_ADDR         0x68
#define CR_IMU_WHOAMI       0x98   // Codey clone; also accept 0x68/0x70/0x71

// Analog sensors
#define CR_PIN_SOUND          36   // ADC1_CH0
#define CR_PIN_KNOB           39   // ADC1_CH3  gear potentiometer
#define CR_PIN_HW_VER         32   // ADC1_CH4
#define CR_PIN_BATTERY        33   // ADC1_CH5
#define CR_PIN_LIGHT          35   // ADC1_CH7

// Speaker DAC / PWM
#define CR_PIN_SPEAKER        25   // DAC_CHANNEL_1

// IR (RMT) — stock firmware (non V1.07)
#define CR_PIN_IR_TX           5
#define CR_PIN_IR_RX          26

// Rocky chassis UART1
#define CR_PIN_ROCKY_TX       17
#define CR_PIN_ROCKY_RX       16
#define CR_ROCKY_BAUD         115200

#define CR_MATRIX_COLS        16
#define CR_MATRIX_ROWS         8
