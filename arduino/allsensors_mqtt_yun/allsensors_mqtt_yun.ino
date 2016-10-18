/***************************************************
  Adafruit MQTT Library Arduino Yun Example

  Make sure your Arduino Yun is connected to a WiFi access point which
  has internet access.  Also note this sketch uses the Console class
  for debug output so make sure to connect to the Yun over WiFi and
  open the serial monitor to see the console output.

  Works great with the Arduino Yun:
  ----> https://www.adafruit.com/products/1498

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Tony DiCola for Adafruit Industries.
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
}

float tempCel = 0;
int lightRelativeLUX = 0;
int tempPin = 0;
int lightPin = 1;

void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();
  Console.println(F("Connected to MQTT server"));
  
  //take the readings
  tempCel = analogRead(tempPin);
  lightRelativeLUX = analogRead(lightPin); 
  
  // do some calculations
  tempCel = tempCel / 9.31;
  
  // Publish the readings
  Temperature.publish(tempCel);
  Console.println(F("Published to feed."));
  Console.print("Temp:"); //Display the temperature on Serial monitor
  Console.println(tempCel);
  //LightSensor.publish(lightRelativeLUX);
  //
  //  Console.println(F("Failed"));
  //} else {
  //  Console.println(F("OK!"));
  //}

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
       delay(5000);  // wait 5 seconds
  }
  Console.println("MQTT Re-connected!");
}
