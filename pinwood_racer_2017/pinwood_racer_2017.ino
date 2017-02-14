#include <SoftwareServo.h>

/* Program constants 
*/
#define STATE_OFF 0
#define STATE_ARM 1
#define STATE_HOT 2
#define STATE_RUN 3

/* What PWM should be used for each throttle setting. */
#define FULL_THROTTLE 80 // 50 for testing. 180 for flight.
#define IDLE_THROTTLE 30 
#define STOP_THROTTLE 10 

/* The number of 20 ms full throttle time steps to operate for. */
#define FT_TIMESTEPS 100

// Limit must be pressed at least ARM_THRESHOLD * 20 ms to arm 
#define ARM_THRESHOLD 100

SoftwareServo myservo;  // create servo object to control a servo

/* What IO to use for the Button, the LED, and the control pin. 
 * the Button pin is configured as a pull up.  Connect it to ground to "press" the button.
 */
#define BTN 3
#define LED 1
#define SPARE_GND 4

#define CONTROL_PIN 0
int arm_count = 0;
int state = STATE_OFF;
/* BTN is a pull-up, so LOW means connected to ground (predded) */
#define BUTTON_PRESSED digitalRead(BTN) == LOW
#define BUTTON_RELEASED digitalRead(BTN) == HIGH

/* 
 *  The setup function is caled when the arduino starts up.
 */
void setup()
{
  myservo.attach(CONTROL_PIN);  // Attaches the servo on pin 2 to the servo object
  pinMode(BTN, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  pinMode(SPARE_GND, OUTPUT);

  digitalWrite(SPARE_GND, LOW);
  arm_count = 0;
  state = STATE_OFF;
}

/* 
 *  STATE_RUN is the race state of the system.
 *  
 *  Full Throttle mode -- runs the fan at full throttle for X seconds.
 *  TODO: Reset to ARM if we run for less than a number of cycles and the switch is pressed.
 *  (AKA debounce)
 *  
 *  *** EXIT STATES CHECKED ***
 */
int state_run(){
  digitalWrite(LED, HIGH);
  myservo.write(FULL_THROTTLE);
  for (int i=0; i < FT_TIMESTEPS; i++) {
    SoftwareServo::refresh();
    delay(20);
    
    // If the button is pressed again, return to hot.
    if (BUTTON_PRESSED) {
      return STATE_HOT;
    }
  }
  return STATE_OFF;
}

/* 
 *  STATE_HOT Means the vehicle is armed and ready to run.
 *  
 *  The only way out of the hot state is STATE_RUN or to press the reset button.  
 *  Transition to run happens when the gate drops, releasing the button (pin 3 -> high)
 *  
 *  *** EXIT STATES CHECKED ***
 */
int state_hot(){
  // Light turns solid when armed.
  analogWrite(LED, 180);
  
  myservo.write(IDLE_THROTTLE);
  while (BUTTON_PRESSED){
    SoftwareServo::refresh();
    delay(20);
  }
  // This is the only exit state.
  return STATE_RUN;
}


/* STATE_ARM is the arming state of the car.   
 *  
 * This is the function we call when the controller is ready to fire.
 * 
 * The basic function is that it waits at IDLE_THROTTLE for a minimum of 
 * ARM_THRESHOLD 20 ms time-steps to avoid accidental firing.
 * 
 * Releasing the button too soon goes back to OFF. Once the time is reached, 
 * system goes from ARM to HOT.
 * 
 *  *** EXIT STATES CHECKED ***
 */
int state_arm(){

  analogWrite(LED, 32);
  myservo.write(IDLE_THROTTLE);
  // Limit must be pressed at least ARM_THRESHOLD * 20 ms to arm 
  while(BUTTON_PRESSED) {
    SoftwareServo::refresh();
    delay(20);   // waits for the servo to get there

    // If we continuously increment this, it could overflow.
    arm_count += 1; 

    // Blink the light while we're still arming.
    analogWrite(LED, 20 * ((arm_count / 8) % 3));

    /* After ARM_THRESHOLD loops, switch to the hot state. */
    if (arm_count >= ARM_THRESHOLD){
      arm_count=0;
      return STATE_HOT;
    }
  }
  arm_count=0;
  return STATE_OFF;
}

/* STATE_OFF is the fan off mode.
 *  
 * This is the mode we initialize to and the mode we exit to after the race.
 *  
 * The OFF State. Pressing the button goes to ARM.
 * 
 * *** EXIT STATES CHECKED ***
 */
int state_off(){
  myservo.write(STOP_THROTTLE);
  analogWrite(LED, 0);
  while(BUTTON_RELEASED) {
    SoftwareServo::refresh();
    delay(20);   // waits for the servo to get there
  }
  // This happens when the button is pressed.
  return STATE_ARM;
}


/* The loop function runs continuously in the Arduino circuit.  In this case, we 
 *  execute a state manager.  Each state function returns the desired next state, 
 * which is captured by the state manager and used to call the output appropriate
 * state function.
 */
void loop()
{
  /* State is initialized in setup. */
  switch(state) {
    case STATE_OFF:
      state = state_off();
      break;
    case STATE_ARM:
      state = state_arm();
      break;
    case STATE_HOT:
      state = state_hot();
      break;
    case STATE_RUN:
      state = state_run();
      break;
  }
}

