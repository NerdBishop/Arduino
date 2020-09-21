#include <SoftwareSerial.h>

SoftwareSerial mySerial(10, 11); // RX, TX
int led_pin = 3;
int button_pin = 8;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(19200);

  // set the data rate for the SoftwareSerial port
  mySerial.begin(9600);

  pinMode(led_pin, OUTPUT);
  pinMode(button_pin, INPUT_PULLUP);
}

void loop() { // run over and over
  if (mySerial.available() > 0) {
    String data = mySerial.readStringUntil('\n');
    Serial.println(data);
    
    int val = data.toInt();
    analogWrite(led_pin, val);
  }

  if(digitalRead(button_pin) == LOW){
    mySerial.write("toggle");
    delay(500);
  }
    
}
