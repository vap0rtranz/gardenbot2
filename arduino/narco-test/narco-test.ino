#include <Narcoleptic.h>

int ledPin = LED_BUILTIN;
int seconds = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  Serial.println("Arduino setup done.");
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:
  seconds = (millis() + Narcoleptic.millis()) / 1000;
  Serial.println(seconds);
  Serial.print("going narco for 5 seconds ...");
  digitalWrite(LED_BUILTIN, LOW);
  Narcoleptic.delay(5000);
  seconds = (millis() + Narcoleptic.millis()) / 1000;
  Serial.println(" ... woken up. Was it 5 seconds?");
  Serial.println(seconds);
  digitalWrite(LED_BUILTIN, HIGH);
}
