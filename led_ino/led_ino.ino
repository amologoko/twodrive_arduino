#include <Wire.h>

int ledPin = 3;                 // LED connected to digital pin 2

void setup()
{
  pinMode(ledPin, OUTPUT);      // sets the digital pin as output
  Serial.begin(9600);
  Wire.begin();
  
  Serial.println("Booting 2Drive...  Version 1");
}

int i=0;

void loop()
{
  digitalWrite(ledPin, HIGH);   // sets the LED on
  delay(1000);                  // waits for a second
  Serial.println("high");
  digitalWrite(ledPin, LOW);    // sets the LED off
  delay(1000);                  // waits for a second  
  Serial.println("low");
//  Serial.println(i);
//  i++;
}
