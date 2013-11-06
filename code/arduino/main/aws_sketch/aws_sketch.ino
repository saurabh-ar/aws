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


#include <DHT22.h>
#include "Adafruit_BMP085.h"


// Analog pin A4(SDA),A5(SCL)
Adafruit_BMP085 bmp;
DHT22 myDHT22(DHT22_PIN);
DHT22_ERROR_t dht22_error;

#define DHT22_PIN 6
#define STEEL_MELTING_POINT 1371
#define AWS_NO_LCD 1 

#if !defined(AWS_NO_LCD)
#include <LiquidCrystal.h>
LiquidCrystal lcd(7,8, 9, 10, 11, 12);
int lcd_pin = 13;    

#endif

// globals
volatile int rain_counter = 0 ;
volatile unsigned long irq_time ;
volatile unsigned long last_irq_time ;

float dht22_temp ;
float humidity ;
int32_t pressure ;

void setup() {
    bmp.begin();  
    // INT0 interrupt
    attachInterrupt(0,isr_pin2,RISING);
    last_irq_time = 0 ;
#if !defined(AWS_NO_SERIAL)
    Serial.begin(9600);
#endif

}
 

void isr_pin2() {
    
    // @imp donot use millis() for comparison
    // millis() donot advance inside an ISR
    irq_time = millis();    
    if((irq_time - last_irq_time) > 250) {
        rain_counter = rain_counter +1 ;
        last_irq_time = irq_time ;
    }
}

void serial_output() {
    Serial.print("Yuktix H ");
    if(temp1 > STEEL_MELTING_POINT) { 
        Serial.println("ERR");
        Serial.print("T ");
        Serial.print("ERR");
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

void lcd_output() {
    pinMode(lcd_pin, OUTPUT);
    digitalWrite(lcd_pin, HIGH);
    lcd.begin(16,2);       
    lcd.clear();
    // line1
    lcd.setCursor(0,0);
    lcd.print("Yuktix H ");
    if(temp1 > STEEL_MELTING_POINT) { 
        lcd.print("ERR");
        lcd.setCursor(0,1);
        lcd.print("T ");
        lcd.print("ERR");
    } else {
        lcd.print(humidity);
        lcd.setCursor(0,1);
        lcd.print("T ");
        lcd.print(dht22_temp);
    }

    lcd.print(" P ");
    lcd.print(pressure);
    lcd.print(" R ");
    lcd.print(rain_counter);
}

void loop() {

    dht22_error = myDHT22.readData();

    if(dht22_error == DHT_ERROR_NONE) {
        dht22_temp = myDHT22.getTemperatureC();
        humidity = myDHT22.getHumidity();
    } else {
        // absurd value
        dht22_temp = 9999.0 ;
    }

    pressure = bmp.readPressure();
    // bmp085_temp = bmp.readTemperature();

#if !defined(AWS_NO_SERIAL)
        serial_output();
#endif

#if !defined(AWS_NO_LCD)
    lcd_output();
#endif

    // 10s delay
    // DHT22 pins need 2 seconds
    for(int i = 0 ; i < 1000 ; i++) {
        //10 ms
        delayMicroseconds(10000);
    }
}
