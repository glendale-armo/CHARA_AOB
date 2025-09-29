#ifndef PARSE_H
#define PARSE_H

#include <Arduino.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "config.h"
#include "driver_io.h"

// Provided by main.ino
extern EthernetClient client;
extern uint8_t  fanSetpoint;
extern bool     pollEnabled[23];

// ─── helpers ─────────────────────────────────────────────────────────
static inline bool ieq2(const char *a, char b0, char b1) {
  return a && (tolower(a[0]) == tolower(b0)) && (tolower(a[1]) == tolower(b1)) && a[2] == '\0';
}
static inline bool ieq1(const char *a, char b0) {
  return a && (tolower(a[0]) == tolower(b0)) && a[1] == '\0';
}
static inline bool ieqStr(const char *a, const char *b) {
  if (!a || !b) return false;
  while (*a && *b) {
    if (tolower(*a++) != tolower(*b++)) return false;
  }
  return *a == '\0' && *b == '\0';
}

// ─── Single token parser ────────────────────────────────────────────
static inline void parseSingle(char *cmd) {
  if (!cmd || !*cmd) return;

  // global: "stop all"
  if (ieqStr(cmd, "stop all")) {
    for (uint8_t id = 1; id <= 22; ++id) stopMotor(id);
    Serial.println("All motors quick stop");
    client.println("All motors quick stop");
    return;
  }

  // fan commands: FG / FS (exact 2 chars)
  if ((cmd[0] == 'F' || cmd[0] == 'f') && cmd[2] == '\0') {
    if (cmd[1] == 'G' || cmd[1] == 'g') { fanSetpoint = FAN_PRESET; return; }
    if (cmd[1] == 'S' || cmd[1] == 's') { fanSetpoint = 0;          return; }
  }

  // Expect "Mx ..." where x is [1..22]
  // Tokenize by whitespace or comma
  char *tok = strtok(cmd, " ,\t");
  if (!tok || (tok[0] != 'M' && tok[0] != 'm')) return;
  uint8_t id = atoi(tok + 1);
  if (id < 1 || id > 22) return;

  char *t1 = strtok(nullptr, " ,\t");
  if (!t1) return;

  // Branch: "st t|f" to toggle polling
  if (ieq2(t1, 's', 't')) {
    char *t2 = strtok(nullptr, " ,\t");
    if (!t2) return;
    if (ieq1(t2, 't')) {
      pollEnabled[id] = true;
      Serial.print("Motor-"); Serial.print(id); Serial.println(" state polling: ON");
      client.print("Motor-"); client.print(id); client.println(" state polling: ON");
    } else if (ieq1(t2, 'f')) {
      pollEnabled[id] = false;
      Serial.print("Motor-"); Serial.print(id); Serial.println(" state polling: OFF");
      client.print("Motor-"); client.print(id); client.println(" state polling: OFF");
    }
    return;
  }

  // New: "s" → quick stop (PR control 0x6002 ← 0x0040)
  if (ieq1(t1, 's')) {
    stopMotor(id);
    Serial.print("Motor-"); Serial.print(id); Serial.println(" quick stop");
    client.print("Motor-"); client.print(id); client.println(" quick stop");
    return;
  }

  // Default: treat t1 as steps (e.g., "M1,1000" or "M1 1000")
  long steps = atol(t1);
  enableMotorHW(id);
  moveMotor(id, steps);
}

// ─── Line parser & inline sequencer (delimiter '+') ─────────────────
static inline void parseLine(char *line) {
  char token[64];
  uint8_t idx = 0;
  for (char *p = line; *p; ++p) {
    char c = *p;
    if (c == '+' || c == '\n' || c == '\r') {
      if (idx) { token[idx] = '\0'; parseSingle(token); idx = 0; }
      continue;
    }
    // keep spaces so sub-tokens like "st t" survive
    if (idx < sizeof(token) - 1) token[idx++] = c;
  }
  if (idx) { token[idx] = '\0'; parseSingle(token); }
}

#endif // PARSE_H
