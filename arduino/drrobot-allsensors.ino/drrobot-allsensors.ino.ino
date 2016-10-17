void setup() {
  // put your setup code here, to run once:
  {
    Serial.begin(9600);
    analogReference(INTERNAL);
  }
}

float tempCel = 0;
int lightRelativeLUX = 0;
int tempPin = 0;
int lightPin = 1;

void loop() {
  //take the readings
  tempCel = analogRead(tempPin);
  lightRelativeLUX = analogRead(lightPin);   //connect grayscale sensor to Analog 0

  // do some calculations
  tempCel = tempCel / 9.31;
  // output the metrics
  
  Serial.print("Temp:"); //Display the temperature on Serial monitor
  Serial.print(tempCel);
  Serial.print("C, ");
  Serial.print("Light:");
  Serial.print(lightRelativeLUX);
  Serial.println(",EOL");//print the value to serial   
    
  // pause 5 secs   
  delay(5000);
}
