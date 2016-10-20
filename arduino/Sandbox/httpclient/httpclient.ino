#include <Bridge.h>
#include <HttpClient.h>

void setup() {
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  Bridge.begin();
  Serial.begin(9600);
  while(!Serial);
}

void loop() {
  // create & get an httpclient
  HttpClient client;
  client.get("http://www.arduino.cc/en/Main/CopyrightNot");

  // while client is ready, read from it, then print it's input
  while (client.available()) {
    char c = client.read();
    Serial.print(c);
  }
  Serial.flush();

  // do this every 5sec
  delay(5000);
}
