#include <Servo.h>
#include <Stepper.h>

// Arduino constant pins.
#define FLYWHEEL_SPEED_PIN A0  // Analog 0
#define FLYWHEEL_BIAS_PIN  A1  // Analog 1
#define PUSHER_DPS_PIN     A2  // Analog 2
#define PUSHER_BURST_PIN   A3  // Analog 3
#define STEPPER_PIN_A1     4   // Digital 4
#define STEPPER_PIN_A2     5   // Digital 5
#define STEPPER_PIN_B1     6   // Digital 6
#define STEPPER_PIN_B2     7   // Digital 7
#define TRIGGER_PIN        8   // Digital 8
#define LOWER_FLYWHEEL     9   // Digital 9
#define UPPER_FLYWHEEL     10  // Digital 10

// Internal constants.
#define STEPS_PER_ROTATION     200
#define ONE_SECOND             1000
#define FLYWHEEL_MIN_VALUE     1060
#define FLYWHEEL_MAX_VALUE     1860
#define FLYWHEEL_SPIN_TIME     3000
#define CALIBRATION_DELAY_TIME 3000

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

// Objects.
Servo upperFlywheel;
Servo lowerFlywheel;
Stepper pusher(STEPS_PER_ROTATION, STEPPER_PIN_A1, STEPPER_PIN_A2, STEPPER_PIN_B1, STEPPER_PIN_B2);

void info(const char str[]) {
  if (Serial) {
    Serial.print(str);
  }
}

void info(int i) {
  if (Serial) {
    Serial.print(i);
  }
}

void setup() {
  // Assign pins.
  pinMode(STEPPER_PIN_A1, OUTPUT);
  pinMode(STEPPER_PIN_A2, OUTPUT);
  pinMode(STEPPER_PIN_B1, OUTPUT);
  pinMode(STEPPER_PIN_B2, OUTPUT);
  pinMode(TRIGGER_PIN, INPUT);
  upperFlywheel.attach(UPPER_FLYWHEEL);
  lowerFlywheel.attach(LOWER_FLYWHEEL);
  // Start the system.
  Serial.begin(9600);
  calibrateFlywheels();
  armFlywheels();
  info("AM-1 is hot, have fun.\n");
}

void armFlywheels() {
  info("Arming flywheels.\n");
  upperFlywheel.writeMicroseconds(FLYWHEEL_MIN_VALUE - 100);
  lowerFlywheel.writeMicroseconds(FLYWHEEL_MIN_VALUE - 100);
  delay(CALIBRATION_DELAY_TIME);
  upperFlywheel.writeMicroseconds(FLYWHEEL_MIN_VALUE);
  lowerFlywheel.writeMicroseconds(FLYWHEEL_MIN_VALUE);
  info("Arming flywheels compeleted.\n");
}

// Used by the Afro ESC 12A Speed Controller.
// This does not work if the Arduino is powered via USB.
void calibrateFlywheels() {
  info("Calibration of flywheels started.\n");
  upperFlywheel.writeMicroseconds(FLYWHEEL_MAX_VALUE);
  lowerFlywheel.writeMicroseconds(FLYWHEEL_MAX_VALUE);
  delay(CALIBRATION_DELAY_TIME);
  upperFlywheel.writeMicroseconds(FLYWHEEL_MIN_VALUE);
  lowerFlywheel.writeMicroseconds(FLYWHEEL_MIN_VALUE);
  delay(CALIBRATION_DELAY_TIME);
  info("Calibration of flywheels compeleted.\n");
}

// Read from potentiometer.
void setFlywheelSpeed() {
  flywheelMaxSpeed = map(analogRead(FLYWHEEL_SPEED_PIN), 0, 1023, FLYWHEEL_MIN_VALUE, FLYWHEEL_MAX_VALUE);
}

// Read from potentiometer.
void setFlywheelBias() {
  int bias = map(analogRead(FLYWHEEL_BIAS_PIN), 0, 1023, -50, 50);
  flywheelUpperBias = bias;
  flywheelLowerBias = bias * -1;
}

// Read from potentiometer.
void setDartsPerSecond() {
  pusherDps = map(analogRead(PUSHER_DPS_PIN), 20, 1000, 1, 10);
  pusher.setSpeed(60 * pusherDps); // Rotations per minute.
}

// Read from potentiometer.
void setDartsToPush() {
  // 1 = Single round.
  // 2 = Double tap.
  // 3 = Three round burst.
  // 4 = Full Auto.
  dartsRemainingToPush = map(analogRead(PUSHER_BURST_PIN), 20, 1000, 1, 4);
}

// Read from momentary switch.
bool getTriggerState() {
  return digitalRead(TRIGGER_PIN) == HIGH;
}

void updateFlywheels() {
  // Start the flywheels.
  if (flywheelsTimeRemaining > 0 && flywheelsSpinning == false) {
    // Ramp up speed if more than 50%.
    // ...
    // Set the final speed.
    upperFlywheel.writeMicroseconds(flywheelMaxSpeed + flywheelUpperBias);
    lowerFlywheel.writeMicroseconds(flywheelMaxSpeed + flywheelLowerBias);
    flywheelsSpinning = true;
    info("Flywheels started.\n");
  }
  // Reduce remaining flywheel time.
  if (flywheelsSpinning == true) {
    flywheelsTimeRemaining--;
    delay(1);
  }
  // If we are out of time stop the flywheels.
  if (flywheelsTimeRemaining <= 0 && flywheelsSpinning == true) {
    upperFlywheel.writeMicroseconds(FLYWHEEL_MIN_VALUE);
    lowerFlywheel.writeMicroseconds(FLYWHEEL_MIN_VALUE);
    flywheelsSpinning = false;
    info("Flywheels stopped.\n");
  }
}

void updatePusher() {
  if (dartsRemainingToPush <= 0) {
    return;
  }
  bool burst = dartsRemainingToPush < 4;
  // If set to full auto change only fire one dart at a time.
  if (dartsRemainingToPush == 4) {
    dartsRemainingToPush = 1;
  }
  for(dartsRemainingToPush; dartsRemainingToPush > 0; dartsRemainingToPush--) {
    pushDart();
  }
  // If this was a burst wait for the trigger to be released.
  while (burst && getTriggerState()) {
    info("Waiting for the trigger to be released...\n");
    delay(200);
  }
}

void pushDart() {
  pusher.step(200);
  totalDartsFired++;
  info("Dart fired.\n");
}

void loop() {
  if(getTriggerState()) {
    flywheelsTimeRemaining = FLYWHEEL_SPIN_TIME;
    setFlywheelSpeed();
    setFlywheelBias();
    setDartsPerSecond();
    setDartsToPush();
    infoFiringRequest();
  }
  updateFlywheels();
  updatePusher();
}

void infoFiringRequest() {
  info("--------------\n");
  info("Firing Request\n");
  info("--------------\n");
  info("Flywheel speed: ");
  info(flywheelMaxSpeed - FLYWHEEL_MIN_VALUE);
  info("\n");
  info("Upper flywheel bias: ");
  info(flywheelUpperBias);
  info("\n");
  info("Lower flywheel bias: ");
  info(flywheelLowerBias);
  info("\n");
  info("Darts per second: ");
  info(pusherDps);
  info("\n");
  info("Dart burst count: ");
  info(dartsRemainingToPush);
  info("\n");
  info("Total darts fired: ");
  info(totalDartsFired);
  info("\n");
}

