# CHARA_AOB

ClearCore control code for DM556RS stepper drivers

This README explains the command format and what each file does.

---

## Motor control commands

Move

```
m<id>, <steps>
```

Example:
`m1, 10000` → move Motor 1 forward 10,000 steps
`m1, -8000` → move Motor 1 backward 8,000 steps

Stop one motor

```
m<id>, s
```

Stop all motors

```
stop all
```

Send multiple commands in one line (batch)

```
m1, 10000 + m2, 10000
```

Note: do **not** put two commands for the **same** motor in one line. The second will preempt the first before it finishes.

Enable / disable motor-state polling (per motor)

```
m<id>, st t   // enable
m<id>, st f   // disable
```

Tip: avoid turning polling on for many motors at once on a slow bus.

Fan control

```
fg   // fan on
fs   // fan off
```

---

## Files

* **CHARA_AOB_V1.ino**
  Main sketch. Sets up Ethernet and serial, starts services, runs the loop (reads TCP, parses commands, polls state, auto-disables idle motors).

* **EthernetTCP.h**
  ClearCore TCP helpers used by the sketch.

* **config.h**
  Project settings: ClearCore IP/port, baud rate, motion presets (speed, acceleration, deceleration), pin selection.

* **dm_556_rs_constants.h**
  DM556RS driver constants (register addresses and values used by the code).

* **dm_556_rs_frames.h**
  Low-level helpers that build Modbus RTU frames (read/write, PR0 position split high/low, trigger). Used by `driver_io.h`.

* **driver_io.h**
  High-level driver actions built on the frames: enable/disable, configure PR0 (mode/velocity/accel/decel), send a relative move, quick stop, single-register reads.

* **fan.h**
  Fan on/off and simple state-change printout.

* **monitors.h**
  Motor-state printing (reads the driver’s motion state and logs changes) and basic limit-switch polling for M1/M2.

* **motor_ids.h**
  `constexpr` slave IDs for Motors 1–22.

* **motor_init.h**
  Initializes all 22 motors (maps inputs, sets PR0 parameters, saves, then disables).

* **motor_state.cpp**
  Defines the `MotorState motors[22]` array with default values.

* **parse.h**
  Tokenizer and command parser for lines received over Ethernet. Supports move, stop one, stop all, polling toggle, fan, and `+` batching.

* **runtime_state.h**
  Declares `struct MotorState`, `extern` array, and `mById()` so types come before the array definition in the build order.
