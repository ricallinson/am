#include <Servo.h>
#include <Stepper.h>
 
Servo upperFlywheel;
Servo lowerFlywheel;
Stepper pusher(200, 4, 5, 6, 7);

// Arduino constant pins.
const int FLYWHEEL_SPEED_PIN = 0; // Analog 0
const int FLYWHEEL_BIAS_PIN = 1;  // Analog 1
const int PUSHER_DPS_PIN = 2;     // Analog 2
const int PUSHER_BURST_PIN = 3;   // Analog 3
const int TRIGGER_PIN = 8;        // Digital 8
const int LOWER_FLYWHEEL = 9;     // Digital 9
const int UPPER_FLYWHEEL = 10;    // Digital 10

// Internal constants.
const int ONE_SECOND = 1000;
const int FLYWHEEL_MIN_VALUE = 1060;
const int FLYWHEEL_MAX_VALUE = 1860;
//const int FLYWHEEL_MID_VALUE = FLYWHEEL_MIN_VALUE + ((FLYWHEEL_MAX_VALUE - FLYWHEEL_MIN_VALUE) / 2); // ~1440;
//const int FLYWHEEL_STOP_VALUE = FLYWHEEL_MID_VALUE - 50;
const int FLYWHEEL_SPIN_TIME = 3000;
const int CALIBRATION_DELAY_TIME = 3000;

// External input values.
int flywheelMaxSpeed;
int flywheelUpperBias;
int flywheelLowerBias;
int pusherDps; // Darts per second

// Internal counting values.
int flywheelsTimeRemaining;
int dartsRemainingToPush;
int totalDartsFired;

// Internal state.
bool flywheelsSpinning;

void setup() {
  // Assign pins.
  pinMode(TRIGGER_PIN, INPUT);
  upperFlywheel.attach(UPPER_FLYWHEEL);
  lowerFlywheel.attach(LOWER_FLYWHEEL);
  // Start the system.
  Serial.begin(9600);
  calibrateFlywheels();
}

// Used by the Afro ESC 12A Speed Controller.
void calibrateFlywheels() {
  Serial.print("Calibration started\n");
  upperFlywheel.writeMicroseconds(FLYWHEEL_MAX_VALUE);
  lowerFlywheel.writeMicroseconds(FLYWHEEL_MAX_VALUE);
  delay(CALIBRATION_DELAY_TIME);
  upperFlywheel.writeMicroseconds(FLYWHEEL_MIN_VALUE);
  lowerFlywheel.writeMicroseconds(FLYWHEEL_MIN_VALUE);
  delay(CALIBRATION_DELAY_TIME);
//  // This shouldn't be required as RC_CAR mode should be off?
//  upperFlywheel.writeMicroseconds(FLYWHEEL_MID_VALUE);
//  lowerFlywheel.writeMicroseconds(FLYWHEEL_MID_VALUE);
//  delay(CALIBRATION_DELAY_TIME);
//  upperFlywheel.writeMicroseconds(FLYWHEEL_STOP_VALUE);
//  lowerFlywheel.writeMicroseconds(FLYWHEEL_STOP_VALUE);
  Serial.print("Calibration compeleted\n");
}

// Read from potentiometer.
void setFlywheelSpeed() {
  flywheelMaxSpeed = map(analogRead(FLYWHEEL_SPEED_PIN), 0, 1023, FLYWHEEL_MIN_VALUE + 50, FLYWHEEL_MAX_VALUE);
}

// Read from potentiometer.
void setFlywheelBias() {
  int bias = map(analogRead(FLYWHEEL_BIAS_PIN), 0, 1023, -100, 100);
  flywheelUpperBias = bias;
  flywheelLowerBias = bias * -1;
}

// Read from potentiometer.
void setDartsPerSecond() {
  pusherDps = map(analogRead(PUSHER_DPS_PIN), 0, 1023, 1, 8);
  pusher.setSpeed(60 * pusherDps); // Rotations per minute.
}

// Read from potentiometer.
void setDartsToPush() {
  // 1 = Single round.
  // 2 = Double tap.
  // 3 = Three round burst.
  // 4 = Full Auto.
  dartsRemainingToPush = map(analogRead(PUSHER_BURST_PIN), 0, 1023, 1, 4);
}

// Read from momentary switch.
bool getTriggerState() {
  return digitalRead(TRIGGER_PIN) == HIGH;
}

void updateFlywheels() {
  // Start the flywheels.
  if (flywheelsTimeRemaining > 0 && flywheelsSpinning == false) {
    setFlywheelSpeed();
    setFlywheelBias();
    // Set the final speed.
    upperFlywheel.writeMicroseconds(flywheelMaxSpeed + flywheelUpperBias);
    lowerFlywheel.writeMicroseconds(flywheelMaxSpeed + flywheelLowerBias);
    flywheelsSpinning = true;
    Serial.print("Flywheels started.\n");
    return;
  }
  // If we are out of time stop the flywheels.
  if (flywheelsTimeRemaining <= 0 && flywheelsSpinning == true) {
    upperFlywheel.writeMicroseconds(FLYWHEEL_MIN_VALUE);
    lowerFlywheel.writeMicroseconds(FLYWHEEL_MIN_VALUE);
    flywheelsSpinning = false;
    Serial.print("Flywheels stopped.\n");
    return;
  }
  // Reduce flywheel time.
  if (flywheelsSpinning == true) {
    flywheelsTimeRemaining--;
    delay(1);
  }
}

void updatePusher() {
  bool burst = dartsRemainingToPush < 4;
  for(int i = dartsRemainingToPush; i > 0; i--) {
    pushDart();
  }
  dartsRemainingToPush = 0;
  // If this was a burst wait for the trigger to be released.
  while (burst && getTriggerState()) {
    delay(100);
  }
}

void pushDart() {
  pusher.step(200); // 200 steps per rotation
  delay(100); // Fake dart firing.
  totalDartsFired++;
  Serial.print("Dart fired\n");
}

void loop() {
  if(getTriggerState()) {
    flywheelsTimeRemaining = FLYWHEEL_SPIN_TIME;
    setDartsPerSecond();
    setDartsToPush();
    log();
  }
  updateFlywheels();
  updatePusher();
}

void log() {
  Serial.print("Darts per second: ");
  Serial.print(pusherDps);
  Serial.print("\n");
  Serial.print("Flywheel speed: ");
  Serial.print(flywheelMaxSpeed);
  Serial.print("\n");
  Serial.print("Upper flywheel bias: ");
  Serial.print(flywheelUpperBias);
  Serial.print("\n");
  Serial.print("Lower flywheel bias: ");
  Serial.print(flywheelLowerBias);
  Serial.print("\n");
  Serial.print("Dart burst count: ");
  Serial.print(dartsRemainingToPush);
  Serial.print("\n");
  Serial.print("Total darts fired: ");
  Serial.print(totalDartsFired);
  Serial.print("\n");
}

