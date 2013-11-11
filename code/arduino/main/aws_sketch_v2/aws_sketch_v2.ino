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
#include <TimeAlarms.h>
#include <DS1307RTC.h>
#include <AWS.h>

#define DHT22_PIN 6
#define AWS_NO_SERIAL 1 
#define DS1307_CTRL_ID 0x68 

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

    rtc_code = init_rtc();
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

    Alarm.alarmRepeat(8,30,0, send_bulletin);

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


uint8_t dec2bcd(uint8_t num) {
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
    // alarm wont work w/o alarm delay
    Alarm.delay(1000);
}
