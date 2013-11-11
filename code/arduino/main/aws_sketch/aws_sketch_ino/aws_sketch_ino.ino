/*
 * main aws sketch
 *
 * DHT22 for T/H (digital)
 * BMP085 for T/P (analog)
 * LDR for reading light (analog)
 * LCD for display 
 *
 * Davis 7852 Rain Gauge on INT0 (pin2) 
 * GSM modem on hardware rx,tx8
 *
 *
 * 
 */

#include <avr/wdt.h>
#include <Wire.h>
#include <DHT22.h>
#include <Adafruit_BMP085.h>
#include <Time.h>
#include <TimeAlarms.h>
#include <DS1307RTC.h>
#include <AWS.h>

#define DHT22_PIN 6
//#define AWS_NO_LCD 1
//#define AWS_NO_GSM 1

// Analog pin A4(SDA),A5(SCL)
Adafruit_BMP085 bmp;
DHT22 myDHT22(DHT22_PIN);


#if !defined(AWS_NO_GSM)
#include <SoftwareSerial.h>
SoftwareSerial gsmSerial(4,5);
#endif

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
float total_rain = 0.0;

const char *month_names[12] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

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
    micro_delay(20);

    if(rtc_code == AWS_RTC_ERROR_NONE) {
        setSyncProvider(RTC.get);   
        if(timeStatus()!= timeSet) {
            // wait a bit
            micro_delay(400);
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

#if !defined(AWS_NO_GSM)
    
    gsmSerial.begin(9600);
    gsmSerial.println("AT+CMGF=1");
    micro_delay(20);
    gsmSerial.println("AT+CSMS=1");
    micro_delay(20);
    gsmSerial.println("AT+CNMI=2,2,0,0");
    micro_delay(20);
#endif

    Alarm.alarmRepeat(16,30,01, send_bulletin);

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

void update_display() {
    
    // DHT22 pin needs 2 seconds
    micro_delay(250);
    dht22_code = myDHT22.readData();
    
    if(dht22_code == DHT_ERROR_NONE) {
        dht22_temp = myDHT22.getTemperatureC();
        humidity = myDHT22.getHumidity();
        
    } else {
        // absurd value
        dht22_temp = 9999.0 ;
    }

    pressure = bmp.readPressure();
    calculate_rain();

#if !defined(AWS_NO_SERIAL)
    serial_output();
#endif

#if !defined(AWS_NO_LCD)
    lcd_output();
#endif

}

void send_bulletin() {
    Serial.println("inside send bulletin");
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

#if !defined(AWS_NO_GSM)
    gsmSerial.print("\r");  
    micro_delay(20);
    gsmSerial.print("AT+CMGF=1\r");
    micro_delay(20);
    gsmSerial.print("AT+CMGS=\"+918553518338\"\r"); 
    micro_delay(20);
    // pat watchdog
    wdt_reset();

    gsmSerial.print("Yuktix H");
    if(dht22_code !=  DHT_ERROR_NONE) { 
        gsmSerial.println("ERR");
        gsmSerial.print(dht22_code);
        
    } else {
        gsmSerial.println(humidity);
        gsmSerial.print(" T");
        gsmSerial.print(dht22_temp);
    }

    gsmSerial.print(" P");
    gsmSerial.print(pressure);
    gsmSerial.print(" R");
    gsmSerial.print(total_rain);
    gsmSerial.println();
    gsmSerial.write(0x1A);
    micro_delay(100); 
    // pat watchdog
    wdt_reset();

#endif

}

void calculate_rain() {
    // arduino float is 2 decimal places
    total_rain = rain_counter * 0.01 ;
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
    Serial.print("Yuktix H");
    if(dht22_code !=  DHT_ERROR_NONE) { 
        Serial.println("ERR");
        Serial.print(dht22_code);
        
    } else {
        Serial.println(humidity);
        Serial.print(" T");
        Serial.print(dht22_temp);
    }

    Serial.print(" P");
    Serial.print(pressure);
    Serial.print(" R");
    Serial.print(total_rain);

}
#endif

#if !defined(AWS_NO_LCD)
void lcd_output() {
  
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
    lcd.print(total_rain);
}
#endif

void loop() {

    update_display();
    wdt_reset();
    // refresh in 30 seconds
    micro_delay(2750);
    Alarm.delay(1000);
}
