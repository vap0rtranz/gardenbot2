/***************************************************
  Adafruit MQTT Mockup for Publishing Arduino Yun readings

  Arduino Yun must be connected to public WiFi
  uses the Console class for debug output (you can connect over Wifi)

  Written by Tony DiCola for Adafruit Industries.
  Revised by Justin Pittman for Gardenbo2 project:
  https://www.hackster.io/gardnergeeks/gardenbot2-2efcde
  
  MIT license, all text above must be included in any redistribution
 ****************************************************/
#include <Bridge.h>
#include <Console.h>
#include <BridgeClient.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "vap0rtranz"
#define AIO_KEY         "b0b947b115364b6ba2e6e8409e5aebc9"


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

void setup() {
  // Bridge setup may take a few seconds so indicate when it's bridged via PIN 13
  pinMode(13,OUTPUT);
  digitalWrite(13, LOW);
  Bridge.begin();
  digitalWrite(13, HIGH);
  // for debugging
  Console.begin();
  Console.println(F("Bridge begun."));
  digitalWrite(13, LOW);
}

float tempCel = 0;
float lightRelativeLUX = 0;
int tempPin = 0;
int lightPin = 1;

void loop() {
  // make the first MQTT connection and automatically reconnect when disconnected
  // See the MQTT_connect below
  MQTT_connect();
  Console.println(F("Connected to MQTT server"));
  
  //take the readings
  tempCel = analogRead(tempPin);
  lightRelativeLUX = analogRead(lightPin); 
  
  // do some calculations
  tempCel = tempCel / 9.31;
  
  // Publish the readings
  if (! Temperature.publish(tempCel) ) {
    Console.println(F("Publish Failed"));
    Console.print(":C:"); //Display the temperature on Serial monitor
    Console.println(tempCel);
    } else { 
      Console.println(F("OK!")); 
      digitalWrite(13, HIGH);
      delay(1000);
      digitalWrite(13, LOW);
    }
  if (! LightSensor.publish(lightRelativeLUX) ) {
    Console.println(F("Publish Failed"));
    Console.print(":LUX:"); //Display the temperature on Serial monitor
    Console.println(lightRelativeLUX);
    } else { 
      Console.println(F("OK!"));
      digitalWrite(13, HIGH);
      delay(1000);
      digitalWrite(13, LOW);
      } 

  // ping the server to keep the mqtt connection alive
  if(! mqtt.ping()) {
    Console.println(F("MQTT Ping failed!"));
  }
  
  // wait 5 seconds
  delay(5000);

}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Console.print("Reconnecting to MQTT Server ... ");

  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Console.print(mqtt.connectErrorString(ret));
       Console.print("... Retrying MQTT connection in 5 seconds...");
        mqtt.disconnect();
        digitalWrite(13, HIGH);
        delay(5000);  // wait 5 seconds
        digitalWrite(13, LOW);
  }
  Console.println("MQTT Re-connected!");
}
