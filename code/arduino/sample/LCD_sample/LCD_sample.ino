
/* ------------------------------------------------------------------------------- */
// character LCD example code
// www.hacktronics.com

#include <LiquidCrystal.h>

// Connections:
// rs (LCD pin 4) to Arduino pin 12
// rw (LCD pin 5) to Arduino pin 11
// enable (LCD pin 6) to Arduino pin 10
// LCD pin 15 to Arduino pin 13
// LCD pins d4, d5, d6, d7 to Arduino pins 5, 4, 3, 2
// DONT USE 2,3 pins. They will be used for interrupts. 
LiquidCrystal lcd(12, 11, 10, 5, 4, 3, 2);

int backLight = 13;    // pin 13 will control the backlight

void setup()
{
  pinMode(backLight, OUTPUT);
  digitalWrite(backLight, HIGH);
  lcd.begin(16,2);       
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("H60.3 R1.20");
  lcd.setCursor(0,1);
  lcd.print("T34.2 L98 P101.2");
 }

void loop()
{
}
