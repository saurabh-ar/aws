#include <DHT22.h>

#include <LiquidCrystal.h>
//pin 8to13 used for interfacing via sheild 

#include "Wire.h"
#include "Adafruit_BMP085.h"
#define DHT22_PIN 6
Adafruit_BMP085 bmp;
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

int backLight = 13;    // pin 13 will control the backlight

// Setup a DHT22 instance
DHT22 myDHT22(DHT22_PIN);

void setup() {
  Serial.begin(9600);
  bmp.begin();  
  Serial.println("DHT22 + BMP085 demo");
  pinMode(backLight, OUTPUT);
  digitalWrite(backLight, HIGH);
  lcd.begin(16,2);       
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Yuktix");
  lcd.setCursor(0,1);
  lcd.print("is AWESOME !!!! :D ");
}
 
void loop() {
  DHT22_ERROR_t errorCode;

  delay(2000);
  Serial.print("Requesting data...");
  lcd.print("Requesting data...");
  errorCode = myDHT22.readData();
  if(errorCode == DHT_ERROR_NONE)
  {
    Serial.println("Got Data ");
    lcd.clear();
    lcd.print("Got Data ");
    
    Serial.print(myDHT22.getTemperatureC());
    Serial.println("C ");
    Serial.print(myDHT22.getHumidity());
    Serial.println("%");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("T");
    lcd.print(myDHT22.getTemperatureC());
    lcd.print("C");
    lcd.print(" A");
    lcd.print(bmp.readAltitude());
    lcd.print("m");
    
    Serial.println("Getting data from BMP085...");
    Serial.print("Temperature = ");
    Serial.print(bmp.readTemperature());
    Serial.println(" *C");
    
    Serial.print("Pressure = ");
    Serial.print(bmp.readPressure());
    lcd.setCursor(0,1);
    lcd.print("P");    
    lcd.print(bmp.readPressure());
    lcd.print("Pa");
    lcd.print(" H");
    lcd.print(myDHT22.getHumidity());
    lcd.print("%");
    Serial.println();

  // Calculate altitude assuming 'standard' barometric
    // pressure of 1013.25 millibar = 101325 Pascal
    Serial.print("Altitude = ");
    Serial.print(bmp.readAltitude());
    Serial.println(" meters");
   
    delay(1500);
}
  else
  {
    Serial.print("Error Code ");
    Serial.print(errorCode);
    Serial.println(" readData Failed");
  } 
    
    
}
