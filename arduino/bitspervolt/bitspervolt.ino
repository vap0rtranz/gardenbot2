int analogPin = A0;
int delayPeriod = 1000;
float bitsPerVolt = 1023 / 5;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Arduino setup done.");
}

void loop() {
  // put your main code here, to run repeatedly:
  int pinReads = analogRead(analogPin);
  Serial.println(pinReads);
  float volts = pinReads / bitsPerVolt;
  Serial.println(volts);
  delay(delayPeriod);
}
