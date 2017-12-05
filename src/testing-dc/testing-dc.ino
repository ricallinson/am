//
// Copyright 2017, Ricahrd S Allinson.
// Copyrights licensed under the New BSD License.
// See the accompanying LICENSE file for terms.
//

#include <Servo.h>

#define LED_PIN              13 // Digital 13
#define FLYWHEEL_BUTTON_PIN  12 // Digital 12
#define FLYWHEEL_PIN         10 // Digital 10

#define THROTTLE_VALUE       1 // 0.5 == 50%
#define FLYWHEEL_MIN_VALUE   1060
#define FLYWHEEL_MAX_VALUE   1860
#define FLYWHEEL_SPIN_VALUE  FLYWHEEL_MIN_VALUE + ((FLYWHEEL_MAX_VALUE - FLYWHEEL_MIN_VALUE) * THROTTLE_VALUE)

Servo flywheels;
bool flywheelsSpinning;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(FLYWHEEL_BUTTON_PIN, INPUT);
  pinMode(FLYWHEEL_PIN, OUTPUT);
  flywheels.attach(FLYWHEEL_PIN);
  calibrateFlywheels();
}

// Used by the Afro ESC 12A Speed Controller.
// This does not work if the Arduino is powered via USB.
void calibrateFlywheels() {
  digitalWrite(LED_PIN, HIGH);
  flywheels.writeMicroseconds(FLYWHEEL_MAX_VALUE);
  delay(1000);
  flywheels.writeMicroseconds(FLYWHEEL_MIN_VALUE);
  delay(2000);
  digitalWrite(LED_PIN, LOW);
}

bool flywheelsActive() {
  return digitalRead(FLYWHEEL_BUTTON_PIN) == HIGH;
}

void loop() {
  if (flywheelsActive()) {
    if (!flywheelsSpinning) {
      flywheels.writeMicroseconds(FLYWHEEL_SPIN_VALUE - 600);
      delay(1000);
      flywheels.writeMicroseconds(FLYWHEEL_SPIN_VALUE - 400);
      delay(1000);
      flywheels.writeMicroseconds(FLYWHEEL_SPIN_VALUE - 200);
      delay(1000);
    }
    flywheels.writeMicroseconds(FLYWHEEL_SPIN_VALUE);
    flywheelsSpinning = true;
    digitalWrite(LED_PIN, HIGH);
  } else {
    flywheels.writeMicroseconds(FLYWHEEL_MIN_VALUE);
    flywheelsSpinning = false;
    digitalWrite(LED_PIN, LOW);
  }
}

