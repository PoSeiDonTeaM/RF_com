#include "Stepper.h"
#include "IRremote.h"

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
IRrecv irrecv(receiver);    // create instance of 'irrecv'
decode_results results;     // create instance of 'decode_results'

void setup()
{ 
  irrecv.enableIRIn(); // Start the receiver
  Serial.begin(9600);
  }

void loop()
{
if (irrecv.decode(&results)) // have we received an IR signal?

  {
    switch(results.value)

    {

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
                      
    }

    // Disable stepper motor
    digitalWrite(9, LOW);
    digitalWrite(8, LOW);
    Serial.println(results.value);

      irrecv.resume(); // receive the next value
  }  


}/* --end main loop -- */
