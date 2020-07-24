#include <TinyGPS++.h>
#include <SoftwareSerial.h>

static const int RXPin = 4, TXPin = 3;
static const uint32_t GPSBaud = 9600;

TinyGPSPlus gps;

SoftwareSerial ss(RXPin, TXPin);
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  ss.begin(GPSBaud);
}

void loop() {
    //new sentence is correctly encoded.
  while (ss.available() > 0)
    if (gps.encode(ss.read()))
      displayInfo();

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while(true);
  }
}


void displayInfo(){
  if(gps.satellites.isValid() && gps.location.isValid() && gps.altitude.isValid() ){
     Serial.print(F("Sats: "));
     Serial.print(gps.satellites.value());
     Serial.print(F(" Location: "));
     Serial.print(gps.location.lat(), 6);
     Serial.print(F(" "));
     Serial.print(gps.location.lng(), 6);
     Serial.print(F(" Altitude: ")); 
     Serial.println(gps.altitude.meters());
  }else{
    Serial.print(F("INVALID"));
  }
  delay(1000);
}

