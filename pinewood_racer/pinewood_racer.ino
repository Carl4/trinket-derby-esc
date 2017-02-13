 #include <SoftwareServo.h>

#define FULL_THROTTLE 100
#define IDLE_THROTTLE 30
#define STOP_THROTTLE 10 
// The number of full throttle time steps to operate for.
#define FT_TIMESTEPS 100

// Limit must be pressed at least ARM_THRESHOLD * 20 ms to arm 
#define ARM_THRESHOLD 100

SoftwareServo myservo;  // create servo object to control a servo

#define BTN 3
#define LED 1
#define CONTROL_PIN 0
int arm_count = 0;

void setup()
{
  myservo.attach(CONTROL_PIN);  // attaches the servo on pin 2 to the servo object
  pinMode(BTN, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
}

/* Full Throttle mode -- runs the fan at full throttle for X seconds.  
 *  
 *  
 */
void full_throttle(){
  myservo.write(FULL_THROTTLE);
  for (int i=0; i < FT_TIMESTEPS; i++) {
    SoftwareServo::refresh();
    delay(20);   // waits for the servo to get there
  }  
}

void stop_throttle(){
  myservo.write(STOP_THROTTLE);
  SoftwareServo::refresh();
  delay(20);   // waits for the servo to get there
}
/* This is the function we call when the controller is ready to fire.
 *  
 */
void state_hot(){
  int arm_count = 0;
  myservo.write(IDLE_THROTTLE);
  // Limit must be pressed at least ARM_THRESHOLD * 20 ms to arm 
  while((arm_count < ARM_THRESHOLD ) || !digitalRead(BTN)) {
    SoftwareServo::refresh();
    delay(20);   // waits for the servo to get there

    // Don't be actually armed until arm_count > ARM_THRESHOLD
    if (arm_count < ARM_THRESHOLD) {
      arm_count += 1;
      
      // Blink the light while we're still arming.
      analogWrite(LED, 20 * ((arm_count / 8) % 3));
      if (digitalRead(BTN)) {
        // If the limit is released before we're armed, return without incident.
        return ;
      } 
    } else {
      analogWrite(LED, 180);
    }
  }
  /* GO GO GO!!!! */
  digitalWrite(LED, HIGH);
  full_throttle();
  stop_throttle();
  digitalWrite(LED, LOW);
}


void loop()
{
  if (!digitalRead(BTN)) {
    analogWrite(LED, 32);
    state_hot();
  } else {
    myservo.write(STOP_THROTTLE);
    digitalWrite(LED, LOW);
  }
  SoftwareServo::refresh();
  delay(20);   // waits for the servo to get there
}

