/*
 An example analogue meter using a ILI9341 TFT LCD screen

 Needs Font 2 (also Font 4 if using large scale label)

 Make sure all the display driver and pin connections are correct by
 editing the User_Setup.h file in the TFT_eSPI library folder.

 #########################################################################
 ###### DON'T FORGET TO UPDATE THE User_Setup.h FILE IN THE LIBRARY ######
 #########################################################################
 
Updated by Bodmer for variable meter size
 */

// Define meter size as 1 for tft.rotation(0) or 1.3333 for tft.rotation(1)
#define M_SIZE 1.3333

#include <TFT_eSPI.h>  // Hardware-specific library
#include <SPI.h>
#include "DHT.h"

DHT dht(14, DHT11);

TFT_eSPI tft = TFT_eSPI();  // Invoke custom library

#define TFT_GREY 0x5AEB

float ltx = 0;                                    // Saved x coord of bottom of needle
uint16_t osx = M_SIZE * 120, osy = M_SIZE * 120;  // Saved x & y coords
uint32_t updateTime = 0;                          // time for next update

int old_analog = -999;  // Value last displayed

int old_value[6] = { -1, -1, -1, -1, -1, -1 };
int d = 0;

void setup(void) {
  tft.init();
  tft.setRotation(1);
  Serial.begin(115200);  // For debug
  tft.fillScreen(TFT_BLACK);
  dht.begin();

  analogMeter();  // Draw analogue meter

  updateTime = millis();  // Next update time
}


void loop() {
  if (updateTime <= millis()) {
    updateTime = millis() + 100;  // Update meter every 35 milliseconds

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
    }
    // Create a Sine wave for testing
    //Serial.print("Hum: ");
    //Serial.print(h, 2);
    //Serial.print("%  Temp: ");
    //Serial.print(t, 2);
    //Serial.println("°C ");

    plotNeedle(h, 0);  // It takes between 2 and 12ms to replot the needle with zero delay
    displayTemp(t);
  }
}

// #########################################################################
//  Draw the analogue meter on the screen
// #########################################################################
void analogMeter() {

  // Meter outline
  tft.fillRect(0, 0, M_SIZE * 239, M_SIZE * 126, TFT_GREY);
  tft.fillRect(5, 3, M_SIZE * 230, M_SIZE * 119, TFT_WHITE);

  tft.setTextColor(TFT_BLACK);  // Text colour

  // Draw ticks every 5 degrees from -50 to +50 degrees (100 deg. FSD swing)
  for (int i = -50; i < 51; i += 5) {
    // Long scale tick length
    int tl = 15;

    // Coordinates of tick to draw
    float sx = cos((i - 90) * 0.0174532925);
    float sy = sin((i - 90) * 0.0174532925);
    uint16_t x0 = sx * (M_SIZE * 100 + tl) + M_SIZE * 120;
    uint16_t y0 = sy * (M_SIZE * 100 + tl) + M_SIZE * 140;
    uint16_t x1 = sx * M_SIZE * 100 + M_SIZE * 120;
    uint16_t y1 = sy * M_SIZE * 100 + M_SIZE * 140;

    // Coordinates of next tick for zone fill
    float sx2 = cos((i + 5 - 90) * 0.0174532925);
    float sy2 = sin((i + 5 - 90) * 0.0174532925);
    int x2 = sx2 * (M_SIZE * 100 + tl) + M_SIZE * 120;
    int y2 = sy2 * (M_SIZE * 100 + tl) + M_SIZE * 140;
    int x3 = sx2 * M_SIZE * 100 + M_SIZE * 120;
    int y3 = sy2 * M_SIZE * 100 + M_SIZE * 140;

    //Red zone limits
    if (i >= -25 && i < -15) {
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_RED);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, TFT_RED);
    }

    //Yellow zone limits
    if (i >= -15 && i < 0) {
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_YELLOW);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, TFT_YELLOW);
    }

    // Green zone limits
    if (i >= 0 && i < 25) {
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_GREEN);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, TFT_GREEN);
    }

    // Orange zone limits
    if (i >= 25 && i < 50) {
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_ORANGE);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, TFT_ORANGE);
    }

    // Short scale tick length
    if (i % 25 != 0) tl = 8;

    // Recalculate coords incase tick lenght changed
    x0 = sx * (M_SIZE * 100 + tl) + M_SIZE * 120;
    y0 = sy * (M_SIZE * 100 + tl) + M_SIZE * 140;
    x1 = sx * M_SIZE * 100 + M_SIZE * 120;
    y1 = sy * M_SIZE * 100 + M_SIZE * 140;

    // Draw tick
    tft.drawLine(x0, y0, x1, y1, TFT_BLACK);

    // Check if labels should be drawn, with position tweaks
    if (i % 25 == 0) {
      // Calculate label positions
      x0 = sx * (M_SIZE * 100 + tl + 10) + M_SIZE * 120;
      y0 = sy * (M_SIZE * 100 + tl + 10) + M_SIZE * 140;
      switch (i / 25) {
        case -2: tft.drawCentreString("0", x0, y0 - 12, 2); break;
        case -1: tft.drawCentreString("25", x0, y0 - 9, 2); break;
        case 0: tft.drawCentreString("50", x0, y0 - 7, 2); break;
        case 1: tft.drawCentreString("75", x0, y0 - 9, 2); break;
        case 2: tft.drawCentreString("100", x0, y0 - 12, 2); break;
      }
    }

    // Now draw the arc of the scale
    sx = cos((i + 5 - 90) * 0.0174532925);
    sy = sin((i + 5 - 90) * 0.0174532925);
    x0 = sx * M_SIZE * 100 + M_SIZE * 120;
    y0 = sy * M_SIZE * 100 + M_SIZE * 140;
    // Draw scale arc, don't draw the last part
    if (i < 50) tft.drawLine(x0, y0, x1, y1, TFT_BLACK);
  }

  //tft.drawString("%RH", M_SIZE*(5 + 230 - 40), M_SIZE*(119 - 20), 2); // Units at bottom right
  //tft.drawCentreString("%RH", M_SIZE*120, M_SIZE*70, 4); // Comment out to avoid font 4
  //tft.drawRect(5, 3, M_SIZE*230, M_SIZE*119, TFT_BLACK); // Draw bezel line

  plotNeedle(0, 0);  // Put meter needle at 0
}

