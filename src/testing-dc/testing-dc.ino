//
// Copyright 2017, Ricahrd S Allinson.
// Copyrights licensed under the New BSD License.
// See the accompanying LICENSE file for terms.
//

#include <Servo.h>

#define LED_PIN              13 // Digital 13
#define FLYWHEEL_BUTTON_PIN  12 // Digital 12
#define PUSHER_PIN           11 // Digital 11
#define FLYWHEEL_PIN         10 // Digital 10


#define PUSHER_MIN_VALUE      0
#define PUSHER_MAX_VALUE      255
#define FLYWHEEL_MIN_VALUE    1060
#define FLYWHEEL_MAX_VALUE    1860

Servo flywheels;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(FLYWHEEL_BUTTON_PIN, INPUT);
  pinMode(PUSHER_PIN, OUTPUT);
  pinMode(FLYWHEEL_PIN, OUTPUT);
  flywheels.attach(FLYWHEEL_PIN);
}

bool flywheelsActive() {
  return digitalRead(FLYWHEEL_BUTTON_PIN) == HIGH;
}

void loop() {
  if (flywheelsActive()) {
    digitalWrite(LED_PIN, HIGH);
    flywheels.writeMicroseconds(FLYWHEEL_MAX_VALUE / 4);
  } else {
    digitalWrite(LED_PIN, LOW);
    flywheels.writeMicroseconds(FLYWHEEL_MIN_VALUE);
  }
  analogWrite(PUSHER_PIN, PUSHER_MAX_VALUE / 4);
}

