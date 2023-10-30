////////////////////////////////////////////////
//
//This code was inspired by:
//https://github.com/Yveaux/Arduino_Vcc
//I made a few modifications to read out the volatage of batteries attached to the arduino.
//The positive terminal of the battery should be connected to Analog pin A0 through a voltage divider(both R1 & R2 are 10k ohms each)
//NOTE: 1. Always make sure both R1 & R2 resistors have the same ohm values(even if you use resistor values of your choice)
//      2. Add the Vcc.h and Vcc.cpp files to your Arduino libraries folder before uploading
//visit https://www.youtube.com/delali to learn more about electronics and building smart things from scratch
//
////////////////////////////////////////////////

#include <Vcc.h>

//MORE DECIMAL PLACES LEAD TO MUCH ACCURATE BATTERY READING(thus for VccMax and VccCorrection)
const float VccMin   = 0.0;           // Minimum expected Vcc level, in Volts.
const float VccMax   = 3.303;         // Maximum expected Vcc level, in Volts(Use a multimeter to measure the maximum output voltage of your regulator. Thus when you connect a fully charged battery to it).
const float VccCorrection = 0.98544;  // Tweak this value until the output of "v" on line 38 is the same as what you measured with your multimeter above for VccMax

const float lowBat = 2.9; //Indicate the value your battery manufacturer states as absolutely dead battery
const float fullBat = 4.2; //Indicate the value your fully charged battery will be at

Vcc vcc(VccCorrection);

//BATTERY IS CONNECTED TO A0
int batt_pin = A0;

void setup() {
  Serial.begin(9600);
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

  //PRINTING AVERAGE OF 100 VCC VOLTAGE SAMPLES TO 3 DECIMAL PLACES
  Serial.println();
  Serial.print("VCC: ");
  Serial.println(v, 3);

  
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
  Serial.print(((batt_volt - lowBat )/ (fullBat - lowBat)) * 100);
  Serial.println("%");

  delay(100);
}
