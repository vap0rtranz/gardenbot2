/*****************************************************  
  Revised by Justin Pittman for Yun based Gardenbot2 project:
  https://www.hackster.io/gardnergeeks/gardenbot2-2efcde
  
  Apache-2 Licensed, all text above must be included in any redistribution
 ****************************************************/

// libraries
#include <Bridge.h>
#include <Console.h>
#include <BridgeClient.h>

// configurable global parameters
const int tempPin = 0;
const int lightPin = 1;
const int ledPin = 13; // the pin for Console notification
int samplePeriod = 5000; //1min samples
float tempCel = 0;
float lightRelativeLUX = 0;
int incomingByte;

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
  Console.println(F("You're connected to my console, and Bridge begun."));
  digitalWrite(ledPin, LOW);
}

void loop() {
  
  // simple Console check to see if responsive.  IDE has bug for Console over Wifi so always use SSH and/or telnet to verify
  if (Console.available() > 0) { // see if there's incoming Console data:
    incomingByte = Console.read(); // read the oldest byte in the serial buffer:
    if (incomingByte == 'H') { digitalWrite(ledPin, HIGH); } // if it's a capital H (ASCII 72), turn on the LED 
    if (incomingByte == 'L') { digitalWrite(ledPin, LOW); } // if it's an L (ASCII 76) turn off the LED
  }
  
  //take the readings
  tempCel = analogRead(tempPin);
  lightRelativeLUX = analogRead(lightPin);   

  // do some calculations
  tempCel = (5*tempCel*100/1024); // the LM35 gets 5V input, linear sensitivity of 1C=10mV, and 10bit sample of Atmega , or 1024 stepping
  
  // output the metrics
  Console.print("Temp:"); //Display the temperature on Console monitor
  Console.print(tempCel);
  Console.print("C, ");
  Console.print("Light:");
  Console.print(lightRelativeLUX);
  Console.println(",EOL");//print the value to serial   
    
  delay(samplePeriod); //the base sampling wait
}
