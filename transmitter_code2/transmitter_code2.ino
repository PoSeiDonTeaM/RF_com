#include <SPI.h> // Not actually used but needed to compile
#include <IRremote.h>
#include <Stepper.h>
#include <DHT.h>
#include <RH_ASK.h>
// TODO: fix breaking DHT

/*----- Variables, Pins -----*/
#define STEPS  32   // Number of steps per revolution of Internal shaft
int receiver = 7; // Signal Pin of IR receiver to Arduino Digital Pin
int stepsToTake = 16; // 2048 = 1 Revolution -- Number of steps to take every message
const float maxvolts = 12;
const float minvolts = 4;

/*-----( Declare objects )-----*/
// Setup of proper sequencing for Motor Driver Pins
// In1, In2, In3, In4 in the sequence 1-3-2-4
// 5.62 degrees / 64 steps

const float degsPerStep = 5.625 / 64.0;
const float lambda = 100 / (maxvolts - minvolts);
const float beta   = - minvolts * lambda;

// Forward order: 8 10 9 11
Stepper small_stepper(STEPS, 11, 9, 10, 8);
IRrecv irrecv(receiver);    // create instance of 'irrecv'
decode_results results;

RH_ASK driver;

#define DHTPIN 2     // what pin we're connected to
#define DHTTYPE DHT11   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino


long currentPosition = 0;

// DHT sensor has a 0.5 Hz sampling rate,
// but we are transmitting data faster to
// account for possible packet loss

// DHT22 typical ranges:
// -40 ~ 125 C temperature
//   0 ~ 100 % humidity

int chk;
int hum;  //Stores humidity value
float temp; //Stores temperature value
float bat; // Stores battery value
float lvl; // Stores level value

void setup()
{
  Serial.begin(9600);   // Debugging only

  dht.begin();
  if (!driver.init()) Serial.println("init failed");

  irrecv.enableIRIn(); // Start the receiver
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10,OUTPUT);
  pinMode(11,OUTPUT);
  pinMode(13, OUTPUT);
  small_stepper.setSpeed(700);
}

// Store the timestamp of the last transmission so that we can see
// when the last transmission was last transmitted
unsigned int lastTransmission;

void loop()
{
  // Adjust the stepper if requested
  //while (digitalRead(5) == HIGH) small_stepper.step(2);
  //while (digitalRead(6) == HIGH) small_stepper.step(-2);

  /*
    int sensorValue = analogRead(A0); //read the A0 pin value
    float voltage = sensorValue * (5.00 / 1023.00) * 3.00; //convert the value to a true voltage.;
    bat = lambda * voltage + beta;
  */

  if (irrecv.decode(&results)) { // have we received an IR signal?
    digitalWrite(13, HIGH);
    switch (results.value) {
      case 16: // UP button pressed
      case 1153:
        small_stepper.step(stepsToTake);
        currentPosition += stepsToTake;
        break;

      case 2065: // DOWN button pressed
      case 3201:
        small_stepper.step(-stepsToTake/4);
        currentPosition -= stepsToTake/4;
        break;

      case 97: // RESET button pressed
        small_stepper.step(256);
        currentPosition = 32/degsPerStep;
        break;
      case 2689: // POWER button pressed
        small_stepper.step(-256);
        currentPosition = 0;
        break;
    }
    digitalWrite(13, LOW);
    Serial.println(results.value);
    irrecv.resume(); // receive the next value
  }

  // Disable stepper motor
  digitalWrite(11, LOW);
  digitalWrite(10, LOW);
  digitalWrite(9, LOW);
  digitalWrite(8, LOW);


  if (millis() % 65535 - lastTransmission > 500) {
    lastTransmission = millis();

    digitalWrite(13, HIGH);
    //Read data and store it to variables hum and temp
    hum = dht.readHumidity();
    temp = dht.readTemperature();
    digitalWrite(13, LOW);

    // Convert to integer and fractional parts
    double tempInt;
    float tempFrac = modf(temp, &tempInt);
    double batInt;
    float batFrac = modf(temp, &batInt);
    double lvlInt;
    float lvlFrac = modf(currentPosition * degsPerStep, &lvlInt);

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

