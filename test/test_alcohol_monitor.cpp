// Host-side unit tests for the alcohol monitor logic.
// Build & run:  g++ -std=c++11 -I../driver_alcohol_detector test_alcohol_monitor.cpp -o t && ./t
#include <cstdio>
#include "alcohol_monitor.h"

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); failures++; } } while (0)

static AlcoholConfig cfg() { return {60000UL, 350, 500, 30000UL}; }

// feed a constant reading every 100 ms for `ms` milliseconds
static uint32_t feed(AlcoholMonitor& m, int raw, uint32_t t, uint32_t ms) {
  for (uint32_t end = t + ms; t < end; t += 100) m.update(raw, t);
  return t;
}

int main() {
  // locked during warm-up even with clean air
  { AlcoholMonitor m(cfg());
    uint32_t t = feed(m, 100, 0, 30000);
    CHECK(m.getState() == WARMUP); CHECK(!m.ignitionAllowed());
    t = feed(m, 100, t, 31000);
    CHECK(m.getState() == SAFE); CHECK(m.ignitionAllowed()); }

  // moderate reading -> warning but ignition still allowed
  { AlcoholMonitor m(cfg());
    uint32_t t = feed(m, 100, 0, 61000);
    feed(m, 400, t, 2000);
    CHECK(m.getState() == WARNING); CHECK(m.ignitionAllowed()); }

  // high reading -> locked; stays locked even after a short clean reading
  { AlcoholMonitor m(cfg());
    uint32_t t = feed(m, 100, 0, 61000);
    t = feed(m, 700, t, 2000);
    CHECK(m.getState() == LOCKED); CHECK(!m.ignitionAllowed());
    t = feed(m, 100, t, 10000);
    CHECK(m.getState() == LOCKED);              // only ~10 s clean
    t = feed(m, 400, t, 2000);                  // goes back up -> timer resets
    t = feed(m, 100, t, 25000);
    CHECK(m.getState() == LOCKED);              // 25 s < 30 s after reset
    t = feed(m, 100, t, 6000);
    CHECK(m.getState() == SAFE); CHECK(m.ignitionAllowed()); }

  // single noise spike is smoothed out by the moving average
  { AlcoholMonitor m(cfg());
    uint32_t t = feed(m, 100, 0, 61000);
    m.update(900, t);                           // one spike
    CHECK(m.average() < 350); CHECK(m.getState() == SAFE); }

  if (failures == 0) printf("All alcohol monitor tests passed.\n");
  return failures == 0 ? 0 : 1;
}
