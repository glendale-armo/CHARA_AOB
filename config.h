#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Network
#define PORT_NUM            8888
#define MAX_PACKET_LENGTH   256
static inline IPAddress deviceIP() { return IPAddress(192, 168, 0, 123); }

// RS-485 / Modbus
#define SerialPort          Serial1
#define MODBUS_BAUD         19200UL

// PR0 motion presets
#define RPM                 50u           // PR0 velocity (RPM)
#define ACCEL               50u           // ms per 1000 RPM
#define DECEL               50u           // ms per 1000 RPM

// Auto-disable
#define DISABLE_TIMEOUT_MS  10000UL

// Fan PWM
#define FAN_PWM_PIN         IO0
#define FAN_PRESET          60

#endif // CONFIG_H
