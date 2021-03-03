#include <WaziDev.h>
#include <xlpp.h>
#include <Base64.h>

// NwkSKey (Network Session Key) and Appkey (AppKey) are used for securing LoRaWAN transmissions.
// You need to copy them from/to your LoRaWAN server or gateway.
// You need to configure also the devAddr. DevAddr need to be different for each devices!!
// Copy'n'paste the DevAddr (Device Address): 26011D00
unsigned char devAddr[4] = {0x26, 0x01, 0x1D, 0x00};

// Copy'n'paste the key to your Wazigate: 23158D3BBC31E6AF670D195B5AED5525
unsigned char appSkey[16] = {0x23, 0x15, 0x8D, 0x3B, 0xBC, 0x31, 0xE6, 0xAF, 0x67, 0x0D, 0x19, 0x5B, 0x5A, 0xED, 0x55, 0x25};

// Copy'n'paste the key to your Wazigate: 23158D3BBC31E6AF670D195B5AED5525
unsigned char nwkSkey[16] = {0x23, 0x15, 0x8D, 0x3B, 0xBC, 0x31, 0xE6, 0xAF, 0x67, 0x0D, 0x19, 0x5B, 0x5A, 0xED, 0x55, 0x25};

// A valid payload for the downlink for this sketch has been generated like this:
// > xlpp -e '{"colour0":"#ffaa00","switch1":true}'
// < AIf/qgABjgE=
// You can paste the base64 payload to Chirpstack.
// On Windows, you might need to escape it like this:
// > xlpp -e "{"""colour0""":"""#ffaa00""","""switch1""":true}"

////////////////////////
#include <DHT.h>
#define DHTPIN 7
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
///////////////////////

WaziDev wazidev;

int readBase64(const void *buf, int len)
{
    int encLen = base64_enc_len(len);
    char* h = (char*) malloc(encLen);
    base64_decode(h, buf, len); 
    Serial.println(h);
    String hc = String(h);
    
    if(hc.equals("true")){
      Serial.print("SOMTIN WAS TURNED ON");
    }else if(hc.equals("false")){
      Serial.print("SOMTIN WAS TURNED OFF");
    }
    
    free(h);
    return encLen;
}

void setup()
{
  Serial.begin(38400);
  wazidev.setupLoRaWAN(devAddr, appSkey, nwkSkey);
  dht.begin();
}

XLPP xlpp(120);

void loop(void)
{
  ///////////////////////
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  ///////////////////////
  // 1
  // Create xlpp payload.
  xlpp.reset();
  xlpp.addTemperature(1, temp); // °C
  xlpp.addRelativeHumidity(1, hum);
  xlpp.addLuminosity(1, 55.67);
  
  // 2.
  // Send paload with LoRaWAN.
  serialPrintf("LoRaWAN send ... ");
  uint8_t e = wazidev.sendLoRaWAN(xlpp.buf, xlpp.len);
  if (e != 0)
  {
    serialPrintf("Err %d\n", e);
    delay(30000);
    return;
  }
  serialPrintf("OK\n");
  
  // 3.
  // Receive LoRaWAN message (waiting for 6 seconds only).
  serialPrintf("LoRa receive ... ");
  uint8_t offs = 0;
  long startSend = millis();
  e = wazidev.receiveLoRaWAN(xlpp.buf, &xlpp.offset, &xlpp.len, 6000);
  long endSend = millis();
  if (e != 0)
  {
    if (e == ERR_LORA_TIMEOUT){
      serialPrintf("nothing received\n");
    }
    else
    {
      serialPrintf("Err %d\n", e);
    }
    delay(30000);
    return;
  }
  serialPrintf("OK\n");
  
  serialPrintf("Time On Air: %d ms\n", endSend-startSend);
  serialPrintf("LoRa SNR: %d\n", wazidev.loRaSNR);
  serialPrintf("LoRa RSSI: %d\n", wazidev.loRaRSSI);
  serialPrintf("LoRaWAN frame size: %d\n", xlpp.offset+xlpp.len);
  serialPrintf("LoRaWAN payload len: %d\n", xlpp.len);
  serialPrintf("Payload: ");
  readBase64(xlpp.getBuffer(), xlpp.len);
  serialPrintf("\n");

  // 4.
  // Read xlpp message.
  // You must use the following pattern to properly parse xlpp payload.
  int end = xlpp.len + xlpp.offset;
  while (xlpp.offset < end)
  {
    // Always read the channel first ...
    uint8_t chan = xlpp.getChannel();
    serialPrintf("Chan %2d: ", chan);

    // ... then the type ...
    uint8_t type = xlpp.getType();

    // ... then the value!
    switch (type) {
      case LPP_DIGITAL_OUTPUT:
      {
        uint8_t v = xlpp.getDigitalInput();
        serialPrintf("Digital Output: %hu (0x%02x)\n", v, v);
        break;
      }
      case LPP_ANALOG_OUTPUT:
      {
        float f = xlpp.getAnalogOutput();
        serialPrintf("Analog Output: %.2f\n", f);
        break;
      }
      case LPP_COLOUR:
      {
        Colour c = xlpp.getColour();
        serialPrintf("Colour: R %d, G %d, B %d (#%02X%02X%02X)\n", c.r, c.g, c.b, c.r, c.g, c.b);
        break;
      }
      case LPP_SWITCH:
      {
        uint8_t v = xlpp.getSwitch();
        serialPrintf("Switch: %s\n", v?"on":"off");
        break;
      }
      default:
        // For all the other types, see https://github.com/Waziup/arduino-xlpp/blob/main/test/simple/main.cpp#L147
        serialPrintf("Other unknown type.\n");
        delay(30000);
        return;
    }
  }
  
  delay(30000);
}
