#include <WaziDev.h>
#include <xlpp.h>
#include <Base64.h>
#include "LowPower.h"
#include <DHT.h>
#include <Vcc.h>

DHT dht(3, DHT11);

//MORE DECIMAL PLACES LEAD TO MUCH ACCURATE BATTERY READING(thus for VccMax and VccCorrection)
const float VccMin   = 0.0;           // Minimum expected Vcc level, in Volts.
const float VccMax   = 3.311;         // Maximum expected Vcc level, in Volts(Use a multimeter to measure the maximum output voltage of your regulator. Thus when you connect a fully charged battery to it).
const float VccCorrection = 0.98999; // Tweak this value until the output of "v" on line 38 is the same as what you measured with your multimeter above for VccMax

const float lowBat = 2.9; //Indicate the value your battery manufacturer states as absolutely dead battery
const float fullBat = 4.2; //Indicate the value your fully charged battery will be at

Vcc vcc(VccCorrection);

//BATTERY IS CONNECTED TO A0
int batt_pin = A0;

int sleep_count = 0;
const int mosfet = 7;
int num_transm = 0;

//sleep_count 75 is 10 mins
//sleep_count 150 is 20 mins
//sleep_count 225 is 30 mins
//sleep_count 300 is 40 mins
//sleep_count 375 is 50 mins
//sleep_count 450 is 60 mins

// NwkSKey (Network Session Key) and Appkey (AppKey) are used for securing LoRaWAN transmissions.
// You need to copy them from/to your LoRaWAN server or gateway.
// You need to configure also the devAddr. DevAddr need to be different for each devices!!
// Copy'n'paste the DevAddr (Device Address): 26011D24
unsigned char devAddr[4] = {0x26, 0x01, 0x1D, 0x24};

// Copy'n'paste the key to your Wazigate: 23158D3BBC31E6AF670D195B5AED5525
unsigned char appSkey[16] = {0x23, 0x15, 0x8D, 0x3B, 0xBC, 0x31, 0xE6, 0xAF, 0x67, 0x0D, 0x19, 0x5B, 0x5A, 0xED, 0x55, 0x25};

// Copy'n'paste the key to your Wazigate: 23158D3BBC31E6AF670D195B5AED5525
unsigned char nwkSkey[16] = {0x23, 0x15, 0x8D, 0x3B, 0xBC, 0x31, 0xE6, 0xAF, 0x67, 0x0D, 0x19, 0x5B, 0x5A, 0xED, 0x55, 0x25};

WaziDev wazidev;

void setup()
{
  ///Serial.begin(38400);
  pinMode(mosfet, OUTPUT);
  delay(500);
  dht.begin();
  delay(500);
  wazidev.setupLoRaWAN(devAddr, appSkey, nwkSkey);
}

XLPP xlpp(120);

void loop(void)
{
  if (sleep_count == 75) {
    sleep_count = 0;
    num_transm += 1;
    
    int q = 0;
    float v = 0;
    for (q = 0; q < 50; q++) {
      v += vcc.Read_Volts();
      delay(3);
    }

    //FIND AVERAGE OF 50 VCC VOLTAGE SAMPLES
    v = v / q;

    //COLLECT 50 BATTERY VOLTAGE SAMPLES
    int i = 0;
    float batt_volt = 0;
    for (i = 0; i < 100; i++) {
      //USING THE CURRENT VCC VOLTAGE AS REFFERENCE TO CALCULATE BATTERY VOLTAGE
      batt_volt += ((analogRead(batt_pin) * (v / 1023.0)) * 2);
      delay(3);
    }

    //FIND AVERAGE OF 100 BATTERY VOLTAGE SAMPLES
    batt_volt = batt_volt / i;
    //batt_volt = (int)(batt_volt * 100 + .5);
    //batt_volt = (float)batt_volt / 100;
    ///Serial.println(batt_volt);
    digitalWrite(mosfet, HIGH);
    delay(1500);

    float h = dht.readHumidity();;
    float t = dht.readTemperature();

    ///Serial.print(h);
    ///Serial.print(" %\t");
    ///Serial.print(t);
    ///Serial.println(" °C");

    // 1
    // Create xlpp payload.
    xlpp.reset();

    xlpp.addTemperature(1, t);
    xlpp.addRelativeHumidity(2, h);
    xlpp.addVoltage(3, batt_volt);
    xlpp.addInteger(4, num_transm);

    // 2.
    // Send paload with LoRaWAN.
    ///serialPrintf("LoRaWAN send ... ");
    uint8_t e = wazidev.sendLoRaWAN(xlpp.buf, xlpp.len);
    if (e != 0)
    {
      ///serialPrintf("Err %d\n", e);
      delay(5000);
      return;
    }
    ///serialPrintf("OK\n");

    // 3.
    // Receive LoRaWAN message (waiting for 6 seconds only).
    ///serialPrintf("LoRa receive ... ");
    uint8_t offs = 0;
    long startSend = millis();
    e = wazidev.receiveLoRaWAN(xlpp.buf, &xlpp.offset, &xlpp.len, 6000);
    long endSend = millis();
    if (e != 0)
    {
      if (e == ERR_LORA_TIMEOUT) {
        ///serialPrintf("nothing received\n");
      }
      else
      {
        ///serialPrintf("Err %d\n", e);
      }
      delay(5000);
      return;
    }
    ///serialPrintf("OK\n");
/*
    serialPrintf("Time On Air: %d ms\n", endSend - startSend);
    serialPrintf("LoRa SNR: %d\n", wazidev.loRaSNR);
    serialPrintf("LoRa RSSI: %d\n", wazidev.loRaRSSI);
    serialPrintf("LoRaWAN frame size: %d\n", xlpp.offset + xlpp.len);
    serialPrintf("LoRaWAN payload len: %d\n", xlpp.len);
    serialPrintf("Payload: ");*/
    char payload[100];
    base64_decode(payload, xlpp.getBuffer(), xlpp.len);
    ///serialPrintf(payload);
    ///serialPrintf("\n");
    delay(1000);

  } else {
    sleep_count += 1;
    digitalWrite(mosfet, LOW);
    sx1272.setSleepMode();
    delay(50);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  }
}
