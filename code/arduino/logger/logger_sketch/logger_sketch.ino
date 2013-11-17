/*
 * data logger sketch
 *
 * DHT11 for T/H (digital)
 * LDR for reading light (analog)
 * 
 * 
 */


#include <Wire.h>
#include <Time.h>
#include <TimeAlarms.h>
#include <DS1307RTC.h>
#include <dht11.h>


#define DHT11_PIN 3 

dht11 DHT11;


void setup() {

    Serial.begin(9600);

    // DS1307 CLK 
    setSyncProvider(RTC.get);   
    micro_delay(20);

    if(timeStatus()!= timeSet) {
        // wait a bit
        micro_delay(100);
    }

    // try again
    if(timeStatus()!= timeSet) {
        // CLK error
        char error[] = "clock error" ;
        Serial.println(error);
        for(;;);
    }

    Alarm.timerRepeat(10, log_data);

}

void log_data() {

    char buffer[17] ;
    int hh = hour();
    int mm = minute();
    int ss = second();

    // time
    sprintf(buffer,"%d:%d:%d,%d,%d",hh,mm,ss);

    int dht11_code = DHT11.read(DHT11_PIN);
    if(dht11_code != 0 ) {
        // error
        sprintf(buffer+9,"%7s","error");

    } else {
        sprintf(buffer+9,"%-3d,%-3d",DHT11.temperature, DHT11.humidity);
    }

    buffer[16] = '\0' ;
    micro_delay(200);
    Serial.println(buffer);
}

// param n is to ensure n*10 ms delay
void micro_delay(int n) {
    for(int i = 0 ; i < n ; i++) {
        delayMicroseconds(10000);
        // pat watchdog every 100*10 ms
        // if((i > 100) && (i %100 == 0 )) wdt_reset();
    }
}

void loop() {
    // alarm wont work w/o alarm delay
    Alarm.delay(1000);
}

