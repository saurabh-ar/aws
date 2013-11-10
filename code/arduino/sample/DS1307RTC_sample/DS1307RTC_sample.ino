#include <Wire.h>
#include <Time.h>
#include <DS1307RTC.h>
#include <AWS.h>

// DS1307RTC instance created in DS1307 lib
// DS1307RTC RTC = DS1307RTC();
// DS1307RTC constructor calls Wire.begin()


const char *month_names[12] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

tmElements_t tm;
aws_error_t aws_code ;

void setup() {
  Serial.begin(9600);

  aws_code = init_rtc(__TIME__,__DATE__);
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

aws_error_t init_rtc(const char* str_time, const char* str_date) {

  int hour, min, sec;
  char month[12]; int day, year;
  uint8_t month_index;

  if (sscanf(str_time, "%d:%d:%d", &hour, &min, &sec) != 3) return AWS_RTC_COMPILER_ERROR;
  if (sscanf(str_date, "%s %d %d", month, &day, &year) != 3) return AWS_RTC_COMPILER_ERROR;

  tm.Hour = hour;
  tm.Minute = min;
  tm.Second = sec;


  for (month_index = 0; month_index < 12; month_index++) {
    if (strcmp(month, month_names[month_index]) == 0) break;
  }

  if (month_index >= 12) return AWS_RTC_COMPILER_ERROR;

  tm.Day = day;
  tm.Month = month_index + 1;
  tm.Year = CalendarYrToTm(year);

  // set this date time to DS1307 chip
  bool flag = RTC.write(tm);
  if(!flag) return AWS_RTC_WRITE_ERROR ;

  return AWS_RTC_ERROR_NONE;

}
