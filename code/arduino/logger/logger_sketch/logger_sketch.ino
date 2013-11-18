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

#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal.h>

// @change pins
#define DHT11_PIN 2 

int LCD_PIN = 13;    
int LDR_PIN = A0 ;
const int chipSelect = 9;

dht11 DHT11;
File dataFile;
LiquidCrystal lcd(7,8, 3, 4, 5, 6);


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
		char line2[]= "***" ;
        Serial.println(error);
		lcd_print(error,line2);	
        for(;;);
    }

    // initialize SD card
    pinMode(SS, OUTPUT);
    if (!SD.begin(chipSelect)) {
        Serial.println("Card failed, or not present");
        // don't do anything more:
        while (1) ;
    }
    
	// LCD
	
    Alarm.timerRepeat(10, log_data);

}

void lcd_print(char* line1, char* line2) {

    pinMode(LCD_PIN, OUTPUT);
    digitalWrite(LCD_PIN, HIGH);
    lcd.begin(16,2);       
    lcd.clear();
    // line1
    lcd.setCursor(0,0);
    lcd.print(line1); 
    lcd.setCursor(0,1);
    lcd.print(line2); 
}

void log_data() {

    char ts[18] ;
	char data[12] ;
	get_sensor_data(ts,data);

    dataFile = SD.open("yuktix.log", O_CREAT | O_WRITE | O_APPEND);
    if (! dataFile) {
      Serial.println("error opening yuktix.log");
      // Wait forever since we cant write data
      while (1) ;
    }

    // write to SD card
    dataFile.print(ts);
    dataFile.print(",");
    dataFile.println(data);
	micro_delay(20);
    dataFile.close();  
    micro_delay(20);

    int hh = hour();
    int mm = minute();
	char line2[6] ;
	sprintf(line2,"%02d:%02d",hh,mm);
	
	lcd_print(line2,data);

}

void get_sensor_data(char* ts, char* data) {

    // timestamp
    int hh = hour();
    int mm = minute();
    int ss = second();
	int yyyy = year();
	int mt = month();
	int dd = day();

	// 18 char excel timestamp
    sprintf(ts,"%02d-%02d-%02d %02d:%02d:%02d",yyyy,mt,dd,hh,mm,ss);
	ts[17] = '\0' ;	

	// sensors
    int dht11_code = DHT11.read(DHT11_PIN);
    micro_delay(200);
	int ldr = analogRead(LDR_PIN); 
	
    if(dht11_code != 0 ) {
        // error
        sprintf(data,"%11s","TH_ERR");

    } else {
		// 11 char wide data
        sprintf(data,"%-03d,%-02d,%-04d",DHT11.temperature,DHT11.humidity,ldr);
    }

    data[11] = '\0' ;
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
