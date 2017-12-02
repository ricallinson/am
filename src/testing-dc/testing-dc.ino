//
// Copyright 2017, Ricahrd S Allinson.
// Copyrights licensed under the New BSD License.
// See the accompanying LICENSE file for terms.
//

#include <Servo.h>

#define PUSHER_PIN           A2 // Analog 2
#define PUSHER_BUTTON_PIN    9  // Digital 9
#define FLYWHEEL_BUTTON_PIN  10 // Digital 10
#define FLYWHEEL_PIN         11 // Digital 11

#define PUSHER_MIN_VALUE      0
#define PUSHER_MAX_VALUE      255
#define FLYWHEEL_MIN_VALUE    1060
#define FLYWHEEL_MAX_VALUE    1860

Servo flywheels;

void setup() {
  pinMode(PUSHER_PIN, OUTPUT);
  flywheels.attach(FLYWHEEL_PIN);
}

bool flywheelsActive() {
  return digitalRead(FLYWHEEL_BUTTON_PIN) == HIGH;
}

bool pusherActive() {
  return digitalRead(PUSHER_BUTTON_PIN) == HIGH;
}

void loop() {
  if (flywheelsActive()) {
    flywheels.writeMicroseconds(FLYWHEEL_MAX_VALUE);
  } else {
    flywheels.writeMicroseconds(FLYWHEEL_MIN_VALUE);
  }
  if (pusherActive()) {
    analogWrite(PUSHER_PIN, PUSHER_MAX_VALUE);
  } else {
    analogWrite(PUSHER_PIN, PUSHER_MIN_VALUE);
  }
}

