#include <Wire.h>
#include <Time.h>
#include <DS1307RTC.h>
#include <AWS.h>

// DS1307RTC instance created in DS1307 lib
// DS1307RTC RTC = DS1307RTC();
// DS1307RTC constructor calls Wire.begin()

#define DS1307_CTRL_ID 0x68 

aws_error_t aws_code ;

void setup() {
  Serial.begin(9600);

  aws_code = init_rtc();
  for(int i = 0 ; i < 200 ; i++) {
      delayMicroseconds(1000);
  }

  // set DS1307 RTC as time provider
  // sync interval is default 5 minutes
  if(aws_code == AWS_RTC_ERROR_NONE) {
      setSyncProvider(RTC.get);   
      if(timeStatus()!= timeSet) {
          // wait a bit
          for(int i = 0 ; i < 4000 ; i++) {
              delayMicroseconds(1000);
          }
      }

      // try again
      if(timeStatus()!= timeSet) {
          aws_code = AWS_RTC_SYNC_ERROR ;
      }
  }  

}

void loop() {

    if(aws_code == AWS_RTC_ERROR_NONE ) {
        Serial.print(hour());
        Serial.print(" ");
        Serial.print(minute());
        Serial.print(" ");
        Serial.println(second());
    } else {
        Serial.print("error in RTC setup");
        Serial.println(aws_code);

    }

    delay(1000);
}

uint8_t dec2bcd(uint8_t num)
{
  return ((num/10 * 16) + (num % 10));
}

aws_error_t init_rtc() {

    aws_error_t code ;
    byte second =      00; //0-59
    byte minute =      19; //0-59
    byte hour =        21; //0-23
    byte weekDay =     4; //1-7
    byte monthDay =    27; //1-31
    byte month =       10; //1-12
    byte year  =       13; //0-99

    Wire.beginTransmission(DS1307_CTRL_ID);
    Wire.write((uint8_t)0x00); 

    Wire.write(dec2bcd(second));
    Wire.write(dec2bcd(minute));
    Wire.write(dec2bcd(hour));
    Wire.write(dec2bcd(weekDay));
    Wire.write(dec2bcd(monthDay));
    Wire.write(dec2bcd(month));
    Wire.write(dec2bcd(year));

    Wire.write((uint8_t)0x00); 

    if(Wire.endTransmission() != 0) {
        code = AWS_RTC_WRITE_ERROR ;
    } else {
        code = AWS_RTC_ERROR_NONE;
    }

    return code;

}
