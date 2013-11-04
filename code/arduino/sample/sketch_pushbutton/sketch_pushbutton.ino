/*
 * pushbutton sample with a debouncer circuit
 * and interrupts to write *click* to serial when 
 * pushbutton is pressed.
 *
 * Arduino UNO has two external interrupts
 * int.0 on pin2 and int.1 on pin3
 *
 *
 * 
 */

int ledPin = 13;
volatile int state = LOW;
volatile int counter = 0 ;

void setup()
{
    pinMode(ledPin, OUTPUT);
    // watch pin2
    attachInterrupt(0,isr_pin2,RISING);
}

void loop()
{
    digitalWrite(ledPin, state);
    delay(1000);

}

void isr_pin2()
{
    state = !state ;
    counter++ ;
    Serial.print("pin2 interrupted: ");
    Serial.print("counter: ");
    Serial.println(counter);
}
