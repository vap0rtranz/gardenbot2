/***************************************************
  Adafruit MQTT Mockup for Publishing Arduino Yun readings

  Arduino Yun must be connected to public WiFi
  uses the Console class for debug output (you can connect over Wifi)

  Written by Tony DiCola for Adafruit Industries.
  Revised by Justin Pittman for Gardenbot2 project:
  https://www.hackster.io/gardnergeeks/gardenbot2-2efcde
  
  MIT license, all text above must be included in any redistribution
 ****************************************************/
#include <Bridge.h>
#include <Console.h>
#include <BridgeClient.h>
#include <SPI.h>
#include <SD.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "vap0rtranz"
#define AIO_KEY         "3f445b514597440ab7ed8a15096938f7"

/************ Global State (you don't need to change this!) ******************/

// Create a BridgeClient instance to communicate using the Yun's bridge & Linux OS.
BridgeClient client;
// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

// Setup a feed called 'LightSensor' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish LightSensor = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/LightSensor");
Adafruit_MQTT_Publish Temperature = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Temperature");

/*************************** Sketch Code ************************************/

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

void setup() {
  // Bridge setup may take a few seconds so indicate when it's bridged via PIN 13
  pinMode(ledPin,OUTPUT);
  digitalWrite(ledPin, HIGH);
  Bridge.begin(); // start bridge b/w Atmega/AVR + AR/Linux
  // for debugging
  Console.begin(); // start console output

  while (!Console){
    ; // wait for Console port to connect.
  }
  Console.println(F("You're connected to my console, and Bridge begun."));
  digitalWrite(ledPin, LOW);

  // initialize SD Card incase remote publish of data fails
  if (!SD.begin(chipSelect)) { // check that SD card works
    Console.println("SD card failed, or not present");
    return;   // don't do anything more:
  }
  Console.println("SD card initialized.");
}

void loop() {

  // simple Console check to see if responsive.  IDE has bug for Console over Wifi so always use SSH and/or telnet to verify
  if (Console.available() > 0) // see if there's incoming serial data:
  {
    incomingByte = Console.read(); // read the oldest byte in the serial buffer:
    if (incomingByte == 'H') { digitalWrite(ledPin, HIGH); } // if it's a capital H (ASCII 72), turn on the LED 
    if (incomingByte == 'L') { digitalWrite(ledPin, LOW); } // if it's an L (ASCII 76) turn off the LED
  }
  
  //take the readings
  tempCel = analogRead(tempPin);
  lightRelativeLUX = analogRead(lightPin); 
  
  // do some calculations
  tempCel = (5*tempCel*100/1024); // the LM35 gets 5V input, linear sensitivity of 1C=10mV, and 10bit sample of Atmega , or 1024 stepping

  // write the readings
  CSVDataFile = SD.open("data.csv", FILE_WRITE); // open file. only one file can be open at a time,
  CSVMetricsLine = String(String(tempCel) + "," + String(lightRelativeLUX)); // prep metrics sensor data to file by converting to strings
  if (CSVDataFile) // did the open filehandle succeed?
  {
    CSVDataFile.println(CSVMetricsLine);
    CSVDataFile.close();
    Console.println(CSVMetricsLine); 
  } else { Console.println("error opening data file!"); }
    
  // Publish the readings
  MQTT_connect(); // make MQTT Server connection/reconnection; see fx below
  if (! Temperature.publish(tempCel) ) { Console.println(F("Publish Failed")); } 
  if (! LightSensor.publish(lightRelativeLUX) ) { Console.println(F("Publish Failed")); } 
  
  delay(samplePeriod); //the base sampling wait
}

// Function to connect/reconnect to MQTT Server.
// call from main loop() for any pub/sub clients
void MQTT_connect() {
  
  int8_t ret;
  
  if ((ret = mqtt.connect()) != 0) // we're not connected to MQTT Server
  { 
       Console.print(mqtt.connectErrorString(ret));
       mqtt.disconnect();
       digitalWrite(ledPin, HIGH);     
  } else 
    { 
    if(! mqtt.ping())  // ping the server to keep MQTT connection alive, use only for frequent sampling rates
    { 
      Console.println(F("MQTT Ping failed!")); 
      digitalWrite(ledPin, HIGH);  
    } else { digitalWrite(ledPin, LOW); }
    }
}
