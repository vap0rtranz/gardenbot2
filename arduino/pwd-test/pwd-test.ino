int pwmPin = 6;
int signalVolt;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(pwmPin, OUTPUT);
  Serial.println("Arudino setup done!");
  Serial.println("Enter an integer between 0 - 255");
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available()) {
    signalVolt = Serial.parseInt();
    analogWrite(pwmPin, signalVolt);
  }
}
