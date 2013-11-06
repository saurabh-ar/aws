#include <DHT22.h>

#include "Wire.h"
#include "Adafruit_BMP085.h"
#define DHT22_PIN 7
Adafruit_BMP085 bmp;


// Setup a DHT22 instance
DHT22 myDHT22(DHT22_PIN);

void setup() {
  Serial.begin(9600);
  bmp.begin();  
  Serial.println("DHT22 + BMP085 demo");
}
 
void loop() {
  DHT22_ERROR_t errorCode;

  delay(2000);
  Serial.print("Requesting data...");
  errorCode = myDHT22.readData();
  if(errorCode == DHT_ERROR_NONE)
  {
    Serial.println("Got Data ");
    Serial.print(myDHT22.getTemperatureC());
    Serial.println("C ");
    Serial.print(myDHT22.getHumidity());
    Serial.println("%");
    
    Serial.println("Getting data from BMP085...");
    Serial.print("Temperature = ");
    Serial.print(bmp.readTemperature());
    Serial.println(" *C");
 
    Serial.print("Pressure = ");
    Serial.print(bmp.readPressure());
    Serial.println(" Pa");
    Serial.println();
    delay(1000);
}
  else
  {
    Serial.print("Error Code ");
    Serial.print(errorCode);
    Serial.println(" readData Failed");
  } 
    
    
}
