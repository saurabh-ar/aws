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

#include <avr/wdt.h>
#include <Wire.h>
#include <DHT22.h>
#include <Adafruit_BMP085.h>
#include <Time.h>
#include <DS1307RTC.h>
#include <AWS.h>



#define DHT22_PIN 6
#define AWS_NO_SERIAL 1 

// Analog pin A4(SDA),A5(SCL)
Adafruit_BMP085 bmp;
DHT22 myDHT22(DHT22_PIN);
DHT22_ERROR_t dht22_code;

#if !defined(AWS_NO_LCD)
#include <LiquidCrystal.h>
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

const char *month_names[12] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

char bulletin[33] ;

tmElements_t tm;

// return codes
DHT22_ERROR_t dht22_code ;
aws_error_t rtc_code ;

void setup() {

    // watchdog enabled
    wdt_enable(WDTO_8S);

    // DS1307 RTC 
    // DS1307RTC instance created in DS1307 lib
    // DS1307RTC RTC = DS1307RTC();
    // DS1307RTC constructor calls Wire.begin()
    // sync interval is default 5 minutes

    rtc_code = init_rtc(__TIME__,__DATE__);
    for(int i = 0 ; i < 200 ; i++) {
      delayMicroseconds(1000);
    }

    if(rtc_code == AWS_RTC_ERROR_NONE) {
        setSyncProvider(RTC.get);   
        if(timeStatus()!= timeSet) {
            // wait a bit
            for(int i = 0 ; i < 4000 ; i++) {
                delayMicroseconds(1000);
            }
        }

        // try again
        if(timeStatus()!= timeSet) {
          rtc_code = AWS_RTC_SYNC_ERROR ;
        }
    }  

    bmp.begin();  
    // INT0 interrupt
    attachInterrupt(0,isr_pin2,RISING);
    last_irq_time = 0 ;
#if !defined(AWS_NO_SERIAL)
    Serial.begin(9600);
#endif

}
 

void isr_pin2() {
    
    // @imp millis() donot advance inside an ISR
    irq_time = millis();    
    if((irq_time - last_irq_time) > 250) {
        rain_counter = rain_counter +1 ;
        last_irq_time = irq_time ;
    }
}

float calculate_rain() {

    // arduino float is 2 decimal places
    float total = rain_counter * 0.01 ;
    int hour = hour();

    // reset rain counter at 9AM
    // condition >= is for a power failure 
    // missing the 9'0 clock window
    if(!rain_counter_reset && (hour >= 9)) {
        rain_counter_reset = true ;
        rain_counter = 0 ;
        // @todo send rain bulletin
    } 

    if(hour != 9 && rain_counter_reset) {
        rain_counter_reset = false ;
    }
}

void create_bulletin() {


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

#if !defined(AWS_NO_SERIAL)
void serial_output() {
    Serial.print("Yuktix H ");
    if(dht22_code !=  DHT_ERROR_NONE) { 
        Serial.println("ERR ");
        Serial.print(dht22_code);
        
    } else {
        Serial.println(humidity);
        Serial.print("T ");
        Serial.print(dht22_temp);
    }

    Serial.print(" P ");
    Serial.print(pressure);
    Serial.print(" R ");
    Serial.print(rain_counter);

}
#endif

#if !defined(AWS_NO_LCD)
void lcd_output(DHT22_ERROR_t dht22_code) {
  
    int t1 ;
    int p1 ;
    int h1 ;
    
    pinMode(lcd_pin, OUTPUT);
    digitalWrite(lcd_pin, HIGH);
    lcd.begin(16,2);       
    lcd.clear();
    // line1
    lcd.setCursor(0,0);
    lcd.print("YUKTIX H");
    if(dht22_code !=  DHT_ERROR_NONE) { 
        lcd.print("ERR");
        lcd.setCursor(0,1);
        lcd.print(dht22_code);
        
    } else {
        h1 = round(humidity);
        lcd.print(h1);
        lcd.print("%");
        lcd.setCursor(0,1);
        lcd.print("T");
        t1 = round(dht22_temp);
        lcd.print(t1);
    }

    lcd.print(" P");
    p1 = round(pressure/10.0);
    lcd.print(p1);
    lcd.print(" R");
    lcd.print(rain_counter);
}
#endif

void loop() {

     // DHT22 pin needs 2 seconds
    for(int i = 0 ; i < 250 ; i++) {
        delayMicroseconds(10000);
        if(i % 100 == 0 ) { 
            wdt_reset(); 
        }
    }
    
    dht22_code = myDHT22.readData();
    
    if(dht22_code == DHT_ERROR_NONE) {
        dht22_temp = myDHT22.getTemperatureC();
        humidity = myDHT22.getHumidity();
        
    } else {
        // absurd value
        dht22_temp = 9999.0 ;
    }

    pressure = bmp.readPressure();

#if !defined(AWS_NO_SERIAL)
    serial_output();
#endif

#if !defined(AWS_NO_LCD)
    lcd_output();
#endif

    wdt_reset();

    // total delay 30 seconds
    for(int i = 0 ; i < 2750 ; i++) {
        delayMicroseconds(10000);
        // pat watchdog every second
        if(i % 100 == 0 ) { 
            wdt_reset(); 
        }
    }
}
