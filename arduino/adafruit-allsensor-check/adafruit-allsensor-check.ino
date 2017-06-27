/***************************************************
  Adafruit MQTT Mockup for Publishing Arduino Yun readings

  Arduino Yun must be connected to public WiFi
  uses the Console class for debug output (you can connect over Wifi)

  Written by Tony DiCola for Adafruit Industries.
  Revised by Justin Pittman for Gardenbot2 project:
  https://www.hackster.io/gardnergeeks/gardenbot2-2efcde

  MIT license, all text above must be included in any redistribution
 ****************************************************/
#include <BridgeClient.h>
#include <SD.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "vap0rtranz"
#define AIO_KEY         "d31936d303a14e3b8ed2dadbd7e308fe"

/********** debug Scaffolding as Variadic macro called "DEBUG" ************/
#define DEBUG   //If you comment this line, the DPRINT & DPRINTLN lines are defined as blank.
#ifdef DEBUG    //Macros are usually in all capital letters.
  #define DPRINT(...)    Serial.print(__VA_ARGS__)     //DPRINT is a macro, debug print
  #define DPRINTLN(...)  Serial.println(__VA_ARGS__)   //DPRINTLN is a macro, debug print with new line
#else
  #define DPRINT(...)    //now defines a blank line
  #define DPRINTLN(...)  //now defines a blank line
#endif

/************ Global State (you don't need to change this!) ******************/
// Create a BridgeClient instance to communicate using the Yun's bridge & Linux OS.
BridgeClient client;
// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
//Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME);

/****************************** Feeds / Publishers ***************************************/
// Setup a feed called 'LightSensor' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish LightSensor = Adafruit_MQTT_Publish(&mqtt, "vap0rtranz/Feeds/LightSensor");
Adafruit_MQTT_Publish Temperature = Adafruit_MQTT_Publish(&mqtt, "vap0rtranz/Feeds/Temperature");
Adafruit_MQTT_Publish SoilMoisture = Adafruit_MQTT_Publish(&mqtt, "vap0rtranz/Feeds/SoilMoisture");

/*************************** Sketch Code ************************************/
// configurable global parameters
// tempPin = 0; lightPin = 1; soilPin = 2;
const byte chipSelect = 11; // Sparkfun SD shield: pin 8.  pin 11 is default
const int samplePeriod = 10000; //1min samples
const byte sampleBuffer = 10;

// other vars
byte pin[] = {0, 1, 2};
float sensor[3];
int8_t ret;
File CSVDataFile;

void setup() {
  // for debugging
  pinMode(LED_BUILTIN,OUTPUT);
  while (!Serial){
    digitalWrite(LED_BUILTIN, HIGH); // if L/13 light is lit, then waiting for Serial
  }
  DPRINTLN("You're connected to my console");
  
  // Bridge setup may take a few seconds so indicate when it's bridged via PIN 13
  Bridge.begin(); // start bridge b/w Atmega/AVR + AR/Linux, this appears to be blocking
  DPRINTLN("Bridge finished.");

  // initialize SD Card incase remote publish of data fails
  if (!SD.begin(chipSelect)) { // check that SD card works
    DPRINTLN("SD card failed, or not present");
    digitalWrite(LED_BUILTIN, HIGH); 
    //return;   // don't do anything more?
  } else {
    DPRINTLN("SD card initialized.");
    digitalWrite(LED_BUILTIN, LOW);
  }

  //make MQTT Server connection/reconnection; see fx below
  ret = mqtt.connect();
  if (ret == 0) { 
    DPRINTLN("MQTT connected"); 
    digitalWrite(LED_BUILTIN, LOW);
   } else { 
    DPRINT(mqtt.connectErrorString(ret)); 
    digitalWrite(LED_BUILTIN, HIGH); 
    return;   // don't do anything more?
   }
}

void loop() {

  getSensorReadings();
  
  writeSensorToFile();

  //publishSensorToBroker();

  delay(samplePeriod); //the base sampling wait
  for (int j = 0; j < 3; j++) {
      sensor[j] = 0;
  }
}

void getSensorReadings() {

  //take the readings
  // # of samples to take
  for (int i = 0; i < sampleBuffer; i++) {
    // run through sensors in sequence
    for (int j = 0; j < 3; j++) {
      sensor[j] = sensor[j] + analogRead(pin[j]);
    }
  }
  for (int j = 0; j < 3; j++) {
      sensor[j] = sensor[j] / sampleBuffer;
  }
  // do some sensor data calculations to convert to meaningful units
  // for temperature: the LM35 sensor gets 5V input, linear sensitivity of 1C=10mV, and 10bit sample of Atmega , or 1024 stepping
  sensor[0] = ( 5*sensor[0]*100/1024 ); 
  // 0-5V for reading range of 0 - 10,000, so "relative" luminescence
  // the sensor value description: 0  ~300 dry soil, 300~700 humid soil, 700~950 in water
}

void writeSensorToFile() {
// write the readings
  CSVDataFile = SD.open("data.csv", FILE_WRITE); // open file. only one file can be open at a time,
  if (CSVDataFile) // did the open filehandle succeed?
  {
     for (int j = 0; j < 3; j++) {
      CSVDataFile.print(sensor[j]);
      CSVDataFile.print(",");
      DPRINT(sensor[j]);
      DPRINT(",");
    }
    CSVDataFile.println();
    CSVDataFile.close();
    DPRINTLN(";");
  } else { DPRINTLN("error opening data file!"); }
}

void publishSensorToBroker() {
  // publish sensor data
  //if ((! Temperature.publish(sensor[0])) || (! LightSensor.publish(sensor[1])) || (! SoilMoisture.publish(sensor[2]))) 
  { 
     DPRINTLN("Publish Failed! "); 
     ret = mqtt.disconnect();
     digitalWrite(LED_BUILTIN, HIGH);  // visually indicate problem with MQTT connection
     DPRINT(mqtt.connectErrorString(ret));
     ret = mqtt.connect();
     if (ret == 0) { DPRINTLN("MQTT RE-connected"); } else { DPRINT(mqtt.connectErrorString(ret)); }
     digitalWrite(LED_BUILTIN, LOW);  // visually indicate problem with MQTT connection
  }
}

