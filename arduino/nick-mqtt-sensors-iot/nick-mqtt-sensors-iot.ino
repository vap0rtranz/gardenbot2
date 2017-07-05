
#include <YunClient.h>
#include <SD.h>
#include <PubSubClient.h>
// DHT Written by ladyada, public domain
#include "DHT.h"

/********** debug Scaffolding as Variadic macro called "DEBUG" ************/
#define DEBUG   //If you comment this line, the DPRINT & DPRINTLN lines are defined as blank.
#ifdef DEBUG    //Macros are usually in all capital letters.
  #define DPRINT(...)    Serial.print(__VA_ARGS__)     //DPRINT is a macro, debug print
  #define DPRINTLN(...)  Serial.println(__VA_ARGS__)   //DPRINTLN is a macro, debug print with new line
#else
  #define DPRINT(...)    //now defines a blank line
  #define DPRINTLN(...)  //now defines a blank line
#endif

/************************* Adafruit.io Setup *********************************/
//#define AIO_KEY         "d31936d303a14e3b8ed2dadbd7e308fe"

// configurable global parameters
// Create a YunClient instance to communicate using the Yun's bridge & Linux OS.
YunClient yunClient;
// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
PubSubClient mqttClient("iot.eclipse.org", 1883, yunClient);

/****************************** Feeds / Publishers ***************************************/
// Setup a feed called 'LightSensor' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>

// some constants
const byte chipSelect = 11; // Sparkfun SD shield: pin 8.  pin 11 is default
const byte dhtPin = 8; // digital pin 8
const char* mqttBroker = "io.adafruit.com";
const int mqttPort = 1883;
const char* mqttClientID = "vap0rtranz";
const char* mqttUsername = "vap0rtranz";
const char* mqttPassword = "d31936d303a14e3b8ed2dadbd7e308fe";
const int samplePeriod = 10000; //1min samples
const byte sampleBuffer = 10;

// Pins and such
// the analog pins are, respectively: temperature, light, moisture
// tempPin = 0; lightPin = 1; soilPin = 2;
const byte pin[] = {0, 1, 2};
// DHT sensor setup, specifics which IC: 11 or 22
#define DHTTYPE DHT11  
DHT dht(dhtPin, DHTTYPE);
int sensor[3];
int humidity; 
int fahrenheit; 
int heatIndex;
int8_t ret;
File CSVDataFile;
String pubString;
char* message_buff;

/*************************** Sketch Code ************************************/
void setup() {
  // for debugging
  pinMode(LED_BUILTIN,OUTPUT);
  // this will wait until Serial is connected, so only do for debugging
#ifdef DEBUG 
  while (!Serial){
    digitalWrite(LED_BUILTIN, HIGH); // if L/13 light is lit, then waiting for Serial
  }
  DPRINTLN("You're connected to my console");
#endif
  
  // Bridge setup may take a few seconds so indicate when it's bridged via PIN 13
  digitalWrite(LED_BUILTIN, HIGH); // if L/13 light is lit, then waiting for Serial
  Bridge.begin(); // start bridge b/w Atmega/AVR + AR/Linux, this appears to be blocking
  digitalWrite(LED_BUILTIN, LOW); // if L/13 light is lit, then waiting for Serial
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

  if (!yunClient.connect("example.org", 80)) {
    DPRINTLN("WARN: not connected to Internet?");
  } else {
    DPRINTLN("connected to Internet.");
  }
  //make MQTT Server connection/reconnection; see fx below
  //mqttClient.setCallback(callback);
  mqttReconnect();
   
  // Initialize DHT sensor.
  dht.begin();
}

void loop() {

  getAnalogSensorReadings();
  getDHTSensorReadings();
  writeSensorToFile();
  publishSensorToBroker();

  delay(samplePeriod); //the base sampling wait
  for (int j = 0; j < 3; j++) {
      sensor[j] = 0;
  }
}

