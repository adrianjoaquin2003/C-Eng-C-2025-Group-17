int sensorPin = A0;  // line detection sensor interface
int val;            // variable to store sensor reading
int checkspeed = 10;
#include "AFMotor_R4.h"
int line_direction[6];
int range = 180;
AF_DCMotor motor(3);  // Create motor on M3

void setup() {
  pinMode(sensorPin,INPUT);  // define sensor as input  
  Serial.begin(9600);     // initialize serial communication with PC
  motor.setSpeed(checkspeed);  // Set speed (0-255)
    // Start motor
}

void loop() {
  val = analogRead(sensorPin); // read value from sensor
  // for(int count;count<7;count++){
  // Serial.println(line_direction[count]);
//    }
  
  if (val == HIGH) { 
    Serial.println("Line detected"); 
  } else { 
    Serial.println("Line NOT detected"); 
  }

  delay(500);
}

void findLine(){
  int direction[6];
  int angle_id = 0;
  for (int i =0; i<= 180; i+=30){
    motor.run(FORWARD);
    delay(10);
    motor.run(BRAKE);
    line_direction[angle_id] = analogRead(sensorPin);
    delay(500);
    angle_id++;
  }
  return direction;
}
