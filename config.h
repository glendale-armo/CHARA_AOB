#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <Ethernet.h>

// Network
#define PORT_NUM            8888
#define MAX_PACKET_LENGTH   256
static inline IPAddress deviceIP() { return IPAddress(192, 168, 0, 123); }

// RS-485 / Modbus
// COM-1 remains the primary. COM-0 is added transparently in driver_io.h.
#define SerialPort          Serial1
#define MODBUS_BAUD         19200UL

// Secondary RS-485 bus enable (COM-0). Set to 0 to disable COM-0.
#define USE_COM0            1
#define SerialPortA         Serial1   // COM-1
#define SerialPortB         Serial0   // COM-0

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
