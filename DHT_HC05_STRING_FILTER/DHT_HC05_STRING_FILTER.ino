#include <DHT.h>
#define DHTPIN 2 //Digital Pin 2
#define DHTTYPE DHT22 //DHT11 If you have a DH11
DHT dht(DHTPIN, DHTTYPE);

const int led_pin = 13; //LED Pin

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); //Starting Serial Communication
  dht.begin(); //Initiallising DHT Sensor
  pinMode(led_pin, OUTPUT); //

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
      String c = Serial.readString();
      //uncomment the line below and comment the line above(Preference)
      //String c = Serial.readStringUntil("\n");
      
      c = c.substring(c.indexOf("#") + 1,c.lastIndexOf("#"));
      Serial.println(c);
      
      if(c.equals("on")){
        digitalWrite(led_pin, HIGH);
      }else if(c.equals("off")){
        digitalWrite(led_pin, LOW);
      }
    }
  }
 
  delay(500);

 
  
}
