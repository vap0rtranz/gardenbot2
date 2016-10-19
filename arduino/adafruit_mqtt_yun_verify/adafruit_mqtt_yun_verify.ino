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

/************ MQTT Global State (you don't need to change this!) ******************/

// Create a BridgeClient instance to communicate using the Yun's bridge & Linux OS.
BridgeClient client;
// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/*************************** MQTT Error Reporting *********************************/

const char ERRORS[] PROGMEM = AIO_USERNAME "/errors";
Adafruit_MQTT_Subscribe errors = Adafruit_MQTT_Subscribe(&mqtt, ERRORS); 
const char THROTTLE[] PROGMEM = AIO_USERNAME "/throttle";
Adafruit_MQTT_Subscribe throttle = Adafruit_MQTT_Subscribe(&mqtt, THROTTLE);

/****************************** Feeds ***************************************/

// Setup feeds for publishing.
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
File dataFile;
String CSVMetricsLine = "";

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
  
  // initialize SD Card incase remote publish of data fails
  if (!SD.begin(chipSelect)) { // check that SD card works
    Console.println("Card failed, or not present");
    return;   // don't do anything more:
  }
  Console.println("SD card initialized.");

  // mqtt.subscribe(&throttle);
  // mqtt.subscribe(&errors);
}

void loop() {

  Console.println(F("Looping through main sketch."));
  // simple Console check to see if responsive.  IDE has bug for Console over Wifi so always use SSH and/or telnet to verify
  if (Console.available() > 0) { // see if there's incoming serial data:
    incomingByte = Console.read(); // read the oldest byte in the serial buffer:
    if (incomingByte == 'H') { digitalWrite(ledPin, HIGH); } // if it's a capital H (ASCII 72), turn on the LED 
    if (incomingByte == 'L') { digitalWrite(ledPin, LOW); } // if it's an L (ASCII 76) turn off the LED
  }

  CSVMetricsLine = String(String(tempCel) + "," + String(lightRelativeLUX)); // prep metrics sensor data to file by converting to strings
  if (dataFile) // did the open filehandle succeed?
  {
    dataFile.println(CSVMetricsLine);
    dataFile.close();
    Console.println(CSVMetricsLine); 
  } 
    else { Console.println("error opening data file!"); }

  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  // this is our 'wait for incoming subscription packets' busy subloop
  // try to spend your time here
  /***** 
   *  
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if(subscription == &errors) {
      Console.print(F("ERROR: "));
      Console.println((char *)errors.lastread);
    } else if(subscription == &throttle) {
      Console.print(F("THROTTLED: "));
      Console.println((char *)throttle.lastread);
    }
  }
  *
  */

  mqtt.ping();
  delay(samplePeriod); //the base sampling wait
}

// Function to connect/reconnect to MQTT Server.
// call from main loop() for any pub/sub clients
void MQTT_connect() {
  
  int8_t ret;
  Console.print("Connecting to MQTT Server ... ");
  if (mqtt.connected()) { return; } // check if already connected. exit if connected

  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Console.print(mqtt.connectErrorString(ret));
       Console.println("... Retrying MQTT connection ...");
       mqtt.disconnect();
       digitalWrite(ledPin, HIGH);
       delay(5000);  // wait for reconnect
       digitalWrite(ledPin, LOW);
  }
  Console.println("MQTT Re-connected!");
}
