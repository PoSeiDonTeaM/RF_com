#define DEBUG 1

#include "Stepper.h"
#if DEBUG
#else
#include "IRremote.h"
#endif

/*----- Variables, Pins -----*/
#define STEPS  32   // Number of steps per revolution of Internal shaft
int  Steps2Take;  // 2048 = 1 Revolution
int receiver = 6; // Signal Pin of IR receiver to Arduino Digital Pin 6
const int STEPPER_PINS[] = { 8, 10, 9, 11};
int stepsToTake = 256;

/*-----( Declare objects )-----*/
// Setup of proper sequencing for Motor Driver Pins
// In1, In2, In3, In4 in the sequence 1-3-2-4
// 5.62 degrees / 64 steps

const float degsPerStep = 5.625/64.0;

Stepper small_stepper(STEPS, 8, 10, 9, 11);
#if !DEBUG
IRrecv irrecv(receiver);    // create instance of 'irrecv'
decode_results results;     // create instance of 'decode_results'
#else
struct { int value; } results;
#endif
long currentPosition = 0;

void setup()
{
  #if !DEBUG
  irrecv.enableIRIn(); // Start the receiver
  #endif
  Serial.begin(9600);
  pinMode(13, OUTPUT);
  small_stepper.setSpeed(700);
}

void loop()
{
  while(digitalRead(5) == HIGH) {
    small_stepper.step(2);
  }
  while(digitalRead(6) == HIGH) {
    small_stepper.step(-2);
  }
  #if DEBUG
  if (digitalRead(2) == HIGH || digitalRead(3) == HIGH || digitalRead(4) == HIGH) { // have we received an IR signal?
    if (digitalRead(2) == HIGH) results.value = 16;
    else if (digitalRead(3) == HIGH) results.value = 2065;
    else results.value = 107;
    while(digitalRead(2) == HIGH) {}
    while(digitalRead(3) == HIGH) {}
    while(digitalRead(4) == HIGH) {}
  #else
  if (irrecv.decode(&results)) { // have we received an IR signal?
  // add infrared signal to override
  #endif
    switch (results.value) {
      case 16: // UP button pressed
        small_stepper.step(stepsToTake);
        currentPosition += stepsToTake;
        break;

      case 2065: // DOWN button pressed
        small_stepper.step(-stepsToTake);
        currentPosition -= stepsToTake;
        break;

      case 107: // RESET button pressed
        digitalWrite(13, HIGH);
        while(currentPosition != 0) {
          if (currentPosition > 0) {
            small_stepper.step(-1);
            currentPosition -= 1;
          } else {
            small_stepper.step(1);
            currentPosition += 1;
          }
        }
        digitalWrite(13, LOW);
        break;

    }

   
    Serial.println(results.value);

    #if !DEBUG
    irrecv.resume(); // receive the next value
    #endif
  }

  // Disable stepper motor
    digitalWrite(11,LOW);
    digitalWrite(10,LOW);
    digitalWrite(9, LOW);
    digitalWrite(8, LOW);
}

