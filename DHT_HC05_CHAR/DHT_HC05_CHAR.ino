#include <DHT.h>
#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

const int led_pin = 13;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  dht.begin();
  pinMode(led_pin, OUTPUT);

}

void loop() {
  // put your main code here, to run repeatedly:
 int temp = dht.readTemperature();
 int hum = dht.readHumidity();

  Serial.print("Temp: ");
  Serial.print(temp);
  Serial.println(" C");

  Serial.print("Hum: ");
  Serial.print(hum);
  Serial.println(" %");

  Serial.println("");

  if(Serial.available() > 0){
    while(Serial.available() > 0){
      char c = Serial.read();
      Serial.println(c);
      switch(c){
        case 'a':
          digitalWrite(led_pin, HIGH);
          break;
        case 'b':
          digitalWrite(led_pin, LOW);
          break;
        default:
          break;
      }
    }
  }
 
  delay(500);

 
  
}
