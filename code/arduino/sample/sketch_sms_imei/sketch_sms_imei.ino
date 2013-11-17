#include <SoftwareSerial.h>

SoftwareSerial gsmSerial(10, 11);
int flag = 1 ;

void setup() {
 
  Serial.begin(9600);
  gsmSerial.begin(9600);
 
}

void loop() {
  
     
   if(flag == 1 ) {
     Serial.println("** sending AT command ** ");
     gsmSerial.println("AT");  
     delay(500);
     gsmSerial.println("AT+CGMI");
     delay(200);
     gsmSerial.println("AT+CGSN");
     delay(500);
     flag = 2 ;
     
   }
   
   if (gsmSerial.available()) {
    Serial.write(gsmSerial.read());
   } else {
    delay(1000); 
   }
}
