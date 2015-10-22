#include <Bridge.h>
#include <Temboo.h>
#include "TembooAccount.h" // Contains Temboo account information   

// The number of times to trigger the action if the condition is met
// We limit this so you won't use all of your Temboo calls while testing
int maxCalls = 10;

// The number of times this Choreo has been run so far in this sketch
int calls = 0;

int inputPin = A0;


void setup() {
  Serial.begin(9600);
  
  // For debugging, wait until the serial console is connected
  delay(4000);
  while(!Serial);
  Bridge.begin();
  
  // Initialize pins
  pinMode(inputPin, INPUT);

  Serial.println("Setup complete.\n");
}

void loop() {
  int sensorValue = analogRead(inputPin);
  Serial.println("Sensor: " + String(sensorValue));

  if (sensorValue >= 0) {
    if (calls < maxCalls) {
      Serial.println("\nTriggered! Calling GetWeatherByAddress Choreo...");
      runGetWeatherByAddress(sensorValue);
      calls++;
    } else {
      Serial.println("\nTriggered! Skipping to save Temboo calls. Adjust maxCalls as required.");
    }
  }
  delay(250);
}

void runGetWeatherByAddress(int sensorValue) {
  TembooChoreo GetWeatherByAddressChoreo;

  // Invoke the Temboo client
  GetWeatherByAddressChoreo.begin();

  // Set Temboo account credentials
  GetWeatherByAddressChoreo.setAccountName(TEMBOO_ACCOUNT);
  GetWeatherByAddressChoreo.setAppKeyName(TEMBOO_APP_KEY_NAME);
  GetWeatherByAddressChoreo.setAppKey(TEMBOO_APP_KEY);

  // Set Choreo inputs
  GetWeatherByAddressChoreo.addInput("Address", "4646 N BEACON ST. CHICAGO, IL 60640");

  // Identify the Choreo to run
  GetWeatherByAddressChoreo.setChoreo("/Library/Yahoo/Weather/GetWeatherByAddress");

  // Run the Choreo
  unsigned int returnCode = GetWeatherByAddressChoreo.run();

  // Read and print the error message
  while (GetWeatherByAddressChoreo.available()) {
    char c = GetWeatherByAddressChoreo.read();
    Serial.print(c);
  }
  Serial.println();
  GetWeatherByAddressChoreo.close();
}
