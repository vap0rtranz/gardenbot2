void setup() {
  // put your setup code here, to run once:
  {
    Serial.begin(9600);
    analogReference(INTERNAL);
  }
}

float tempC = 0;
int reading = 0;
int tempPin = 0;

void loop() {
  // put your main code here, to run repeatedly:
  reading = analogRead(tempPin);
  tempC = reading / 9.31;
  Serial.print("Temp:"); //Display the temperature on Serial monitor
  Serial.print(tempC);
  Serial.println("C");
  delay(5000);
}
