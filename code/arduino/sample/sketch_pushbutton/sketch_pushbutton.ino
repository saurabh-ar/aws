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

volatile unsigned long irq_time ;
volatile unsigned long last_irq_time ;

void setup()
{
    pinMode(ledPin, OUTPUT);
    // watch pin2
    attachInterrupt(0,isr_pin2,RISING);
    last_irq_time = 0 ;
    Serial.begin(9600);

}

void loop()
{
    digitalWrite(ledPin, state);
    delay(5000);
    Serial.print("counter=");
    Serial.println(counter);

}

void isr_pin2()
{
    
    // @imp donot use millis() for comparison
    // millis() donot advance inside an ISR
    irq_time = millis();    
    if((irq_time - last_irq_time) > 250) {
        state = !state ;
        counter = counter +1 ;
        last_irq_time = irq_time ;
    }
}
