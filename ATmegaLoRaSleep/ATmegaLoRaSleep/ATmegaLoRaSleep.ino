#include "LowPower.h"
#include <SPI.h>
#include <LoRa.h>

const int mosfet1 = 6;
const int mosfet2 = 7;
const int onboard_LED = 8;

void setup() {
    pinMode(mosfet1, OUTPUT);
    pinMode(mosfet2, OUTPUT);
    pinMode(onboard_LED, OUTPUT);

    delay(500);

    digitalWrite(mosfet1, LOW);
    digitalWrite(mosfet2, LOW);
    digitalWrite(onboard_LED, LOW);

    delay(500);
    
   if (!LoRa.begin(868E6)) {
     Serial.println("LoRa init failed. Check your connections.");
     while (true);
   }
   
   delay(500);

   //Lora Sleep
   LoRa.sleep();

   delay(500);
   //ATmega Sleep
   LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
}

void loop() {
   
}
