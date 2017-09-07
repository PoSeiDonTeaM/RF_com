// Print battery voltage
// to 16x2 LCD via I2C
// with Voltage Divider (2x 10K resistor)
/*
  Resistors are aligned in series.
  One end goes to Battery - and also to Arduino GND
  The other goes to Battery + and also to Arduino Vin
  The middle (connection between two resistors) goes to Arduino A0
*/

#include <Wire.h>

void setup()
{
  Serial.begin(9600);
}

void loop()
{
  printVolts();
}
 
 void printVolts()
{
  int sensorValue = analogRead(A0); //read the A0 pin value
  float voltage = sensorValue * (5.00 / 1023.00) * 2; //convert the value to a true voltage.
  Serial.print("voltage:");
  Serial.println(voltage);
  Serial.print("Voltage percentance:");
  float percentance = (voltage/6.20)*100;
  Serial.print(percentance);
  Serial.println("%");
  delay(1000);
  
  
}
