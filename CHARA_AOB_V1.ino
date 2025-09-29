/*********************************************************************
 * ClearCore ⇆ DM556RS — Ethernet-driven motion demo + Fan toggle (v1.5)
 * -------------------------------------------------------------------
 * Change: on-demand motion-state polling via "Mx st t|f" commands.
 * ETH echo added: mirror the same Serial messages to Ethernet, unguarded.
 *********************************************************************/

#include <SPI.h>
#include <Ethernet.h>
#include "ClearCore.h"
#include "EthernetTcpServer.h"

#include "config.h"
#include "dm_556_rs_constants.h"
#include "runtime_state.h"
#include "dm_556_rs_frames.h"
#include "motor_ids.h"
#include "driver_io.h"
#include "parse.h"
#include "monitors.h"
#include "motor_init.h"
#include "fan.h"

/* Helper with external linkage (no 'static') */
MotorState &mById(uint8_t id) { return motors[id - 1]; }

/* ─── Ethernet server and RX line buffer ───────────────────────────── */
static unsigned char packetReceived[MAX_PACKET_LENGTH];
static EthernetServer server(PORT_NUM);
EthernetClient client;

/* ─── Fan control (PWM on IO0) ─────────────────────────────────────── */
uint8_t  fanSetpoint = 0;    // 0..255
bool     fanWasOn = false;

/* ─── Motion-state polling enable flags (per motor) ────────────────── */
bool pollEnabled[23] = {false, false, false};

/* ─── setup() — hardware bring-up ──────────────────────────────────── */
void setup() {
  Serial.begin(9600);
  uint32_t t0 = millis();
  while (!Serial && millis() - t0 < 3000) {}

  pinMode(IO1, OUTPUT);
  fanSetup();

  while (Ethernet.linkStatus() == LinkOFF) delay(500);
  byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
  Ethernet.begin(mac, deviceIP());
  server.begin();

  // RS-485 buses: COM-1 always; COM-0 optional via USE_COM0 in config.h
  SerialPort.begin(MODBUS_BAUD);   // COM-1 (Serial1)
#if USE_COM0
  Serial0.begin(MODBUS_BAUD);      // COM-0 (Serial0)
#endif

  delay(500);
  initAllDrivers();

  Serial.println("System ready. Commands: Mx,steps | Mx st t|f | Mx,s | stop all | FG / FS");
  client.println("System ready. Commands: Mx,steps | Mx st t|f | Mx,s | stop all | FG / FS");
}

/* ─── loop() — cooperative scheduler ───────────────────────────────── */
void loop() {
  fanRefresh();
  monitorLimitSwitches_M12();   // ONLY M1 and M2
  monitorMotionStates();

  // Accept a newly connected client once; persist it
  EthernetClient nc = server.accept();
  if (nc.connected()) {                    // active connection delivered once per attempt
    if (client.connected()) { client.stop(); } // drop prior client if any
    client = nc;                           // store persistent client
    digitalWrite(IO1, HIGH);
  }

  // Read a line if any bytes are waiting; sending is now allowed any time while Connected()
  if (client.connected()) {
    int len = 0;
    while (client.available() > 0 && len < MAX_PACKET_LENGTH - 1) {
      int16_t r = client.read();
      if (r < 0) break;
      char c = static_cast<char>(r);
      packetReceived[len++] = c;
      if (c == '\n' || c == '\r') break;
    }
    if (len) {
      packetReceived[len] = '\0';
      parseLine(reinterpret_cast<char *>(packetReceived));
      memset(packetReceived, 0, len);
    }
  } else {
    digitalWrite(IO1, LOW);
    // no .stop(); use Close() when you explicitly want to terminate
  }

  uint32_t now = millis();
  for (uint8_t id = 1; id <= 22; ++id) {
    MotorState &m = mById(id);
    if (m.enabled && now - m.lastMoveMs >= DISABLE_TIMEOUT_MS)
      disableMotorHW(id);
  }
  // Required for LwIP/TCP housekeeping
  EthernetMgr.Refresh();
}
