#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>

Adafruit_SSD1306 display(4);

// Arduino constant pins.
#define DATA_PIN           A2  // Analog 2
#define VDC_PIN            A3  // Analog 3
#define STEPPER_PIN_A1     3   // Digital 3
#define STEPPER_PIN_A2     4   // Digital 4
#define STEPPER_PIN_B1     5   // Digital 5
#define STEPPER_PIN_B2     6   // Digital 6
#define LOWER_FLYWHEEL     7   // Digital 7
#define UPPER_FLYWHEEL     8   // Digital 8
#define RELOAD_PIN         9   // Digital 9
#define TRIGGER_PIN        10  // Digital 10
#define DISPLAY_PIN        11  // Digital 11
#define PUSHER_HOME_MARKER 12  // Digital 12

// Internal constants.
#define STEPS_PER_PUSH         15
#define STEP_DELAY             2
#define FLYWHEEL_MIN_VALUE     1060
#define FLYWHEEL_MAX_VALUE     1860
#define FLYWHEEL_SPINUP_TIME   200
#define FLYWHEEL_SPIN_TIME     3000
#define CALIBRATION_DELAY_TIME 3000
#define VDC_MIN 10.0
#define VDC_MAX 12.6
#define VDC_SAMPLE_SIZE 10

// External input values.
int flywheelFps = FLYWHEEL_MIN_VALUE + 200;
int flywheelUpperBias = 0;
int flywheelLowerBias = 0;
int pusherDps = 1; // Darts per second
int magSize = 6;

// Internal counting values.
int dartsToPush = 4;
int totalDartsFired;
int remainingDarts;

// Internal state.
int  displayId;
bool resetInputValue;
bool newMag;
float vdcReadings[VDC_SAMPLE_SIZE];
int vdcReadingsIndex;
bool flywheelsSpinning;
long flywheelsStartTime;

// Objects.
Servo upperFlywheel;
Servo lowerFlywheel;

//
// Logging functions.
//

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

//
// Stepper functions.
//

void step1() {
  digitalWrite(STEPPER_PIN_A1, HIGH);
  digitalWrite(STEPPER_PIN_A2, LOW);
  digitalWrite(STEPPER_PIN_B1, HIGH);
  digitalWrite(STEPPER_PIN_B2, LOW);
  delay(STEP_DELAY);
}
void step2() {
  digitalWrite(STEPPER_PIN_A1, LOW);
  digitalWrite(STEPPER_PIN_A2, HIGH);
  digitalWrite(STEPPER_PIN_B1, HIGH);
  digitalWrite(STEPPER_PIN_B2, LOW);
  delay(STEP_DELAY);
}
void step3() {
  digitalWrite(STEPPER_PIN_A1, LOW);
  digitalWrite(STEPPER_PIN_A2, HIGH);
  digitalWrite(STEPPER_PIN_B1, LOW);
  digitalWrite(STEPPER_PIN_B2, HIGH);
  delay(STEP_DELAY);
}
void step4() {
  digitalWrite(STEPPER_PIN_A1, HIGH);
  digitalWrite(STEPPER_PIN_A2, LOW);
  digitalWrite(STEPPER_PIN_B1, LOW);
  digitalWrite(STEPPER_PIN_B2, HIGH);
  delay(STEP_DELAY);
}
void stopMotor() {
  digitalWrite(STEPPER_PIN_A1, LOW);
  digitalWrite(STEPPER_PIN_A2, LOW);
  digitalWrite(STEPPER_PIN_B1, LOW);
  digitalWrite(STEPPER_PIN_B2, LOW);
}

//
// Operational functions.
//

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
  startDisplay();
  homePusher();
  calibrateFlywheels();
  info("AM-1 is hot, have fun.\n");
}

void startDisplay() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("BOOTING...");
  display.display();
  info("Display started.\n");
}

void updateDisplay(int id) {
  display.clearDisplay();
  switch (id) {
    case 0:
      // Show remaining darts.
      renderRemainingDarts();
      break;
    case 1:
      // Change Mag Size.
      renderMagSize();
      break;
    case 2:
      // Change Mode.
      renderMode();
      break;
    case 3:
      // Change DPS.
      renderDps();
      break;
    case 4:
      // Change FPS.
      renderFps();
      break;
    case 5:
      // Change Bias.
      renderBias();
      break;
    case 6:
      // Show Info.
      renderInfo();
      break;
  }
  renderBattery();
  display.display();
}

void renderRemainingDarts() {
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println("REMAINING");
  display.setTextSize(3);
  display.setCursor(0, 10);
  display.println(remainingDarts);
}

