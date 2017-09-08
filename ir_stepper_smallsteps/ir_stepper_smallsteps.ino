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

/*-----( Declare objects )-----*/
// Setup of proper sequencing for Motor Driver Pins
// In1, In2, In3, In4 in the sequence 1-3-2-4
// 5.62 degrees / 64 steps

Stepper small_stepper(STEPS, 8, 10, 9, 11);
#if !DEBUG
IRrecv irrecv(receiver);    // create instance of 'irrecv'
decode_results results;     // create instance of 'decode_results'
#else
struct { int value; } results;
#endif

void setup()
{
  #if !DEBUG
  irrecv.enableIRIn(); // Start the receiver
  #endif
  Serial.begin(9600);
  pinMode(13, OUTPUT);
}

void loop()
{
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
  #endif
    switch (results.value) {
      case 16: // UP button pressed
        small_stepper.setSpeed(700); //Max seems to be 700
        Steps2Take  =  256;  // Rotate CW
        small_stepper.step(Steps2Take);
        break;

      case 2065: // DOWN button pressed
        small_stepper.setSpeed(700);
        Steps2Take  =  -256;  // Rotate CCW
        small_stepper.step(Steps2Take);
        break;

      case 107: // RESET button pressed
        digitalWrite(13, HIGH);
        small_stepper.setSpeed(160);
        small_stepper.step(30);delay(100);small_stepper.step(-30);
        digitalWrite(13, LOW);
        break;

    }

    // Disable stepper motor
    digitalWrite(9, LOW);
    digitalWrite(8, LOW);
    Serial.println(results.value);

    #if !DEBUG
    irrecv.resume(); // receive the next value
    #endif
  }

}
