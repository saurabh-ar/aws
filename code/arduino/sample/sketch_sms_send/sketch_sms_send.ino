#include <SoftwareSerial.h>

SoftwareSerial gsmSerial(10, 11);

void setup() {
 
  Serial.begin(9600);
  gsmSerial.begin(9600);
  
  gsmSerial.println("AT+CMGF=1");
  delay(200);
  gsmSerial.println("AT+CSMS=1");
  delay(200);
  gsmSerial.println("AT+CNMI=2,2,0,0");
  delay(200);
 
 
}

void loop() {
 uint8_t count = 1 ;
 
 while(count < 3) {
    gsmSerial.print("\r");  
    delay(1000);
    gsmSerial.print("AT+CMGF=1\r");
    delay(1000);
    gsmSerial.print("AT+CMGS=\"+919886124428\"\r"); 
    delay(1000);
    gsmSerial.print("yuktix sms test @");
    gsmSerial.print(count);
    gsmSerial.println();
    gsmSerial.write(0x1A);
    count++ ;
    delay(3000); 
    
    
  }
  
  // keep waiting
  while(1);
  
  
}
