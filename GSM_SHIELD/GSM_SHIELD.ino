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
/*
  gsm.println("AT+CSAPBR=3,1,\"Contype\",\"GPRS\"");
  ShowSerialData();
  delay(5000);
  
  gsm.println("AT+CSAPBR=3,1,\"APN\",\"browse\""); //need to put your provider's APN here
  ShowSerialData();
  delay(5000);
  
  gsm.println("AT+CSAPBR=1,1");
  ShowSerialData();
  delay(5000);
  
  gsm.println("AT+CHTTPINIT");
  ShowSerialData();
  delay(5000);
  
  gsm.println("AT+CHTTPPARA=\"CID\",1");
  ShowSerialData();
  delay(5000);
  
  gsm.println("AT+CHTTPPARA=\"URL\",\"api.thingspeak.com\""); //web address to send data to
  ShowSerialData();
  delay(5000);
  
  gsm.println("AT+CHTTPDATA=100,10000"); //100 refers to how many bytes you're sending.  You'll probably have to tweak or just put a large #
  delay(5000);
  
  gsm.println("/apps/thinghttp/send_request?api_key=71QFQ61B302G21TY"); //ie latitude,longitude,etc...
  ShowSerialData();
  delay(5000);
  
  gsm.println("AT+CHTTPACTION=1"); //POST the data
  ShowSerialData();
  delay(5000);
  
  gsm.println("AT+CHTTPTERM"); //terminate http
  ShowSerialData();
  delay(5000);
  
  gsm.println("AT+CSAPBR=0,1");// Disconnect GPRS
  ShowSerialData();
  delay(5000);
  */
  //gsm.println("AT+CREG?");
  //ShowSerialData();
  delay(5000);
  gsm.println("AT+CGATT=1");
  ShowSerialData();
  delay(1000);
  //gsm.println("AT+AGPS=0");
  //delay(2000);
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
  //ShowSerialData();
  delay(2000);
  //gsm.print("GET /apps/thinghttp/send_request?api_key=71QFQ61B302G21TY HTTP/1.1\r\nHost:api.thingspeak.com\r\n");
  //gsm.print("GET https://api.thingspeak.com/apps/thinghttp/send_request?api_key=71QFQ61B302G21TY HTTP/1.1");
  //gsm.print("GET api.thingspeak.com/apps/thinghttp/send_request?api_key=71QFQ61B302G21TY HTTP/1.1");
  //gsm.print("GET /2vJtCLh HTTP/1.1\r\nHost:bit.ly\r\n");
  //gsm.print("GET /2vJtCLh HTTP/1.1");
  //gsm.print("GET bit.ly/2vJtCLh HTTP/1.1\r\n");
  gsm.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  delay(2000);
  //ShowSerialData();
  gsm.print(char(26));//the ASCII code of the ctrl+z is 26
  //delay(10000);
  ShowSerialData();
//https://bit.ly/2vJtCLh
}

void GetSignalQuality(){
   gsm.println("AT+CSQ");
   Serial.println("Tips:+CSQ: XX,QQ : It means the Signal Quality poor when the XX is '99'!");
}
