
/* ------------------------------------------------------------------------------- */
// character LCD example code
// www.hacktronics.com

#include <LiquidCrystal.h>
//pin 8to13 used for interfacing via sheild 
LiquidCrystal lcd(8, 9, 10, 11, 12, 13);

int backLight = 13;    // pin 13 will control the backlight

void setup()
{
  pinMode(backLight, OUTPUT);
  digitalWrite(backLight, HIGH);
  lcd.begin(16,2);       
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Yuktix H60.3L98");
  lcd.setCursor(0,1);
  lcd.print("T34.2P101.2R1.20");
 }

void loop()
{
}
