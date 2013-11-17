
/* 
   DHT11 library
   https://github.com/AnnaGerber/ButterflyProject/tree/master/arduino/libraries/DHT11
   */
   

#include <dht11.h>
#define DHT11PIN 3

dht11 DHT11;
int ldr_val = 0; 
int humidity ;
int temp;

void setup()   
{
  Serial.begin(9600);
}

void loop()   
{ 
  ldr_val = analogRead(3);
  int chk = DHT11.read(DHT11PIN);
  
  Serial.print("temp ");
  Serial.println(DHT11.temperature);
  Serial.print("humidity ");
  Serial.println(DHT11.humidity);
  Serial.print("ldr ");
  Serial.println(ldr_val);  
  Serial.println();
  delay(5000);
}