void renderMagSize() {
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println("MAG SIZE");
  display.setTextSize(3);
  display.setCursor(0, 10);
  display.println(magSize);
}

void renderFps() {
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println("FPS");
  display.setTextSize(3);
  display.setCursor(0, 10);
  display.println(flywheelFps - FLYWHEEL_MIN_VALUE);
}

void renderDps() {
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println("DPS");
  display.setTextSize(3);
  display.setCursor(0, 10);
  display.println(pusherDps);
}

void renderBias() {
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println("BIAS");
  display.setTextSize(3);
  display.setCursor(0,10);
  display.println(flywheelUpperBias);
  display.setCursor(65, 10);
  display.println(flywheelLowerBias);
}

void renderMode() {
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println("BURST MODE");
  display.setTextSize(3);
  display.setCursor(0, 10);
  switch (dartsToPush) {
    case 1:
      display.println("Single");
      break;
    case 2:
      display.println("Double");
      break;
    case 3:
      display.println("Tripple");
      break;
    case 4:
      display.println("Auto");
      break;
  }
}

void renderBattery() {
  display.setTextColor(WHITE);
  display.setCursor(100, 0);
  display.setTextSize(1);
  display.print(map(getBatteryVoltage(), VDC_MIN, VDC_MAX, 0, 100));
  display.setCursor(120, 0);
  display.print("%");
}

void renderBatteryError() {
  float vdc = getBatteryVoltage();
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.setTextSize(1);
  if (vdc <= VDC_MIN) {
    display.println("BAT LOW");
  } else if (vdc >= VDC_MAX) {
    display.println("BAT ERR");
  }
  display.display();
}

void renderInfo() {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  // Top.
  display.setCursor(0, 0);
  display.print("INFO");
  // Left side.
  display.setCursor(0, 8);
  display.print("FPS:   ");
  display.println(flywheelFps - FLYWHEEL_MIN_VALUE);
  display.print("BURST: ");
  display.println(dartsToPush);
  display.print("FIRED: ");
  display.println(totalDartsFired);
  // Right Side.
  display.setCursor(80, 8);
  display.print("DPS: ");
  display.println(pusherDps);
  display.setCursor(80,16);
  display.print("UB:  ");
  display.println(flywheelUpperBias);
  display.setCursor(80, 24);
  display.print("LB:  ");
  display.println(flywheelLowerBias);
}

void homePusher() {
  info("Calibration of pusher.\n");
  pusherExtend(5);
  byte homed = digitalRead(PUSHER_HOME_MARKER);
  while (homed == LOW) {
    pusherRetract(1);
    homed = digitalRead(PUSHER_HOME_MARKER);
  }
  info("Calibration of pusher compeleted.\n");
  display.println("Pusher.");
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
  display.println("Flywheels.");
}

void readDisplayIdInputValue() {
  if (digitalRead(DISPLAY_PIN) == HIGH) {
    displayId++;
    if (displayId > 6) {
      displayId = 0;
    }
    resetInputValue = true;
    while (digitalRead(DISPLAY_PIN) == HIGH) {
      delay(200);
    }
  }
}

// Read from potentiometer.
void readPotInputValue(int id) {
  int value = analogRead(DATA_PIN);
  if (resetInputValue && value > 0) {
    return;
  }
  resetInputValue = false;
  switch (id) {
    case 1:
      // Change Mag Size.
      magSize = map(value, 0, 1023, 1, 35);
      remainingDarts = magSize;
      break;
    case 2:
      // Change Mode.
      // 1 = Single round.
      // 2 = Double tap.
      // 3 = Three round burst.
      // 4 = Full Auto.
      dartsToPush = map(value, 10, 900, 1, 4);
      break;
    case 3:
      // Change DPS.
      pusherDps = map(value, 0, 1023, 1, 4);
      break;
    case 4:
      // Change FPS.
      flywheelFps = map(value, 0, 1023, FLYWHEEL_MIN_VALUE, FLYWHEEL_MAX_VALUE);
      break;
    case 5:
      // Change Bias.
      int bias = map(value, 0, 1023, -50, 50);
      flywheelUpperBias = bias;
      flywheelLowerBias = bias * -1;
      break;
  }
}

float getBatteryVoltage() {
  int sum;
  // take the VDC analog samples and add them up.
  for (int i = 0; i < VDC_SAMPLE_SIZE; i++) {
      sum += vdcReadings[i];
  }
  // Calculate the voltage
  // Use 5.0 for a 5.0V ADC reference voltage
  // 5.015V is the calibrated reference voltage
  return ((float)sum / (float)VDC_SAMPLE_SIZE * 5.015) / 1024.0;
}