// #########################################################################
// Update needle position
// This function is blocking while needle moves, time depends on ms_delay
// 10ms minimises needle flicker if text is drawn within needle sweep area
// Smaller values OK if text not in sweep area, zero for instant movement but
// does not look realistic... (note: 100 increments for full scale deflection)
// #########################################################################
void plotNeedle(float value, byte ms_delay) {
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  char buf[8];
  dtostrf(value, 4, 0, buf);
  tft.drawRightString(buf, M_SIZE * 60, M_SIZE * (119 - 20), 4);

  if (value < -10) value = -10;  // Limit value to emulate needle end stops
  if (value > 110) value = 110;



  // Move the needle until new value reached
  while (!(value == old_analog)) {
    if (old_analog < value) old_analog++;
    else old_analog--;

    if (ms_delay == 0) old_analog = value;  // Update immediately if delay is 0

    float sdeg = map(old_analog, -10, 110, -150, -30);  // Map value to angle
    // Calculate tip of needle coords
    float sx = cos(sdeg * 0.0174532925);
    float sy = sin(sdeg * 0.0174532925);

    // Calculate x delta of needle start (does not start at pivot point)
    float tx = tan((sdeg + 90) * 0.0174532925);

    // Erase old needle image
    tft.drawLine(M_SIZE * (120 + 20 * ltx - 1), M_SIZE * (140 - 20), osx - 1, osy, TFT_WHITE);
    tft.drawLine(M_SIZE * (120 + 20 * ltx), M_SIZE * (140 - 20), osx, osy, TFT_WHITE);
    tft.drawLine(M_SIZE * (120 + 20 * ltx + 1), M_SIZE * (140 - 20), osx + 1, osy, TFT_WHITE);

    // Re-plot text under needle
    tft.setTextColor(TFT_BLACK);
    tft.drawCentreString("%RH", M_SIZE * 120, M_SIZE * 70, 4);  // // Comment out to avoid font 4

    // Store new needle end coords for next erase
    ltx = tx;
    osx = M_SIZE * (sx * 98 + 120);
    osy = M_SIZE * (sy * 98 + 140);

    // Draw the needle in the new postion, magenta makes needle a bit bolder
    // draws 3 lines to thicken needle
    tft.drawLine(M_SIZE * (120 + 20 * ltx - 1), M_SIZE * (140 - 20), osx - 1, osy, TFT_RED);
    tft.drawLine(M_SIZE * (120 + 20 * ltx), M_SIZE * (140 - 20), osx, osy, TFT_MAGENTA);
    tft.drawLine(M_SIZE * (120 + 20 * ltx + 1), M_SIZE * (140 - 20), osx + 1, osy, TFT_RED);

    // Slow needle down slightly as it approaches new postion
    if (abs(old_analog - value) < 10) ms_delay += ms_delay / 5;

    // Wait before next update
    delay(ms_delay);
  }
}

void displayTemp(float temp) {
  if (temp <= 18) {
    tft.setTextColor(TFT_BLUE, TFT_BLACK);
  } else if (temp > 18 && temp <= 24.5) {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
  } else {
    tft.setTextColor(TFT_RED, TFT_BLACK);
  }

  char buff[8];
  dtostrf(temp, 4, 1, buff);
  tft.drawRightString(buff, M_SIZE * 88, M_SIZE * (135), 6);
  tft.drawRightString("0", M_SIZE * 98, M_SIZE * (135), 2);
  tft.drawRightString("C", M_SIZE * 110, M_SIZE * (145), 4);
  //tft.drawRightString(buff, horizontal, vertical, font);
}
