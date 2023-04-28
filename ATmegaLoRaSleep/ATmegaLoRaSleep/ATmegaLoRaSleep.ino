#include "LowPower.h"
#include <SPI.h>
#include <LoRa.h>

void setup() {
  
   if (!LoRa.begin(868E6)) {
     Serial.println("LoRa init failed. Check your connections.");
     while (true);
   }
   
   delay(1000);

   //Lora Sleep
   LoRa.sleep();

   delay(1000);
   //ATmega Sleep
   LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
}

void loop() {
   
}
