#include <Servo.h>

#define DOF 4

Servo servo[DOF];
int current_pos[DOF];
int potentiometers_pinout[DOF] = {A0,A1,A2,A3};
int servos_pinout[DOF] = {4,5,6,7};

void setup_servos(){
  for(int i=0; i<DOF; i++){
    servo[i].attach(servos_pinout[i]);
  }
}

void setup_potentiometers(){
  for(int i=0; i<DOF; i++){
    pinMode(potentiometers_pinout[i], INPUT);
  }  
}

void get_potentiometers() {
  for(int i=0; i<DOF; i++){
    current_pos[i] = map(
      analogRead(potentiometers_pinout[i]),
      0, 1023, 0, 180);
  }  
}

void set_servos(){
  for(int i=0; i<DOF; i++){
    servo[i].write(current_pos[i]);
  }
}

void setup() {
  setup_servos();
  setup_potentiometers();
}

void loop() {
  // put your main code here, to run repeatedly:
  get_potentiometers();
  set_servos();
}
