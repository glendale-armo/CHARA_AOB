#ifndef MONITORS_H
#define MONITORS_H

#include <Arduino.h>
#include "config.h"
#include "driver_io.h"

// Provided by main.ino
extern EthernetClient client;
extern bool pollEnabled[23];

// ─── Motion-state monitor (on-demand poll) ──────────────────────────
static inline void monitorMotionStates() {
  static uint32_t lastPoll = 0;
  static uint8_t  nextId   = 1;
  static uint16_t prev[23]  = {0xFFFF, 0xFFFF, 0xFFFF};

  if (millis() - lastPoll < 50) return;
  lastPoll = millis();

  // find next enabled ID to poll
  uint8_t id = 0;
  for (uint8_t i = 0; i < 22; ++i) {
    uint8_t cand = nextId;
    if (pollEnabled[cand]) { id = cand; break; }
    nextId = (nextId == 22) ? 1 : (uint8_t)(nextId + 1);
  }
  if (!id) return;

  uint16_t ms = readReg(id, 0x1003);
  if (ms != 0xFFFF && (ms == 0x0006 || ms == 0x0032) && ms != prev[id]) {
    // Serial message (unchanged)
    Serial.print("Motor-"); Serial.print(id);
    Serial.print(" motion-state 0x1003 changed: 0x"); Serial.println(ms, HEX);
    // Ethernet echo immediately after
    client.print("Motor-"); client.print(id);
    client.print(" motion-state 0x1003 changed: 0x"); client.println(ms, HEX);
  }
  prev[id] = ms;

  delayMicroseconds(5000);
  nextId = (id == 22) ? 1 : (uint8_t)(id + 1);
}

// ─── Limit-switch polling ONLY for M1 and M2 ────────────────────────
static const uint8_t kLimitPollIds[] = { 1, 2 };
static const uint8_t kLimitPollCount = sizeof(kLimitPollIds) / sizeof(kLimitPollIds[0]);
static uint8_t  lsInited[23]      = {0};
static uint8_t  lsIdleLevel[23]   = {0};
static uint8_t  lsPrevPressed[23] = {0};
static uint32_t lsLastPollMs[23]  = {0};

static inline void monitorLimitSwitches_M12() {
  const uint32_t now = millis();
  for (uint8_t i = 0; i < kLimitPollCount; ++i) {
    const uint8_t id = kLimitPollIds[i];

    if (now - lsLastPollMs[id] < 10) continue;      // per-ID ≈10 ms
    lsLastPollMs[id] = now;

    uint16_t di = readReg(id, 0x0179);              // DI status: bit0..6 = DI1..DI7
    if (di == 0xFFFF) continue;

    uint8_t di2 = (di & 0x0002) ? 1 : 0;            // DI2
    if (!lsInited[id]) {
      lsIdleLevel[id] = di2;                        // learn idle (NO/NC agnostic)
      lsInited[id] = 1;
    }
    uint8_t pressed = (di2 != lsIdleLevel[id]);     // engaged = deviates from idle

    if (pressed && !lsPrevPressed[id]) {
      Serial.print("limit hit for M"); Serial.println(id);
      client.print("limit hit for M"); client.println(id);
    }
    lsPrevPressed[id] = pressed;

    delayMicroseconds(5000);                        // modest RS-485 guard
  }
}

#endif // MONITORS_H
