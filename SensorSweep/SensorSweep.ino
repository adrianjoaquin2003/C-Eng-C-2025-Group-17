#include "AFMotor_R4.h"

// Hardware
const int sensorPin = A0;   // analog sensor
AF_DCMotor motorL(3);       // left foot motor (M3) - adjust if swapped
AF_DCMotor motorR(4);       // right foot motor (M4)
AF_DCMotor sensorM(2);      // sensor motor

// Tuning parameters - tune these to match your robot
const int threshold = 500;        // analog threshold for line detection
const int walkSpeed = 180;        // 0-255
const unsigned long stepDuration = 200;    // ms for a full step forward/back (tune)
const unsigned long stepPause = 50;        // ms pause between step parts

// State
enum State {WALKING, STOPPED_SEARCHING};
State state = WALKING;

// helper to read sensor
inline bool onLine() {
  int v = analogRead(sensorPin);
  // return true when sensor sees the line (tune threshold)
  return (v < threshold);
}

// primitive motor control helpers
void holdAll() {
  // stop both motors and "hold" (RELEASE stops powering; if you need braking use BRAKE)
  motorL.run(RELEASE);
  motorR.run(RELEASE);
}

void stepForward(AF_DCMotor* m, int speed, unsigned long duration) {
  m->setSpeed(speed);
  m->run(FORWARD);
  unsigned long start = millis();
  while (millis() - start < duration) {
    // allow sensor checks if needed (but this simple step blocks)
    delay(5);
  }
  // bring foot back to neutral (brief reverse to plant more firmly if needed)
  m->run(BACKWARD);
  m->setSpeed(speed/2);
  delay(stepPause);
  m->run(RELEASE);
}

// A walking cycle: left then right (one step each)
void walkCycle() {
  // left step
  stepForward(&motorL, walkSpeed, stepDuration);

  // check quickly whether we lost the line mid-cycle
  if (!onLine())
    return;

  // right step
  stepForward(&motorR, walkSpeed, stepDuration);
}

// Search routine: short alternating pulses, return which foot triggered detection
// returns: 0 = not found, 1 = left foot found, 2 = right foot found
int searchForLine() {

  sensorM.run(FORWARD);
  for (int i = 255; i != 0; i--) {
    sensorM.setSpeed(i);  
    if(onLine())
      return 1;
    delay(5);
  }

  sensorM.run(BACKWARD);
  for (int i = 255; i != 0; i--) {
    sensorM.setSpeed(i);  
    if(onLine())
      return 2;
    delay(5);
  }

  return 0;
}

// corrective step: if leftFound==true then sensor became over line while left moved.
// we choose to move the opposite foot to recenter.
void correctiveStep(int footFound) {
  if (footFound == 1) {
    // left found line -> step right to recenter
    Serial.println("Correct: stepping RIGHT once");
    stepForward(&motorR, walkSpeed, stepDuration);
  } else if (footFound == 2) {
    // right found line -> step left to recenter
    Serial.println("Correct: stepping LEFT once");
    stepForward(&motorL, walkSpeed, stepDuration);
  }
}

void setup() {
  pinMode(sensorPin, INPUT);
  Serial.begin(115200);
  motorL.setSpeed(walkSpeed);
  motorR.setSpeed(walkSpeed);
  holdAll();
}

void loop() {
  if (state == WALKING) {
    // normal walking: do a cycle while sensor says we're still on the line
    if (onLine()) {
      walkCycle();
      // short delay to control cadence
      delay(50);
      // ready for next cycle
    } else {
      // lost line -> stop at flat feet and go to searching state
      Serial.println("Line lost: stop and search");
      holdAll();
      state = STOPPED_SEARCHING;
    }
  }

  else if (state == STOPPED_SEARCHING) {
    // ensure feet are flat (motors released)
    holdAll();
    // search with short alternating pulses
    int foundBy = searchForLine();
    switch (foundBy) {
      case 0:
        Serial.println("LINE SEARCH FAILED");
        // Optionally keep searching or do something else
        break;

      case 1:
        Serial.println("LINE FOUND by LEFT foot -> step RIGHT");
        correctiveStep(1);  // step right to recenter
        state = WALKING;    // back to walking now
        break;

      case 2:
        Serial.println("LINE FOUND by RIGHT foot -> step LEFT");
        correctiveStep(2);  // step left to recenter
        state = WALKING;    // back to walking now
        break;
    }
  }
}
