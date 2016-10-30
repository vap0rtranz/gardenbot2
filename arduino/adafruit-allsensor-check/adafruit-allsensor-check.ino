#include "Arduino.h"

/***************************************************
  Adafruit MQTT Mockup for Publishing Arduino Yun readings

  Arduino Yun must be connected to public WiFi
  uses the Console class for debug output (you can connect over Wifi)

  Written by Tony DiCola for Adafruit Industries.
  Revised by Justin Pittman for Gardenbot2 project:
  https://www.hackster.io/gardnergeeks/gardenbot2-2efcde

  MIT license, all text above must be included in any redistribution
 ****************************************************/
#include <Console.h>
#include <BridgeClient.h>
#include <Process.h>
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
//Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME);

/****************************** Feeds ***************************************/

// Setup a feed called 'LightSensor' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish LightSensor = Adafruit_MQTT_Publish(&mqtt, "vap0rtranz/feeds/LightSensor");
Adafruit_MQTT_Publish Temperature = Adafruit_MQTT_Publish(&mqtt, "vap0rtranz/feeds/Temperature");

/*************************** Sketch Code ************************************/

// configurable global parameters
const int tempPin = 0;
const int lightPin = 1;
const int ledPin = 13; // the pin for Console notification
const int chipSelect = 8; // Sparkfun SD shield: pin 8
const int samplePeriod = 60000; //1min samples

// other vars
float tempCel = 0;
float lightRelativeLUX = 0;
int8_t ret;
Process dateProcess;
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
  Console.println("You're connected to my console, and Bridge begun.");
  digitalWrite(ledPin, LOW);

  // initialize SD Card incase remote publish of data fails
  if (!SD.begin(chipSelect)) { // check that SD card works
    Console.println("SD card failed, or not present");
    return;   // don't do anything more:
  }
  Console.println("SD card initialized.");

  //make MQTT Server connection/reconnection; see fx below
  ret = mqtt.connect();
  if (ret == 0) { Console.println("MQTT connected"); } else { Console.print(mqtt.connectErrorString(ret)); }
}

void loop() {

  //take the readings
  if (!dateProcess.running()) { // must block Process to wait results from across the Bridge
    dateProcess.begin("date");
    dateProcess.addParameter("+%x_%H:%M:%S");
    dateProcess.run();
  } else { Console.println("Running date process ..."); }
  
  String timestamp = dateProcess.readString();
  timestamp.trim();
  tempCel = analogRead(tempPin);
  lightRelativeLUX = analogRead(lightPin);

  // do some calculations
  tempCel = ( 5*tempCel*100/1024 ); // the LM35 gets 5V input, linear sensitivity of 1C=10mV, and 10bit sample of Atmega , or 1024 stepping

  // write the readings
  CSVDataFile = SD.open("data.csv", FILE_WRITE); // open file. only one file can be open at a time,
  CSVMetricsLine = String(timestamp + "," + String(tempCel) + "," + String(lightRelativeLUX)); // prep metrics sensor data to file by converting to strings
  if (CSVDataFile) // did the open filehandle succeed?
  {
    CSVDataFile.println(CSVMetricsLine);
    CSVDataFile.close();
    Console.println(CSVMetricsLine);
  } else { Console.println("error opening data file!"); }

  if ((! Temperature.publish(tempCel)) || (! LightSensor.publish(lightRelativeLUX) )) 
  { 
     Console.print("Publish Failed! "); 
     ret = mqtt.disconnect();
     digitalWrite(ledPin, HIGH);  // visually indicate problem with MQTT connection
     Console.println(mqtt.connectErrorString(ret));
     ret = mqtt.connect();
     if (ret == 0) { Console.println("MQTT RE-connected"); } else { Console.print(mqtt.connectErrorString(ret)); }
     digitalWrite(ledPin, LOW);  // visually indicate problem with MQTT connection
  }
  delay(samplePeriod); //the base sampling wait
}