void readBatteryVoltage() {
  analogRead(VDC_PIN); // Read the PIN as we could get an old value.
  delay(10); // Make sure the MUTEX has switched to our PIN.
  vdcReadings[vdcReadingsIndex] = analogRead(VDC_PIN);
  vdcReadingsIndex++;
  if (vdcReadingsIndex >= VDC_SAMPLE_SIZE) {
    vdcReadingsIndex = 0;
  }
}

void readLoadingState() {
  if (newMag == true && digitalRead(RELOAD_PIN) == HIGH) {
    remainingDarts = 0;
    newMag = false;
    info("Mag removed.\n");
  }
  if (newMag == false && digitalRead(RELOAD_PIN) == LOW) {
    remainingDarts = magSize;
    newMag = true;
    info("Mag loaded.\n");
  }
}

// Read from momentary switch.
bool readTriggerState() {
  return digitalRead(TRIGGER_PIN) == HIGH;
}

void startFlywheels() {
  // Ramp up speed if more than 50%.
  // ...
  // Set the final speed.
  upperFlywheel.writeMicroseconds(flywheelFps + flywheelUpperBias);
  lowerFlywheel.writeMicroseconds(flywheelFps + flywheelLowerBias);
  delay(FLYWHEEL_SPINUP_TIME);
  flywheelsSpinning = true;
  info("Flywheels started.\n");
}

void stopFlywheels() {
  upperFlywheel.writeMicroseconds(FLYWHEEL_MIN_VALUE);
  lowerFlywheel.writeMicroseconds(FLYWHEEL_MIN_VALUE);
  flywheelsSpinning = false;
  info("Flywheels stopped.\n");
}

void updateFlywheels() {
  long timeSpinning = millis() - flywheelsStartTime;
  if (flywheelsSpinning == true && timeSpinning > FLYWHEEL_SPIN_TIME) {
    stopFlywheels();
    flywheelsSpinning = false;
  }
  if (flywheelsSpinning == false && timeSpinning < FLYWHEEL_SPIN_TIME) {
    startFlywheels();
    flywheelsSpinning = true;
  }
}

void pusherExtend(int steps) {
  while (steps > 0) {
    step4();
    step3();
    step2();
    step1();
    steps--;
  }
  stopMotor();
}

void pusherRetract(int steps) {
  while (steps > 0) {
    step1();
    step2();
    step3();
    step4();
    steps--;
  }
  stopMotor();
}

void updatePusher() {
  if (dartsToPush == 4) {
    autoPusher();
  } else {
    burstPusher(dartsToPush);
  }
}

void burstPusher(int i) {
  for(i; i > 0; i--) {
    pushDart();
  }
  while (readTriggerState()) {
    info("Waiting for the trigger to be released...\n");
    delay(200);
  }
}

void autoPusher() {
  while (readTriggerState()) {
    pushDart();
  }
}

void pushDart() {
  pusherExtend(STEPS_PER_PUSH);
  pusherRetract(STEPS_PER_PUSH);
  remainingDarts--;
  totalDartsFired++;
  info("Dart fired.\n");
}

bool isCharged() {
  float vdc = getBatteryVoltage();
  if (vdc > VDC_MIN || vdc < VDC_MAX) {
    return true;
  }
  // Show change battery.
  renderBatteryError();
  info("Battery ~");
  info(vdc);
  info(" VDC\n");
  return false;
}

bool isFiring() {
  if(readTriggerState() == false) {
    return false;
  }
  displayId = 0;
  flywheelsStartTime = millis();
  updateFlywheels();
  updatePusher();
  infoFiringRequest();
  return true;
}

void loop() {
  if (isCharged() == false) {
    // If the battery low or high don't do anything.
    //return;
  }
  if (isFiring() == false) {
    readLoadingState();
    readDisplayIdInputValue();
    readBatteryVoltage();
  }
  if (displayId > 0) {
    readPotInputValue(displayId);
  }
  updateDisplay(displayId);
  updateFlywheels();
}

void infoFiringRequest() {
  info("--------------\n");
  info("Firing Request\n");
  info("--------------\n");
  info("Flywheel feet per second: ");
  info(flywheelFps - FLYWHEEL_MIN_VALUE);
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
  info(dartsToPush);
  info("\n");
  info("Total darts fired: ");
  info(totalDartsFired);
  info("\n");
}

