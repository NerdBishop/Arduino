#include <SoftwareSerial.h>

SoftwareSerial mySerial(10, 11); // RX, TX

int ldr = A0;
bool led_state = false;
int led_pin = 2;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(19200);

  // set the data rate for the SoftwareSerial port
  mySerial.begin(9600);
  pinMode(ldr, INPUT);
  pinMode(led_pin, OUTPUT);
}

void loop() { // run over and over
  if (mySerial.available() > 0) {
    String data = mySerial.readStringUntil('\n');
    Serial.println(data);
    
    if(data.equals("toggle")){
      led_state = !led_state;
      digitalWrite(led_pin, led_state);
    }
  }

  int ldr_val = analogRead(ldr);
  ldr_val = map(ldr_val, 0, 1023, 0, 255);
  
  mySerial.println(ldr_val);
  delay(10);
}
