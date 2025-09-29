#ifndef RUNTIME_STATE_H
#define RUNTIME_STATE_H
#include <Arduino.h>

struct MotorState {
  uint8_t  id;
  bool     enabled;
  uint32_t lastMoveMs;
};

extern MotorState motors[22];

// Provided by main (or keep as-is if you already define it there)
MotorState &mById(uint8_t id);

#endif
