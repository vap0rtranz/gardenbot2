// MQTT client gateway w/ sensors, slim version

#include <BridgeClient.h>
#include <PubSubClient.h>

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
BridgeClient yunClient;
// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
PubSubClient mqttClient(yunClient);

/****************************** Feeds / Publishers ***************************************/
// Setup a feed called 'LightSensor' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>

// some constants
const char mqttBroker[] = "io.adafruit.com";
const int mqttPort = 1883;
const char mqttClientID[] = "vap0rtranz";
const char mqttUsername[] = "vap0rtranz";
const char mqttPassword[] = "d31936d303a14e3b8ed2dadbd7e308fe";
const int samplePeriod = 5000; //5sec samples
const byte sampleBuffer = 10;

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

  while (!yunClient.connect("www.example.org", 80)) {
    DPRINTLN("ERROR: not connected to Internet? ");
    delay(5000); // give 5 seconds to connect
  }
  
  //make MQTT Server connection/reconnection; see fx below
  mqttClient.setServer(mqttBroker, mqttPort); 
  mqttReconnect();

}

void loop() {

  publishSensorToBroker();

  delay(samplePeriod); //the base sampling wait
  
}

void mqttReconnect() {
    DPRINT("state of MQTT client is: ");
    DPRINTLN(mqttClient.state());
    DPRINT("connected return value is: ");
    DPRINTLN(mqttClient.connected());
    // Attempt to connect
    while (!mqttClient.connect(mqttClientID, mqttUsername, mqttPassword)) {
      digitalWrite(LED_BUILTIN, HIGH);  // visually indicate problem with MQTT connection
      DPRINT("ERROR: connection failed, state of client is: ");
      DPRINTLN(mqttClient.state());
      delay(5000);
    } 
    digitalWrite(LED_BUILTIN, LOW);  // visually indicate problem with MQTT connection
    DPRINT("connection attempted. state of client is: ");
    DPRINTLN(mqttClient.state());    
}

void publishSensorToBroker() {
  // publish sensor data
  if (!mqttClient.connected()) {
    DPRINTLN("Evidently we aren't connected to MQTT broker!");
    mqttReconnect();
  } else {
    DPRINTLN("publishing sensor data");
  }
  if (!mqttClient.publish("vap0rtranz/feeds/temperature", "20")) {
    DPRINTLN("ERROR: celcius temp publish");
  }
  delay(1000); //  throttle required for Adafruit broker
  if (!mqttClient.publish("vap0rtranz/feeds/lightsensor", "200")) {
    DPRINTLN("ERROR: light sensor publish");
  }
  delay(1000); // throttle required for Adafruit broker
  if (!mqttClient.publish("vap0rtranz/feeds/soilmoisture", "200")) {
    DPRINTLN("ERROR: soil moisture publish");
  }
  delay(1000); // throttle required for Adafruit broker
}

