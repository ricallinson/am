//
// Copyright 2017, Ricahrd S Allinson.
// Copyrights licensed under the New BSD License.
// See the accompanying LICENSE file for terms.
//

#include <SPI.h>
#include <Wire.h>
#include <Servo.h>

// Arduino constant pins.
#define VOLTAGE_PIN           A2  // Analog 2
#define SCREEN_DATA_PIN       A3  // Analog 3
                                  // Analog 4 SDA
                                  // Analog 5 SCL
#define PUSHER_MOTOR_PIN      A6  // Analog 6
#define LOWER_FLYWHEEL_PIN    7   // Digital 7
#define UPPER_FLYWHEEL_PIN    8   // Digital 8
#define PUSHER_EXTENDED_PIN   9   // Digital 9
#define MAG_INSERTED_PIN      10  // Digital 10
#define TRIGGER_PIN           11  // Digital 11
#define SCREEN_SELECT_PIN     12  // Digital 12

// Internal constants.
#define VOLTAGE_MIN 9.5
#define VOLTAGE_MAX 12.6
#define VOLTAGE_SAMPLE_SET     10
#define FULL_AUTO              4
#define FLYWHEEL_MIN_VALUE     1060
#define FLYWHEEL_MAX_VALUE     1860
#define FLYWHEEL_SPINUP_TIME   250
#define FLYWHEEL_SPIN_TIME     3000

// Internal state
float voltageSamples[VOLTAGE_SAMPLE_SET];
int voltageSamplesIndex;
int numOfDartsToPush = 1;
int flywheelsActivationTime;
int flywheelSpeed;
int dartsRemaining;
int currentScreenId;
float voltage;

// User settings.
int magSize;
int fireMode;
int dartFps;

// Brushless DC motors used for flywheels.
Servo upperFlywheel;
Servo lowerFlywheel;

void setupFlywheels() {
  upperFlywheel.writeMicroseconds(FLYWHEEL_MAX_VALUE);
  lowerFlywheel.writeMicroseconds(FLYWHEEL_MAX_VALUE);
  delay(1000);
  upperFlywheel.writeMicroseconds(FLYWHEEL_MIN_VALUE);
  lowerFlywheel.writeMicroseconds(FLYWHEEL_MIN_VALUE);
  delay(1000);
}

void setup() {
  pinMode(PUSHER_EXTENDED_PIN, INPUT);
  pinMode(MAG_INSERTED_PIN, INPUT);
  pinMode(TRIGGER_PIN, INPUT);
  pinMode(SCREEN_SELECT_PIN, INPUT);
  upperFlywheel.attach(UPPER_FLYWHEEL_PIN);
  lowerFlywheel.attach(LOWER_FLYWHEEL_PIN);
  setupScreens();
  setupFlywheels();
}

// Returns the state of the pusher position.
bool isPusherExtended() {
  return digitalRead(PUSHER_EXTENDED_PIN) == HIGH;
}

// Returns the state of the mag.
bool isMagInserted() {
  return digitalRead(MAG_INSERTED_PIN) == HIGH;
}

// Returns if the state of the trigger.
bool isTriggerPressed() {
  return digitalRead(TRIGGER_PIN) == HIGH;
}

// Returns if the state of the screen select button.
bool isScreenSelectorPushed() {
  return digitalRead(SCREEN_SELECT_PIN) == HIGH;
}

// Walk the voltage samples and return an average.
float getBatteryVoltage() {
  float sum;
  // take the VDC analog samples and add them up.
  for (int i = 0; i < VOLTAGE_SAMPLE_SET; i++) {
      sum += voltageSamples[i];
  }
  // Calculate the voltage
  // Use 5.0 for a 5.0V ADC reference voltage
  // 5.015V is the calibrated reference voltage
  return ((sum / (float)VOLTAGE_SAMPLE_SET * 5.015) / 1024.0) * 11.132;
}

// Read the current battery voltage and add it to the sample array.
void readBatteryVoltage() {
  analogRead(VOLTAGE_PIN); // Read the PIN as we could get an old value.
  delay(10); // Make sure the MUTEX has switched to our PIN.
  voltageSamples[voltageSamplesIndex] = analogRead(VOLTAGE_PIN);
  voltageSamplesIndex++;
  if (voltageSamplesIndex >= VOLTAGE_SAMPLE_SET) {
    voltageSamplesIndex = 0;
  }
  voltage = getBatteryVoltage();
}

// Returns if the battery is in a good state.
bool batteryError() {
  if (voltage < VOLTAGE_MIN || voltage > VOLTAGE_MAX) {
    return true;
  }
  return false;
}

// Puts full power into the pusher motor.
void powerPusher() {
  analogWrite(PUSHER_MOTOR_PIN, 255);
}

// Removes all power from the pusher motor once it's retracted.
void stopPusher() {
  analogWrite(PUSHER_MOTOR_PIN, 50);
  while (isPusherExtended()) {
    delay(1);
  }
  analogWrite(PUSHER_MOTOR_PIN, 0);
}

// Activates the pusher motor.
void pushDart(bool fullAuto) {
    powerPusher();
    if (fullAuto) {
      while (isTriggerPressed()) {
        delay(10);
      }
    }
    stopPusher();
}

void startFlywheels() {
  upperFlywheel.writeMicroseconds(flywheelSpeed);
  lowerFlywheel.writeMicroseconds(flywheelSpeed);
  delay(FLYWHEEL_SPINUP_TIME);
}

void stopFlywheels() {
  upperFlywheel.writeMicroseconds(FLYWHEEL_MIN_VALUE);
  lowerFlywheel.writeMicroseconds(FLYWHEEL_MIN_VALUE);
}

void updateFlywheels() {
  long timeSpinning = millis() - flywheelsActivationTime;
  if (timeSpinning > FLYWHEEL_SPIN_TIME) {
    stopFlywheels();
  }
  if (timeSpinning < FLYWHEEL_SPIN_TIME) {
    startFlywheels();
  }
}

// Starts flywheels and/or updates their stop time.
void activateFlywheels() {
  flywheelsActivationTime = millis();
  updateFlywheels();
}

// Pushes the number of darts given.
// 1 == One dart.
// 2 == Two darts.
// 3 == Three darts.
// 4 == Full Auto.
void fire(int dartsToFire) {
  if (dartsToFire == FULL_AUTO) {
    pushDart(true);
    return;
  }
  for (int i = dartsToFire; i > 0; i--) {
    pushDart(false);
  }
}

// Main control loop.
void loop() {
  readBatteryVoltage();
  if (batteryError()) {
//    renderBatteryError(voltage);
//    delay(1000);
//    return;
  }
  if (isTriggerPressed()) {
    activateFlywheels();
    fire(numOfDartsToPush);
  }
  updateFlywheels();
  updateScreens(currentScreenId, dartsRemaining, magSize, fireMode, dartFps, voltage);
  delay(1); // Allow the world rotate.
}

