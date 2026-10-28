# 🚗 Driver Alcohol Detector

> Breath-alcohol ignition interlock with an Arduino Uno and an MQ-3 sensor: the vehicle won't start if the driver has been drinking.

![badge](https://img.shields.io/badge/Arduino%20Uno-00878F?logo=arduino&logoColor=white&style=flat) ![badge](https://img.shields.io/badge/C%2B%2B-00599C?logo=cplusplus&logoColor=white&style=flat) ![badge](https://img.shields.io/badge/Road%20Safety-E34F26?style=flat)
![tests](https://github.com/MelbinManoj/driver-alcohol-detector/actions/workflows/tests.yml/badge.svg)

> 📝 **About this code:** I built this project during my B.Tech, but the original source code was lost. The code here is a **rebuilt version (2026)** based on the original design. It compiles for the Arduino Uno, and its logic is checked by automated tests that run on simulated sensor data. It has not yet been re-tested on the physical hardware.

## 📖 Overview

Drunk driving causes many road accidents. This system measures alcohol in the driver's breath with an **MQ-3 gas sensor**. If the level is too high, it **cuts the ignition relay** and sounds an alarm. The vehicle stays locked until the driver gives a clean reading for 30 seconds in a row.

## ✨ Features

- Ignition stays **locked during the sensor's 60 s warm-up**, so it fails safe
- Moving average over 8 samples, so a single noise spike can't trigger or clear the lock
- Two levels: **WARNING** (beeps, ignition still allowed) and **LOCKED** (ignition cut, continuous alarm)
- The lock **latches**: it needs 30 s of continuous clean readings, and the timer restarts if the level rises again
- Green and red LEDs and a buzzer show the state; live readings on the Serial Monitor

**States:** `WARMUP` → `SAFE` ⇄ `WARNING` → `LOCKED` → (30 s clean) → `SAFE`

## 🧰 Components

| Component | Qty |
|---|:---:|
| Arduino Uno | 1 |
| MQ-3 alcohol sensor module | 1 |
| 5 V relay module (ignition interlock) | 1 |
| Buzzer | 1 |
| Red + green LEDs with 220 Ω resistors | 2 |
| Jumper wires | — |

## 🔌 Wiring

| Connection | Arduino pin |
|---|---|
| MQ-3 AOUT | A0 |
| MQ-3 VCC / GND | 5V / GND |
| Ignition relay IN | D7 (active LOW, ON = ignition enabled) |
| Buzzer | D8 |
| Red LED | D9 |
| Green LED | D10 |

## 🚀 Getting Started

1. Open [`driver_alcohol_detector/driver_alcohol_detector.ino`](./driver_alcohol_detector/driver_alcohol_detector.ino) in the Arduino IDE. The `alcohol_monitor.h` file in the same folder opens with it.
2. Select **Arduino Uno** (or Nano) and upload. No extra libraries are needed.
3. Open the Serial Monitor to watch the live readings.
4. **Calibrate:** MQ-3 modules vary a lot. Watch `level=` on the Serial Monitor in clean air and near a cotton swab with hand sanitiser, then adjust `warnLevel` / `lockLevel`.

## 🧪 Tests

All decision logic lives in [`alcohol_monitor.h`](./driver_alcohol_detector/alcohol_monitor.h), with no Arduino dependencies, so it can be tested on a PC with simulated sensor data:

```bash
cd test
g++ -std=c++11 -I../driver_alcohol_detector test_alcohol_monitor.cpp -o test && ./test
```

What's tested:

- ✅ Ignition locked during warm-up even in clean air
- ✅ Moderate alcohol gives a warning but allows ignition
- ✅ High alcohol locks the ignition
- ✅ Lock only clears after 30 s of continuous clean readings, and the timer resets if the level rises
- ✅ A single noise spike is smoothed out

These tests run automatically on every push with GitHub Actions.

## 📂 Project Structure

```text
driver-alcohol-detector/
├── driver_alcohol_detector/
│   ├── driver_alcohol_detector.ino      # hardware layer: pins, sensors, outputs
│   └── alcohol_monitor.h     # decision logic (unit-tested)
├── test/
│   └── test_alcohol_monitor.cpp
└── README.md
```

## 👤 Author

**Melbin Manoj** — [GitHub](https://github.com/MelbinManoj) · [LinkedIn](https://www.linkedin.com/in/melbin-manoj-8ba512269/)
