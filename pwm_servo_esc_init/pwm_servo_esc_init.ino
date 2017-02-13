#include <SoftwareServo.h>

#define FULL_THROTTLE 170
#define IDLE_THROTTLE 20
#define STOP_THROTTLE 10 

// Limit must be pressed at least ARM_THRESHOLD * 20 ms to arm 
#define ARM_THRESHOLD 200

SoftwareServo myservo;  // create servo object to control a servo

int btn = 3;
int led = 1;
int arm_count = 0;

void setup()
{
  myservo.attach(0);  // attaches the servo on pin 2 to the servo object
  pinMode(btn, INPUT_PULLUP);
  pinMode(led, OUTPUT);
}

void full_throttle(){
  myservo.write(FULL_THROTTLE);
  for (int i=0; i < 100; i++) {
    SoftwareServo::refresh();
    delay(20);   // waits for the servo to get there
  }  
}

void stop_throttle(){
  myservo.write(STOP_THROTTLE);
  SoftwareServo::refresh();
  delay(20);   // waits for the servo to get there
}

void state_hot(){
  int arm_count = 0;
  myservo.write(IDLE_THROTTLE);
  // Limit must be pressed at least ARM_THRESHOLD * 20 ms to arm 
  while((arm_count < ARM_THRESHOLD ) || !digitalRead(btn)) {
    SoftwareServo::refresh();
    delay(20);   // waits for the servo to get there

    // Don't be actually armed until arm_count > ARM_THRESHOLD
    if (arm_count < ARM_THRESHOLD) {
      arm_count += 1;
      
      // Blink the light while we're still arming.
      analogWrite(led, 20 * ((arm_count / 8) % 3));
      if (digitalRead(btn)) {
        // If the limit is released before we're armed, return without incident.
        return ;
      } 
    } else {
      analogWrite(led, 180);
    }
  }
  digitalWrite(led, HIGH);
  full_throttle();
  stop_throttle();
  digitalWrite(led, LOW);
}


void loop()
{
  if (!digitalRead(btn)) {
    analogWrite(led, 32);
    state_hot();
  } else {
    myservo.write(10);
    digitalWrite(led, LOW);
  }
  SoftwareServo::refresh();
  delay(20);   // waits for the servo to get there
}

