// Pure decision logic for the driver alcohol detector.
// No Arduino dependencies, so it can be unit-tested on a PC.
#ifndef ALCOHOL_MONITOR_H
#define ALCOHOL_MONITOR_H

#include <stdint.h>

struct AlcoholConfig {
  uint32_t warmupMs;      // MQ-3 heater warm-up time; ignition locked meanwhile
  int warnLevel;          // averaged ADC level that triggers a warning
  int lockLevel;          // averaged ADC level that locks the ignition
  uint32_t clearMs;       // reading must stay below warnLevel this long to unlock
};

enum AlcoholState { WARMUP, SAFE, WARNING, LOCKED };

class AlcoholMonitor {
 public:
  static const int WINDOW = 8;   // moving-average window

  explicit AlcoholMonitor(const AlcoholConfig& c)
      : cfg(c), state(WARMUP), count(0), index(0), sum(0), cleanSince(0), cleanTiming(false) {
    for (int i = 0; i < WINDOW; i++) samples[i] = 0;
  }

  // Feed one raw ADC sample (0-1023) and the current time.
  AlcoholState update(int raw, uint32_t nowMs) {
    sum -= samples[index];
    samples[index] = raw;
    sum += raw;
    index = (index + 1) % WINDOW;
    if (count < WINDOW) count++;

    if (nowMs < cfg.warmupMs && state == WARMUP) return state;

    int avg = average();
    if (state == LOCKED) {
      // Stay locked until the driver gives a clean reading for clearMs.
      if (avg < cfg.warnLevel) {
        if (!cleanTiming) { cleanTiming = true; cleanSince = nowMs; }
        if (nowMs - cleanSince >= cfg.clearMs) { state = SAFE; cleanTiming = false; }
      } else {
        cleanTiming = false;
      }
      return state;
    }

    if (avg >= cfg.lockLevel) { state = LOCKED; cleanTiming = false; }
    else if (avg >= cfg.warnLevel) state = WARNING;
    else state = SAFE;
    return state;
  }

  int average() const { return count ? (int)(sum / count) : 0; }
  AlcoholState getState() const { return state; }
  bool ignitionAllowed() const { return state == SAFE || state == WARNING; }

 private:
  AlcoholConfig cfg;
  AlcoholState state;
  int samples[WINDOW];
  int count, index;
  long sum;
  uint32_t cleanSince;
  bool cleanTiming;
};

#endif
