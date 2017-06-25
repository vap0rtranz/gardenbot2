#include <prescaler.h>
#include <LowPower.h>

int ledPin = LED_BUILTIN;
int delayPeriod = 8000;

void setup() {
  // put your setup code here, to run once:
  pinMode(ledPin, OUTPUT);
  setClockPrescaler(CLOCK_PRESCALER_16);
  Serial.begin(9600);
  Serial.println("Arduino setup done.");
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print("Entering low power ...");
  digitalWrite(ledPin, LOW);
  LowPower.idle(SLEEP_8S, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, 
SPI_OFF, USART0_ON, TWI_OFF);
  delay(delayPeriod);
  Serial.println("... woken up");
  digitalWrite(ledPin, HIGH);
}
