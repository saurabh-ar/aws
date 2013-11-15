#include <Wire.h>
#include <Time.h>
#include <DS1307RTC.h>

// sample to ensure that we can use time library 
// with DS1307 RTC chip. You should have run the 
// DS1307 SetTime example before running this sketch.

// DS1307RTC instance created in DS1307 lib
// DS1307RTC RTC = DS1307RTC();
// DS1307RTC constructor calls Wire.begin()


tmElements_t tm;

void setup() {
    Serial.begin(9600);
    // set DS1307 RTC as provider for time library
    // sync interval is default 5 minutes
    setSyncProvider(RTC.get);   
    micro_delay(20);
    if(timeStatus()!= timeSet) {
        micro_delay(100);
    }

    // try again
    if(timeStatus()!= timeSet) {
        Serial.println("clock is not set, bailing out...");
        for(;;);
    }

}

void micro_delay(int n) {
    for(int i = 0 ; i < n ; i++)
        delayMicroseconds(10000);
}

void loop() {

    int hh = hour();
    int mm = minute();
    int ss = second();
    Serial.print(hh);
    Serial.print(":");
    Serial.print(mm);
    Serial.print(":");
    Serial.println(ss);
    Serial.println();
     
    micro_delay(100);
}
