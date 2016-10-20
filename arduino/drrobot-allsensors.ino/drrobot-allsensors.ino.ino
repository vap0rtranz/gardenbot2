/*****************************************************  
  Revised by Justin Pittman for Yun based Gardenbot2 project:
  https://www.hackster.io/gardnergeeks/gardenbot2-2efcde
  
  Apache-2 Licensed, all text above must be included in any redistribution
 ****************************************************/

// libraries
#include <Bridge.h>
#include <Console.h>
#include <BridgeClient.h>
#include <SPI.h>
#include <SD.h>

// configurable global parameters
const int tempPin = 0;
const int lightPin = 1;
const int ledPin = 13; // the pin for Console notification
const int chipSelect = 8; // Sparkfun SD shield: pin 8
int samplePeriod = 5000; //1min samples
float tempCel = 0;
float lightRelativeLUX = 0;
int incomingByte;
File CSVDataFile;
String CSVMetricsLine = "";

BridgeClient client; //  to communicate using the Yun's bridge & Linux OS.

void setup() {
  // for debugging: Bridge setup may take a few seconds so indicate when it's bridged via PIN 13
  pinMode(ledPin,OUTPUT);
  digitalWrite(ledPin, HIGH);
  Bridge.begin(); // start bridge b/w Atmega/AVR + AR/Linux
  Console.begin(); // start console output after bridged so we can get remote/Wifi Console
  while (!Console){ // enable this if you don't want to start main loop() until your Console connects
    ; // wait for Console port to connect.
  }
  delay(1000); // give your remote console a second to setup
  Console.println(F("You're connected to my console, and Bridge begun."));
  digitalWrite(ledPin, LOW);
  
  // initialize SD Card incase remote publish of data fails
  if (!SD.begin(chipSelect)) { // check that SD card works
    Console.println("Card failed, or not present");
    return;   // don't do anything more:
  }
  Console.println("SD card initialized.");
}

void loop() {
  
  // simple Console check to see if responsive.  IDE has bug for Console over Wifi so always use SSH and/or telnet to verify
  if (Console.available() > 0) // see if there's incoming Console data:
  { 
    incomingByte = Console.read(); // read the oldest byte in the serial buffer:
    if (incomingByte == 'H') { digitalWrite(ledPin, HIGH); } // if it's a capital H (ASCII 72), turn on the LED 
    if (incomingByte == 'L') { digitalWrite(ledPin, LOW); } // if it's an L (ASCII 76) turn off the LED
  }

  CSVDataFile = SD.open("data.csv", FILE_WRITE); // open file. only one file can be open at a time,
  
  //take the readings
  tempCel = analogRead(tempPin);
  lightRelativeLUX = analogRead(lightPin);   

  // do some calculations
  tempCel = (5*tempCel*100/1024); // the LM35 gets 5V input, linear sensitivity of 1C=10mV, and 10bit sample of Atmega , or 1024 stepping
  
  CSVMetricsLine = String(String(tempCel) + "," + String(lightRelativeLUX)); // prep metrics sensor data to file by converting to strings
  if (CSVDataFile) // did the open filehandle succeed?
  {
    CSVDataFile.println(CSVMetricsLine);
    CSVDataFile.close();
    Console.println(CSVMetricsLine); 
  } 
    else { Console.println("error opening data file!"); }
    
  delay(samplePeriod); //the base sampling wait
}