void mqttReconnect() {
    DPRINT("state of MQTT client is: ");
    DPRINTLN(mqttClient.state());
    if (! mqttClient.state() == 0) {
        DPRINTLN("WARN: not connected to MQTT broker.");
    }
    DPRINT("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect("test")) {
    //if (mqttClient.connect("vap0rtranz", "vap0rtranz", "d31936d303a14e3b8ed2dadbd7e308fe")) {
      digitalWrite(LED_BUILTIN, LOW);  // visually indicate problem with MQTT connection
      DPRINT("supposedly we're connected? state of client is: ");
      DPRINTLN(mqttClient.state());
    } else {
      digitalWrite(LED_BUILTIN, HIGH);  // visually indicate problem with MQTT connection
      DPRINT("ERROR: connection failed, state of client is: ");
      DPRINTLN(mqttClient.state());
    }
}

void getAnalogSensorReadings() {

  //take the analog readings
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

void getDHTSensorReadings() {
    // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  humidity = dht.readHumidity();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  fahrenheit = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(humidity) || isnan(fahrenheit)) {
    DPRINTLN("Failed to read from DHT sensor!");
  } else {
    // Compute heat index in Fahrenheit (the default)
    heatIndex = dht.computeHeatIndex(fahrenheit, humidity);
  }
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
    CSVDataFile.print(humidity);
    CSVDataFile.print(",");
    CSVDataFile.print(fahrenheit);
    CSVDataFile.print(",");
    CSVDataFile.print(heatIndex);
    CSVDataFile.print(",");
    DPRINT(humidity);
    DPRINT(",");
    DPRINT(fahrenheit);
    DPRINT(",");
    DPRINT(heatIndex);
    DPRINT(",");
    CSVDataFile.println();
    DPRINTLN(";");
    CSVDataFile.close();
  } else { DPRINTLN("error opening data file!"); }
}

void publishSensorToBroker() {
  // publish sensor data
  if (mqttClient.state() != 0) {
    mqttReconnect();
  }
  pubString = String(sensor[0]);
  pubString.toCharArray(message_buff, pubString.length()+1);
  mqttClient.publish("vap0rtranz/feeds/Temperature", message_buff);
  delay(1000);
  pubString = String(sensor[1]);
  pubString.toCharArray(message_buff, pubString.length()+1);
  mqttClient.publish("vap0rtranz/feeds/LightSensor", message_buff);
  delay(1000);
  pubString = String(sensor[2]);
  pubString.toCharArray(message_buff, pubString.length()+1);
  mqttClient.publish("vap0rtranz/feeds/SoilMoisture", message_buff);
  delay(1000);
  pubString = String(fahrenheit);
  pubString.toCharArray(message_buff, pubString.length()+1);
  mqttClient.publish("vap0rtranz/feeds/Fahrenheit", message_buff);
  delay(1000);
  pubString = String(humidity);
  pubString.toCharArray(message_buff, pubString.length()+1);
  mqttClient.publish("vap0rtranz/feeds/Humidity", message_buff);
  /*
  if (ret > 0) 
  { 
     DPRINTLN("A Publish Failed! "); 
     ret = mqtt.disconnect();
     digitalWrite(LED_BUILTIN, HIGH);  // visually indicate problem with MQTT connection
     DPRINT(mqtt.connectErrorString(ret));
     ret = mqtt.connect();
     if (ret == 0) { DPRINTLN("MQTT RE-connected"); } else { DPRINT(mqtt.connectErrorString(ret)); }
     digitalWrite(LED_BUILTIN, LOW);  // visually indicate problem with MQTT connection
  }
  */
}

// callback the payload from MQTT connection
void callback(char* topic, byte* payload, unsigned int length) {
  DPRINT("Message arrived [");
  DPRINT(topic);
  DPRINT("] ");
  for (int i=0;i<length;i++) {
    DPRINT((char)payload[i]);
  }
  DPRINTLN();
}

