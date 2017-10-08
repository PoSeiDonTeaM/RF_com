#include <SPI.h> // Not actually used but needed to compile
#include <RH_ASK.h>

#include <Adafruit_HMC5883_U.h>

/**
 * Constants
 */
const float transmissions_per_second = 10;

/**
 * Pin definitions
 */
#define BME_SCK 8
#define BME_MISO 11
#define BME_MOSI 11
#define BME_CS 10

/*
   WIRELESS TRANSMITTER
*/
RH_ASK transmitter;

/**
   MAGNETIC HALL SENSOR
*/
Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);

int chk;
// Values are stored and transmitted in:
// Magnetic field: uT (microTesla)
// Temperature: Celsius
// Pressure: ???
// Battery: Volts
float mag_x; // Magnetic field X value
float mag_y; // Magnetic field Y value
float mag_z; // Magnetic field Z value
float temp = 25.2; // Temperature
float pres = 25.1; // Pressure
float bat = 5; // Battery charge level

void setup()
{
  Serial.begin(9600);   // Debugging only
  Serial.println("Booting up...");

  if (!transmitter.init()) Serial.println("RfInit failed");

  if (!mag.begin()) Serial.println("MagInit failed");
  mag.setMagGain(HMC5883_MAGGAIN_8_1);

  pinMode(13, OUTPUT);

  Serial.println("Init successful");
}

// Store the timestamp of the last transmission so that we can see
// when the last transmission was last transmitted
unsigned long lastTransmission;

/*
   Magnetic sensor data acquisition
*/
void magneticLoop()
{
  sensors_event_t event;
  mag.getEvent(&event);

  mag_x = event.magnetic.x;
  mag_y = event.magnetic.y;
  mag_z = event.magnetic.z;

  /*Serial.print("Magnetic data: ");
  Serial.print(mag_x);
  Serial.print(" ");
  Serial.print(mag_y);
  Serial.print(" ");
  Serial.print(mag_z);
  Serial.println();*/
}

/*
 * Battery voltage acquisition
 */
void batteryLoop() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif

  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH
  uint8_t high = ADCH; // unlocks both

  long result = (high<<8) | low;
  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000

  bat = ((float) result) / 1000.0;
  temp = bat;
}

/**
   Data transmission
*/
void rfLoop()
{
  // Convert to integer and fractional parts
  double magxInt;
  float magxFrac = modf(mag_x, &magxInt);
  double magyInt;
  float magyFrac = modf(mag_y, &magyInt);
  double magzInt;
  float magzFrac = modf(mag_z, &magzInt);
  double tempInt;
  float tempFrac = modf(temp, &tempInt);
  double batInt;
  float batFrac = modf(bat, &batInt);
  double presInt;
  float presFrac = modf(pres, &presInt);

  /*Serial.println(lastTransmission);
  Serial.print(" ");*/

  // Create an array with the data that will be sent wirelessly
  const int8_t buffer[13] = {
    -127, // an identifying value that will not show up in the data
    // so that we know when the transmission starts
    (int8_t) tempInt, // this assumes temperature is between -126 and 127
    (int8_t) (tempFrac * 127), // convert the fractional part to an integer -
    // the receiver will have to decode this
    (int8_t) magxInt, // assuming magnetic field is between 0 and 127
    (int8_t) (magxFrac * 127),
    (int8_t) magyInt, // assuming magnetic field is between 0 and 127
    (int8_t) (magyFrac * 127),
    (int8_t) magzInt, // assuming magnetic field is between 0 and 127
    (int8_t) (magzFrac * 127),
    (int8_t) presInt,
    (int8_t) (presFrac * 127),
    (int8_t) batInt, // battery is between 0 and 127
    (int8_t) (batFrac * 127)
  };

  transmitter.send((uint8_t *)buffer, 8);
  //driver.waitPacketSent();
}

void loop()
{
  if (millis() % 65535 - lastTransmission > 1000.0/transmissions_per_second) {
    lastTransmission = millis();
    magneticLoop();
    batteryLoop();

    rfLoop();
    digitalWrite(13, HIGH);
    delayMicroseconds(400);
    digitalWrite(13, LOW);
  }
}

