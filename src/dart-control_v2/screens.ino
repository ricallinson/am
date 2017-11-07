//
// Copyright 2016, Yahoo Inc.
// Copyrights licensed under the New BSD License.
// See the accompanying LICENSE file for terms.
//

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 Screens(4);

void setupScreens() {
  Screens.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  Screens.clearDisplay();
  Screens.setTextColor(WHITE);
  Screens.setCursor(0, 0);
  Screens.println("INITIALISING...");
  Screens.display();
}

void updateScreens(int screenId, int dartsRemaining, int magSize, int fireMode, int dartFps, float voltage) {
  Screens.clearDisplay();
  switch (screenId) {
    case 0:
      // Show remaining darts.
      renderDartsRemaining(dartsRemaining);
      break;
    case 1:
      // Change Mag Size.
      renderMagSize(magSize);
      break;
    case 2:
      // Change Mode.
      renderMode(fireMode);
      break;
    case 3:
      // Change FPS.
      renderFps(dartFps);
      break;
  }
  renderBatteryHeader(voltage);
  Screens.display();
}

void renderDartsRemaining(int dartsRemaining) {
  Screens.setTextColor(WHITE);
  Screens.setCursor(0, 0);
  Screens.setTextSize(1);
  Screens.println("REMAINING");
  Screens.setTextSize(3);
  Screens.setCursor(0, 10);
  Screens.println(dartsRemaining);
}

void renderMagSize(int magSize) {
  Screens.setTextColor(WHITE);
  Screens.setCursor(0, 0);
  Screens.setTextSize(1);
  Screens.println("MAG SIZE");
  Screens.setTextSize(3);
  Screens.setCursor(0, 10);
  Screens.println(magSize);
}

void renderFps(int fps) {
  Screens.setTextColor(WHITE);
  Screens.setCursor(0, 0);
  Screens.setTextSize(1);
  Screens.println("FPS");
  Screens.setTextSize(3);
  Screens.setCursor(0, 10);
  Screens.println(fps);
}

void renderMode(int fireMode) {
  Screens.setTextColor(WHITE);
  Screens.setCursor(0, 0);
  Screens.setTextSize(1);
  Screens.println("FIRE MODE");
  Screens.setTextSize(3);
  Screens.setCursor(0, 10);
  switch (fireMode) {
    case 1:
      Screens.println("Single");
      break;
    case 2:
      Screens.println("Double");
      break;
    case 3:
      Screens.println("Tripple");
      break;
    case 4:
      Screens.println("Full Auto");
      break;
  } 
}

void renderBatteryHeader(float voltage) {
  Screens.setTextColor(WHITE);
  Screens.setCursor(90, 0);
  Screens.setTextSize(1);
  Screens.print(voltage);
  Screens.setCursor(120, 0);
  Screens.print("V");
}

void renderBatteryError(float voltage) {
  Screens.clearDisplay();
  Screens.setTextColor(WHITE);
  Screens.setCursor(0, 0);
  Screens.setTextSize(1);
  if (voltage <= VOLTAGE_MIN) {
    Screens.println("BATTERY LOW");
  } else if (voltage >= VOLTAGE_MAX) {
    Screens.println("BATTERY ERROR");
  }
  Screens.setTextSize(3);
  Screens.setCursor(0, 10);
  Screens.print(voltage);
  Screens.display();
}

