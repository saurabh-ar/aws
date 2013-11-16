/*
 * main aws sketch
 *
 * DHT22 for T/H (digital)
 * BMP085 for T/P (analog)
 * LDR for reading light (analog)
 * LCD for display 
 *
 * Davis 7852 Rain Gauge on INT0 (pin2) 
 * GSM modem on hardware rx,tx
 *
 *
 * 
 */

#include <stdlib.h>
#include <avr/wdt.h>
#include <Wire.h>
#include <DHT22.h>
#include <Adafruit_BMP085.h>
#include <Time.h>
#include <TimeAlarms.h>
#include <DS1307RTC.h>

// @change pins and output modes 
#define DHT22_PIN 6
#define AWS_NO_SERIAL 1 

// Analog pin A4(SDA),A5(SCL)
Adafruit_BMP085 bmp;
DHT22 myDHT22(DHT22_PIN);


#if !defined(AWS_NO_GSM)
#include <SoftwareSerial.h>
// @change gsm rx,tx
SoftwareSerial gsmSerial(4,5);
#endif

#if !defined(AWS_NO_LCD)
#include <LiquidCrystal.h>
// @change lcd pins
LiquidCrystal lcd(7,8, 9, 10, 11, 12);
int lcd_pin = 13;    

#endif

// globals
volatile int rain_counter = 0 ;
volatile bool rain_counter_reset = false ;

volatile unsigned long irq_time ;
volatile unsigned long last_irq_time ;

float dht22_temp ;
float humidity ;
int32_t pressure ;

// return codes
DHT22_ERROR_t dht22_code ;

void setup() {

#if !defined(AWS_NO_SERIAL)
    Serial.begin(9600);
#endif

    // watchdog enabled
    wdt_enable(WDTO_8S);

    // synchronize time library with DS1307 RTC 
    // DS1307RTC instance created in DS1307 lib
    // DS1307RTC RTC = DS1307RTC();
    // DS1307RTC constructor calls Wire.begin()
    // sync interval is default 5 minutes

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
#if !defined(AWS_NO_SERIAL)
        Serial.println(error);
#endif
#if !defined(AWS_NO_LCD)
       lcd_one_liner(error); 
        
#endif 
        for(;;);
    }

    bmp.begin();  
    // INT0 interrupt
    attachInterrupt(0,isr_pin2,RISING);
    last_irq_time = 0 ;


#if !defined(AWS_NO_GSM)
    gsmSerial.begin(9600);
    gsmSerial.println("AT+CMGF=1");
    micro_delay(20);
    gsmSerial.println("AT+CSMS=1");
    micro_delay(20);
    gsmSerial.println("AT+CNMI=2,2,0,0");
    micro_delay(20);
#endif

    Alarm.alarmRepeat(21,55,10, send_bulletin);

}

void isr_pin2() {
    
    // @imp millis() donot advance inside an ISR
    irq_time = millis();    
    if((irq_time - last_irq_time) > 250) {
        rain_counter = rain_counter +1 ;
        last_irq_time = irq_time ;
    }
}

// param n is to ensure n*10 ms delay
void micro_delay(int n) {
    for(int i = 0 ; i < n ; i++) {
        delayMicroseconds(10000);
        // pat watchdog every 100*10 ms
        if((i > 100) && (i %100 == 0 ))
            wdt_reset();
    }
}

void update_display(char* output) {
    
    // char output[33] ;
    output[0] = 'T' ;
    // DHT22 pin needs 2 seconds
    micro_delay(250);
    dht22_code = myDHT22.readData();
    
    if(dht22_code == DHT_ERROR_NONE) {
        dht22_temp = myDHT22.getTemperatureC();
        humidity = myDHT22.getHumidity();
        // print H and T into char buffer
        dtostrf(dht22_temp,5,1,output+1);  
        output[6] = ' ' ;
        output[7] = 'H' ;
        dtostrf(humidity,4,1,output + 8);  
        output[12] = '%' ;
        for(int i = 13 ; i < 16; i++) { output[i] = ' '; }
        
    } else {
        output[1] = ' ';
        output[2] = 'E';
        output[3] = 'R';
        output[4] = 'R';
        output[5] = 'O';
        output[6] = 'R';
        for(int i = 7 ; i < 16; i++) { output[i] = ' '; }
    }

    output[16] = 'P' ;
    pressure = bmp.readPressure();
    pressure = round(pressure/100.0) ;
    sprintf(output+17,"%-5lu",pressure);
    output[22] = ' ';
    output[23] = 'R';
    
    // more rain in a day than cherapunji 
    // receives in a year!
    sprintf(output+24,"%-5d",rain_counter);
    for(int i = 29 ; i < 32; i++){ output[i] = ' ' ; }
    // null  
    output[32] = '\0' ;
}

void send_bulletin() {

#if !defined(AWS_NO_GSM)
    char buffer[33] ;
    update_display(buffer);

    gsmSerial.print("\r");  
    micro_delay(20);
    gsmSerial.print("AT+CMGF=1\r");
    micro_delay(20);
    // @change sms number
    gsmSerial.print("AT+CMGS=\"+919886124428\"\r"); 
    micro_delay(20);
    // pat watchdog
    wdt_reset();

    gsmSerial.print(buffer);
    gsmSerial.println();
    gsmSerial.write(0x1A);

    micro_delay(100); 
    // pat watchdog
    wdt_reset();

#endif

    int hr = hour();

    // reset rain counter at 9AM
    // condition >= is for a power failure 
    // missing the 9'0 clock window
    if(!rain_counter_reset && (hr >= 9)) {
        rain_counter_reset = true ;
        rain_counter = 0 ;
    } 

    if(hr != 9 && rain_counter_reset) {
        rain_counter_reset = false ;
    }
    

}

#if !defined(AWS_NO_SERIAL)
void serial_output(char* buffer) {
    int hh = hour();
    int mm = minute();
    int ss = second();

    char ts[9] ;
    sprintf(ts,"%d:%d:%d",hh,mm,ss);
    // time
    Serial.println(ts);
    // data
    Serial.println(buffer);

}

#endif

#if !defined(AWS_NO_LCD)
void lcd_output(char* buffer) {
  
    pinMode(lcd_pin, OUTPUT);
    digitalWrite(lcd_pin, HIGH);
    lcd.begin(16,2);       
    lcd.clear();
    // line1
    lcd.setCursor(0,0);
    for(int i = 0 ; i < 15; i++){ lcd.print(buffer[i]); }
    lcd.setCursor(0,1);
    for(int i = 16 ; i < 32; i++){ lcd.print(buffer[i]); }
}

void lcd_one_liner(char* buffer) {
    pinMode(lcd_pin, OUTPUT);
    digitalWrite(lcd_pin, HIGH);
    lcd.begin(16,2);       
    lcd.clear();
    // line1
    lcd.setCursor(0,0);
    lcd.print(buffer);

}

#endif

void loop() {

    char buffer[33];
    update_display(buffer);
#if !defined(AWS_NO_SERIAL)
    serial_output(buffer);
#endif

#if !defined(AWS_NO_LCD)
    lcd_output(buffer);
#endif
    wdt_reset();
    // refresh in 30 seconds
    micro_delay(2750);
    // alarm wont work w/o alarm delay
    Alarm.delay(1000);
}

