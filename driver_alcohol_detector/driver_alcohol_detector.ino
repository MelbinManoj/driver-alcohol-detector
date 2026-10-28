// Driver Alcohol Detector - Arduino Uno
// An MQ-3 alcohol sensor measures alcohol in the driver's breath. Above the
// lock level the ignition relay is cut and a buzzer sounds; the vehicle stays
// locked until a clean breath reading is held for 30 seconds.
//
// Wiring:
//   MQ-3 module AOUT -> A0, VCC -> 5V, GND -> GND
//   Ignition relay IN -> D7 (active LOW; relay ON = ignition enabled)
//   Buzzer -> D8, Red LED -> D9, Green LED -> D10 (each LED with a 220 ohm resistor)

#include "alcohol_monitor.h"

const uint8_t SENSOR_PIN = A0;
const uint8_t RELAY_PIN = 7;
const uint8_t BUZZER_PIN = 8;
const uint8_t RED_LED = 9;
const uint8_t GREEN_LED = 10;
const bool RELAY_ACTIVE_LOW = true;

// Thresholds are raw ADC values; calibrate them for your MQ-3 module.
AlcoholConfig config = {
  60000UL,  // 60 s heater warm-up
  350,      // warning level
  500,      // lock level
  30000UL   // 30 s of clean readings to unlock
};

AlcoholMonitor monitor(config);
unsigned long lastSample = 0;
unsigned long lastPrint = 0;

void setIgnition(bool enabled) {
  digitalWrite(RELAY_PIN, (enabled ^ RELAY_ACTIVE_LOW) ? HIGH : LOW);
}

void showState(AlcoholState s, unsigned long now) {
  bool blink = (now / 250) % 2;
  switch (s) {
    case WARMUP:   // green blinks slowly while warming up
      digitalWrite(GREEN_LED, (now / 500) % 2); digitalWrite(RED_LED, LOW); noTone(BUZZER_PIN); break;
    case SAFE:
      digitalWrite(GREEN_LED, HIGH); digitalWrite(RED_LED, LOW); noTone(BUZZER_PIN); break;
    case WARNING:  // red blinks, short beeps
      digitalWrite(GREEN_LED, LOW); digitalWrite(RED_LED, blink);
      if (blink) tone(BUZZER_PIN, 1500); else noTone(BUZZER_PIN);
      break;
    case LOCKED:   // red solid, continuous alarm
      digitalWrite(GREEN_LED, LOW); digitalWrite(RED_LED, HIGH); tone(BUZZER_PIN, 2500); break;
  }
}

const char* stateName(AlcoholState s) {
  switch (s) {
    case WARMUP: return "WARMUP";
    case SAFE: return "SAFE";
    case WARNING: return "WARNING";
    default: return "LOCKED";
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  setIgnition(false);   // locked until the sensor has warmed up
  Serial.println(F("Driver Alcohol Detector - warming up sensor..."));
}

void loop() {
  unsigned long now = millis();
  if (now - lastSample >= 100) {   // 10 samples per second
    lastSample = now;
    monitor.update(analogRead(SENSOR_PIN), now);
    setIgnition(monitor.ignitionAllowed());
  }
  showState(monitor.getState(), now);

  if (now - lastPrint >= 1000) {
    lastPrint = now;
    Serial.print(F("level="));
    Serial.print(monitor.average());
    Serial.print(F(" state="));
    Serial.print(stateName(monitor.getState()));
    Serial.print(F(" ignition="));
    Serial.println(monitor.ignitionAllowed() ? F("ENABLED") : F("LOCKED"));
  }
}
