#include <EEPROM.h>

//holds the code retried from memory
String code_stored;

void setup() {
  Serial.begin(38400);
  
  //test code im using;
  String code = "6709";

  //function for writing the code to memory
  Write_Mem(code);
  delay(10);
}

void loop() {
  
  //function for getting user code 
  Read_Mem();
  delay(5000);
}

void Write_Mem(String num){
  //writing 4 digit code to memory
  if(num.length() == 4 ){
    int b = 0; 
    int e = 1;
    for(int addr = 0; addr < 4; addr++){
      int val = num.substring(b,e).toInt();
      if (!(addr == EEPROM.length())) {
        EEPROM.write(addr, val);
      }
      b += 1;
      e += 1;
    }
    Serial.println("Stored");
  }
}

void Read_Mem(){
  //retrieving 4 digit code stored in eeprom
  String retrieved = "";
  byte value;
  for(int addr = 0; addr < 4; addr++){
    value = EEPROM.read(addr);
    retrieved += String(value, DEC);
  }

  //checking to make sure the value retrieved from eeprom is 4 digits
  if(retrieved.length() == 4 ){
    code_stored = retrieved;
    Serial.println(code_stored);
  }
}
