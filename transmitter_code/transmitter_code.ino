#include <SPI.h> // Not actually used but needed to compile
#include <RH_ASK.h>

#define BME_SCK 8
#define BME_MISO 11
#define BME_MOSI 11 
#define BME_CS 10

RH_ASK driver;

int chk;
int hum;  //Stores humidity value
float temp; //Stores temperature value
float bat; // Stores battery value
float lvl; // Stores level value

void setup()
{
  Serial.begin(9600);   // Debugging only

  if (!driver.init()) Serial.println("init failed");

  Adafruit_BME280 bme; // I2C
}

// Store the timestamp of the last transmission so that we can see
// when the last transmission was last transmitted
unsigned int lastTransmission;

void loop()
{
  if (millis() % 65535 - lastTransmission > 100) {
    lastTransmission = millis();

    temp = random(220,230)/10.0;

    // Convert to integer and fractional parts
    double tempInt;
    float tempFrac = modf(temp, &tempInt);
    double batInt;
    float batFrac = modf(temp, &batInt);
    double lvlInt;
    float lvlFrac = modf(1 * 2, &lvlInt);

    Serial.print(lastTransmission);
    Serial.print(" ");
    Serial.println(hum);

    const int8_t buffer[8] = {
      -127, // an identifying value that will not show up in the data
      // so that we know when the transmission starts
      (int8_t) tempInt, // this assumes temperature is between -126 and 127
      (int8_t) (tempFrac * 127), // convert the fractional part to an integer -
      // the receiver will have to decode this
      (int8_t) hum, // humidity is always between 0 and 100
      (int8_t) lvlInt, // level is between 0 and 90
      (int8_t) (lvlFrac * 127),
      (int8_t) batInt, // battery is between 0 and 127
      (int8_t) (batFrac * 127)
    };

    driver.send((uint8_t *)buffer, 8);
    digitalWrite(13, HIGH);
    //driver.waitPacketSent();
    
    digitalWrite(13, LOW);
  }
}

