/*
 * Arduino watchdog sample
 * some Arduio boards have watchdog enabled on reboot
 * and if you do not issue a reset quickly then watchdog
 * will reset the board again causing an eternal reboot.
 *
 * Uno has the optiboot bootloader and we should not have 
 * the eternal reboot issue on Uno.
 *
 * @see https://code.google.com/p/arduino/issues/detail?id=181
 * @see http://forum.arduino.cc/index.php?topic=128717.0
 * 
 *
 */

#include <avr/wdt.h>

void setup () {
  Serial.begin (9600);
  Serial.println ("Restarted.");
  // reset after one second if no pat received
  wdt_enable (WDTO_1S);  
 }

void loop () {
  Serial.println ("Entered loop ...");
  wdt_reset ();  
  while (true) ;
} 
