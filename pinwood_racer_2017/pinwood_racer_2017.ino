#include <SoftwareServo.h>

/* Program constants */
#define STATE_OFF 0
#define STATE_ARM 1
#define STATE_HOT 2
#define STATE_RUN 3

/* What PWM should be used for each throttle setting. */
#define FULL_THROTTLE 40
#define IDLE_THROTTLE 20
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
#define CONTROL_PIN 0
int arm_count = 0;
int state = STATE_OFF;

/* The setup function is caled when the arduino starts up.
 */
void setup()
{
  myservo.attach(CONTROL_PIN);  // attaches the servo on pin 2 to the servo object
  pinMode(BTN, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
}

/* Full Throttle mode -- runs the fan at full throttle for X seconds.
 *  TODO: Reset to ARM if we run for less than a number of cycles and the switch is pressed.
 *  (AKA debounce)
 */
int state_run(){
  digitalWrite(LED, HIGH);
  myservo.write(FULL_THROTTLE);
  for (int i=0; i < FT_TIMESTEPS; i++) {
    SoftwareServo::refresh();
    delay(20);   // waits for the servo to get there
    // If the button is pressed again, return to hot.
    if (digitalRead(BTN)) {
      return STATE_HOT;
    }
  }
  return STATE_OFF;
}

/* The only way out of the hot state is STATE_RUN or to press the reset button.  
 *  Transition to run happens when the gate drops, releasing the button (pin 3 -> high)
 */
int state_hot(){
  // Light turns solid when armed.
  analogWrite(LED, 180);
  
  while (digitalRead(BTN)){
    SoftwareServo::refresh();
    delay(20);   // waits for the servo to get there
  }
  return STATE_RUN;
}


/* This is the function we call when the controller is ready to fire.
 * 
 * The basic function is that it waits at IDLE_THROTTLE for a minimum of 
 * ARM_THRESHOLD 20 ms time-steps to avoid accidental firing.
 * 
 * Releasing the button too soon goes back to OFF. Once the time is reached, 
 * system goes from ARM to HOT.
 */
int state_arm(){
  int arm_count = 0;

  analogWrite(LED, 32);
  myservo.write(IDLE_THROTTLE);
  // Limit must be pressed at least ARM_THRESHOLD * 20 ms to arm 
  while(digitalRead(BTN) == LOW) {
    SoftwareServo::refresh();
    delay(20);   // waits for the servo to get there

    // If we continuously increment this, it could overflow.
    arm_count += 1; 

    // Blink the light while we're still arming.
    analogWrite(LED, 20 * ((arm_count / 8) % 3));

    /* If the button is released, return to the off state. */
    if (digitalRead(BTN) == HIGH) {
      // If the limit is released before we're armed, return without incident.
      return STATE_OFF;
    } 

    /* After ARM_THRESHOLD loops, switch to the hot state. */
    if (arm_count >= ARM_THRESHOLD){
      return STATE_HOT;
    }
  }
}

/* The OFF State. Pressing the button goes to ARM.
 */
int state_off(){
  while(true) {
    SoftwareServo::refresh();
    delay(20);   // waits for the servo to get there

    /* When the BTN input is pulled low (connected to ground), switch to the arm state
     */
    if (digitalRead(BTN == LOW)) {
      return STATE_ARM;
    } else {
      myservo.write(STOP_THROTTLE);
      digitalWrite(LED, LOW);
    }
  }
}


/* The loop function runs continuously in the Arduino circuit.  In this case, we 
 *  execute a state manager.  Each state function returns the desired next state, 
 * which is captured by the state manager and used to call the output appropriate
 * state function.
 */
void loop()
{
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

