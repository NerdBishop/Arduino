////////////////////////////////////////////////
//This code was inspired by:
//https://github.com/Yveaux/Arduino_Vcc
//I made a few modifications to read out battery voltage.
//The battery should be connected to Analog pin A0 through a voltage divider(both R1 & R2 are 10k ohms each)
//NOTE: 1. Always make sure both R1 & R2 resistors have the same ohm values
//      2. Add the Vcc.h and Vcc.cpp files to your Arduino folder before uploading
//visit https://www.youtube.com/delali to learn more about electronics and building smart things from scratch
////////////////////////////////////////////////

#include <Vcc.h>

const float VccMin   = 0.0;           // Minimum expected Vcc level, in Volts.
const float VccMax   = 3.303;         // Maximum expected Vcc level, in Volts(Use a multimeter to measure the maximum output voltage of your regulator. The more decimal places the better).
const float VccCorrection = 0.98544;  // Tweak this value

Vcc vcc(VccCorrection);

//BATTERY IS CONNECTED TO A0
int batt_pin = A0;

void setup() {
  Serial.begin(115200);
}

void loop() {
  //READ CURRENT REGULATOR OUTPUT VOLTAGE(VCC)
  //COLLECT 100 VCC VOLTAGE SAMPLES
  int j;
  float v = 0;
  for (j = 0; j < 100; j++) {
    v += vcc.Read_Volts();
    delay(5);
  }

  //FIND AVERAGE OF 100 VCC VOLTAGE SAMPLES
  v = v / j;

  //READ CURRENT REGULATOR OUTPUT PERCENTAGE (VccMin - VccMax)
  //float p = vcc.Read_Perc(VccMin, VccMax);
  //Serial.print("VCC = ");
  //Serial.print(p);
  //Serial.println(" %");

  //COLLECT 100 BATTERY VOLTAGE SAMPLES
  int i;
  float batt_volt = 0;
  for (i = 0; i < 100; i++) {
    //USING THE CURRENT VCC VOLTAGE AS REFFERENCE TO CALCULATE BATTERY VOLTAGE
    batt_volt += ((analogRead(batt_pin) * (v / 1023.0)) * 2);
    delay(5);
  }

  //FIND AVERAGE OF 100 BATTERY VOLTAGE SAMPLES
  batt_volt = batt_volt / i;

  Serial.print("Battery: ");
  Serial.print(batt_volt, 2);
  Serial.print("V ");
  //CONVERTING BATTERY VOLTAGE TO PERCENTAGE ASSUMING 4.19V AS THE FULL CHARGE VALUE
  Serial.print((batt_volt / 4.19) * 100);
  Serial.println("%");
}
