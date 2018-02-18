//
// Copyright 2017, Ricahrd S Allinson.
// Copyrights licensed under the New BSD License.
// See the accompanying LICENSE file for terms.
//

#define TRIGGER_PIN       13 // Digital 13
#define PUSHER_HOME_PIN   12 // Digital 12
#define PUSHER_MOTOR_PIN  11 // Digital 11
#define FULLAUTO_PIN      10 // Digital 10

void setup() {
  pinMode(TRIGGER_PIN, INPUT);
  pinMode(PUSHER_HOME_PIN, INPUT);
  pinMode(PUSHER_MOTOR_PIN, INPUT);
  pinMode(FULLAUTO_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(PUSHER_MOTOR_PIN), pusherStatus, CHANGE);
}

bool triggerActive() {
  return digitalRead(TRIGGER_PIN) == HIGH;
}

bool pusherHomeActive() {
  return digitalRead(PUSHER_HOME_PIN) == HIGH;
}

bool fullautoActive() {
  return digitalRead(FULLAUTO_PIN) == HIGH;
}

void movePusher(bool v) {
  if (v) {
    digitalWrite(PUSHER_MOTOR_PIN, HIGH);
  } else {
    digitalWrite(PUSHER_MOTOR_PIN, LOW);
  }
}

void pusherStatus() {
  // Using fullauto so keep pushing.
  if (fullautoActive() && triggerActive()) {
    return;
  }
  // While the pusher is not home keep it moving.
  if (!pusherHomeActive()) {
     return;
  }
  // The pusher is home so stop it.
  movePusher(false);
  // Wait until the tigger is released.
  while (triggerActive()) {
    delay(100);
  }
}

void loop() {
  if (triggerActive()) {
    movePusher(true);
  }
}

