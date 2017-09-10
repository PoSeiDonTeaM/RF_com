#include <SPI.h> // Not actually used but needed to compile
#include <IRremote.h>
#include <Stepper.h>
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

Stepper small_stepper(STEPS, 8, 10, 9, 11);
IRrecv irrecv(receiver);    // create instance of 'irrecv'
decode_results results;

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

  irrecv.enableIRIn(); // Start the receiver
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10,OUTPUT);
  pinMode(11,OUTPUT);
  pinMode(13, OUTPUT);
  small_stepper.setSpeed(700);
}

void loop()
{
  // Store the timestamp of the last transmission so that we can see
  // when the last transmission was last transmitted
  static int lastTransmission = millis();

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
        small_stepper.step(512);
        break;
      case 2689: // POWER button pressed
        small_stepper.step(-256);
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


  if (millis() - lastTransmission > 500) {
    lastTransmission = millis();

    digitalWrite(13, HIGH);
    //delay(1);
    digitalWrite(13, LOW);
  }
}

