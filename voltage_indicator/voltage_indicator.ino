// Print battery voltage
// to 16x2 LCD via I2C
// with Voltage Divider
/*
  Resistors are aligned in series.
  One end goes to Battery - and also to Arduino GND
  The other goes to Battery + and also to Arduino Vin
  The middle (connection between two resistors) goes to Arduino A0
*/
// VOLTAGE DIVIDER:
// 1x 2.2k
// 2x 2.2k
//
// + 2.2k A0/A5 2.2k 2.2k -

const float maxvolts = 12;
const float minvolts = 4;

const float lambda = 100/(maxvolts - minvolts);
const float beta   = - minvolts * lambda;

void setup()
{
  Serial.begin(9600);
}

void loop()
{
  printVolts();
  //Serial.println(analogRead(A5));
  delay(400);
}
 
 void printVolts()
{
  int sensorValue = analogRead(A5); //read the A0 pin value
  float voltage = sensorValue * (5.00 / 1023.00) * 3.00; //convert the value to a true voltage.
  Serial.print("voltage:");
  Serial.println(voltage);
  Serial.print("Voltage percentance:");
  float percentance = lambda * voltage + beta;
  Serial.print(percentance);
  Serial.println("%");  
  
}
