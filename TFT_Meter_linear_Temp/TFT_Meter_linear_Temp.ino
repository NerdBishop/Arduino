#define M_SIZE 1.3333

#include <TFT_eSPI.h>
#include <SPI.h>
#include "DHT.h"

DHT dht(14, DHT11);

#define TFT_GREY 0x5AEB

TFT_eSPI tft = TFT_eSPI();

float ltx = 0;
uint16_t osx = M_SIZE * 120, osy = M_SIZE * 120;
uint32_t updateTime = 0;

int old_analog = -999;

void setup(void) {

  Serial.begin(115200);

  delay(500);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  analogMeter();

  updateTime = millis();
}

void loop() {

  if (updateTime <= millis()) {

    updateTime = millis() + 500;

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
    }

    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print("%  Temp: ");
    Serial.print(t);
    Serial.println(" C");

    plotNeedle((int)h, 0);

    displayTemp(t);
  }
}

// #########################################################################
// Draw analogue meter
// #########################################################################
void analogMeter()
{
  tft.fillRect(0, 0, M_SIZE * 239, M_SIZE * 126, TFT_GREY);
  tft.fillRect(5, 3, M_SIZE * 230, M_SIZE * 119, TFT_WHITE);

  tft.setTextColor(TFT_BLACK);

  for (int i = -50; i < 51; i += 5) {

    int tl = 15;

    float sx = cos((i - 90) * 0.0174532925);
    float sy = sin((i - 90) * 0.0174532925);

    uint16_t x0 = sx * (M_SIZE * 100 + tl) + M_SIZE * 120;
    uint16_t y0 = sy * (M_SIZE * 100 + tl) + M_SIZE * 140;
    uint16_t x1 = sx * M_SIZE * 100 + M_SIZE * 120;
    uint16_t y1 = sy * M_SIZE * 100 + M_SIZE * 140;

    float sx2 = cos((i + 5 - 90) * 0.0174532925);
    float sy2 = sin((i + 5 - 90) * 0.0174532925);

    int x2 = sx2 * (M_SIZE * 100 + tl) + M_SIZE * 120;
    int y2 = sy2 * (M_SIZE * 100 + tl) + M_SIZE * 140;
    int x3 = sx2 * M_SIZE * 100 + M_SIZE * 120;
    int y3 = sy2 * M_SIZE * 100 + M_SIZE * 140;

    if (i >= 0 && i < 25) {
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_GREEN);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, TFT_GREEN);
    }

    if (i >= 25 && i < 50) {
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_ORANGE);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, TFT_ORANGE);
    }

    if (i % 25 != 0) tl = 8;

    x0 = sx * (M_SIZE * 100 + tl) + M_SIZE * 120;
    y0 = sy * (M_SIZE * 100 + tl) + M_SIZE * 140;
    x1 = sx * M_SIZE * 100 + M_SIZE * 120;
    y1 = sy * M_SIZE * 100 + M_SIZE * 140;

    tft.drawLine(x0, y0, x1, y1, TFT_BLACK);

    if (i % 25 == 0) {

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

    sx = cos((i + 5 - 90) * 0.0174532925);
    sy = sin((i + 5 - 90) * 0.0174532925);

    x0 = sx * M_SIZE * 100 + M_SIZE * 120;
    y0 = sy * M_SIZE * 100 + M_SIZE * 140;

    if (i < 50) tft.drawLine(x0, y0, x1, y1, TFT_BLACK);
  }

  tft.drawString("%RH", M_SIZE * (5 + 230 - 40), M_SIZE * (119 - 20), 2);
  tft.drawCentreString("%RH", M_SIZE * 120, M_SIZE * 70, 4);
  tft.drawRect(5, 3, M_SIZE * 230, M_SIZE * 119, TFT_BLACK);

  plotNeedle(0, 0);
}

// #########################################################################
// Update needle
// #########################################################################
void plotNeedle(int value, byte ms_delay)
{
  tft.setTextColor(TFT_BLACK, TFT_WHITE);

  char buf[8];
  dtostrf(value, 4, 0, buf);

  tft.drawRightString(buf, M_SIZE * 40, M_SIZE * (119 - 20), 2);

  if (value < -10) value = -10;
  if (value > 110) value = 110;

  while (!(value == old_analog)) {

    if (old_analog < value) old_analog++;
    else old_analog--;

    if (ms_delay == 0) old_analog = value;

    float sdeg = map(old_analog, -10, 110, -150, -30);

    float sx = cos(sdeg * 0.0174532925);
    float sy = sin(sdeg * 0.0174532925);

    float tx = tan((sdeg + 90) * 0.0174532925);

    tft.drawLine(M_SIZE * (120 + 20 * ltx - 1),
                 M_SIZE * (140 - 20),
                 osx - 1,
                 osy,
                 TFT_WHITE);

    tft.drawLine(M_SIZE * (120 + 20 * ltx),
                 M_SIZE * (140 - 20),
                 osx,
                 osy,
                 TFT_WHITE);

    tft.drawLine(M_SIZE * (120 + 20 * ltx + 1),
                 M_SIZE * (140 - 20),
                 osx + 1,
                 osy,
                 TFT_WHITE);

    tft.setTextColor(TFT_BLACK);
    tft.drawCentreString("%RH", M_SIZE * 120, M_SIZE * 70, 4);

    ltx = tx;
    osx = M_SIZE * (sx * 98 + 120);
    osy = M_SIZE * (sy * 98 + 140);

    tft.drawLine(M_SIZE * (120 + 20 * ltx - 1),
                 M_SIZE * (140 - 20),
                 osx - 1,
                 osy,
                 TFT_RED);

    tft.drawLine(M_SIZE * (120 + 20 * ltx),
                 M_SIZE * (140 - 20),
                 osx,
                 osy,
                 TFT_MAGENTA);

    tft.drawLine(M_SIZE * (120 + 20 * ltx + 1),
                 M_SIZE * (140 - 20),
                 osx + 1,
                 osy,
                 TFT_RED);

    if (abs(old_analog - value) < 10)
      ms_delay += ms_delay / 5;

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