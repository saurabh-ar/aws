/*
  SD card datalogger
 
 This example shows how to log data from three analog sensors 
 to an SD card using the SD library.
 	
 The circuit:
 * analog sensors on analog ins 0, 1, and 2
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4
 
 created  24 Nov 2010
 modified 9 Apr 2012
 by Tom Igoe
 
 This example code is in the public domain.
 	 
 */

#include <SD.h>

// @change-1
const int chipSelect = 4;

void setup() {
  Serial.begin(9600);
  Serial.print("Initializing SD card...");
  // @change-2
  pinMode(10, OUTPUT);
  
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }

  Serial.println("card initialized.");
}

void loop() {


    char buffer[] = "SD library rocks!";  
    File dataFile = SD.open("tpl.dat", O_APPEND);
    
    // if the file is available, write to it:
    if (dataFile) {
        dataFile.println(buffer);
        dataFile.close();
        // print to the serial port too:
        Serial.println(buffer);
    }  else {
        Serial.println("error opening datalog.txt");
    } 

    delay(3000);
}









