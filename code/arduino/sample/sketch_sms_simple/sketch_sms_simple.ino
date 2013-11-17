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
 while(1) {
    gsmSerial.print("\r");  
    delay(1000);
    gsmSerial.print("AT+CMGF=1\r");
    delay(1000);
    gsmSerial.print("AT+CMGS=\"+918553518338\"\r"); 
    delay(1000);
    gsmSerial.print("yuktix is great");
    gsmSerial.println();
    gsmSerial.write(0x1A);
    delay(3000); 
    break;
  }
  
  if (gsmSerial.available()) {
    Serial.write(gsmSerial.read());
       
  }
  
}
