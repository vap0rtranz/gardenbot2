/*
 * 
 Basic MQTT example

 This sketch demonstrates the basic capabilities of the library.
 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic"
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary

 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.
 
*/

#include <BridgeClient.h>
#include <PubSubClient.h>

BridgeClient yunClient;
PubSubClient mqttClient(yunClient);
int retVal;

/*
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}
*/

void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect("vap0rtranz", "vap0rtranz", "d31936d303a14e3b8ed2dadbd7e308fe")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, error code is: ");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 15 seconds");
      // Wait 15 seconds before retrying
      delay(15000);
    }
  }
}

void setup()
{
  Serial.begin(9600);
  // bridge is blocking so no delay needed
  Bridge.begin();

  mqttClient.setServer("io.adafruit.com", 1883);
  //mqttClient.setCallback(callback);
  
}

void loop()
{
   // Allow the hardware to sort itself out by checking that we can connect as client
  
  if ((!mqttClient.connected()) || (!yunClient.connect("example.org", 80))) {
    Serial.println("not connected.");
    reconnect();
  }
  //mqttClient.loop();
  // Once connected, publish an announcement...
  if (!mqttClient.publish("vap0rtranz/feeds/Fahrenheit","87")) {
   Serial.println("not published"); 
  } else {
    Serial.println("temp published");
  }
  // ... and resubscribe
  if (!mqttClient.subscribe("vap0rtranz/throttle")) {
    Serial.println("not subscribed");
  } else {
    Serial.println("throttle subscribed");
  }
  delay(5000);
}
