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

#include <SPI.h>
#include <SD.h>
// Include remote libs
#include <Console.h>
#include <Bridge.h>
#include <BridgeClient.h>

// Sparkfun SD shield: pin 8
const int chipSelect = 8;

void setup() {
   Bridge.begin(); // start bridge b/w Atmega/AVR + AR/Linux
  // Open serial communications and wait for port to open:
  Console.begin();
  while (!Console) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Console.println("\nYou're connected.  Bridge should be up...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Console.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Console.println("SD card initialized.");
}

void loop() {
  // make a string for assembling the data to log:
  String dataString = "";

  // read three sensors and append to the string:
  for (int analogPin = 0; analogPin < 2; analogPin++) {
    int sensor = analogRead(analogPin);
    dataString += String(sensor);
    if (analogPin < 1) {
      dataString += ",";
    }
  }

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    Console.println(dataString);
  }
  // if the file isn't open, pop up an error:
  else {
    Console.println("error opening datalog.txt");
  }
}





