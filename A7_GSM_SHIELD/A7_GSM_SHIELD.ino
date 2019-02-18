#include <SoftwareSerial.h>

char phone_no[] = "XXXXXXXXXX";
char textmessage[] = "Hola!";
const int pwr_key = 9;
const char * host = "api.thingspeak.com";
const char * url = "/apps/thinghttp/send_request?api_key=XXXXXXXXXX";

SoftwareSerial gsm(7, 8); //RX, TX

void setup() {
  gsm.begin(115200);
  Serial.begin(115200);
  pinMode(pwr_key, OUTPUT);
  digitalWrite(pwr_key, HIGH);
  delay(2000);
  digitalWrite(pwr_key, LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.available()){
    String command = Serial.readStringUntil('\r');
    if(command.startsWith("AT")){
      gsm.println(command);
    }
    
    if(command.equals("call")){
        DialVoiceCall();
      }else if(command.equals("sms")){
        SendTextMessage();
      }else if(command.equals("https")){
        SubmitHttpData();
      }else if(command.equals("httpg")){
        GetHttpData();
      }else if(command.equals("sig")){
        GetSignalQuality();
      }else if(command.equals("cls")){
        clear_sim();
      }
  }
  ShowSerialData();
}

void ShowSerialData(){
  while(gsm.available()){
    char c = gsm.read();
    Serial.print(c);
  }
}

void clear_sim(){
  gsm.println("AT+CMGD=1");
  delay(1000);
  gsm.println("AT+CMGD=2");
  delay(1000);
  gsm.println("AT+CMGD=3");
  delay(1000);
  gsm.println("AT+CMGD=4");
  delay(1000);
  gsm.println("AT+CMGD=5");
  delay(1000);
}

void SendTextMessage(){ 
   gsm.println("AT+CMGF=1");
   delay(1000);
   gsm.print("AT+CMGS=\"");
   gsm.print(phone_no);
   gsm.write(0x22);
   gsm.write(0x0D); // hex equivalent of Carraige return
   gsm.write(0x0A); // hex equivalent of newline
   delay(1000);
   gsm.print(textmessage);
   delay(500);
   gsm.print(char(26)); //the ASCII code of the ctrl+z is 26
}

void DialVoiceCall(){
   gsm.print("ATD");
   gsm.print(phone_no);
   delay(1000);
   gsm.println("ATH");
}

void SubmitHttpData(){

}

void GetHttpData(){
  delay(5000);
  gsm.println("AT+CGATT=1");
  ShowSerialData();
  delay(1000);
  gsm.println("AT+CGDCONT=1,\"IP\",browse\"\"");
  ShowSerialData();
  delay(2000);
  gsm.println("AT+CGACT=1,1");
  delay(1000);
  ShowSerialData();
  gsm.println("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",80");
  delay(5000);
  ShowSerialData();
  gsm.println("AT+CIPSEND");
  delay(2000);
  gsm.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  delay(2000);
  gsm.print(char(26));//the ASCII code of the ctrl+z is 26
  ShowSerialData();
}

void GetSignalQuality(){
   gsm.println("AT+CSQ");
   Serial.println("+CSQ: XX,QQ : Signal Quality is poor when the XX is \"99\"");
}
