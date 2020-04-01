#include <Servo.h>

#define DOF 4
#define PAD_ANALOG_VAL_MIN 0
#define PAD_ANALOG_VAL_MAX 1023

// Decralation 4 servos
// Deklaracja 4 serw
Servo servo[DOF];
// Current position of controllers pads 
// Obecne położenie gałek kontrolerów
int current_pos[DOF];
// Controllers pads to analog INPUT
// Pady kontrolera podłączone do odpowiednich wejść uC 
int potentiometers_pinout[DOF] = {A0,A1,A2,A3};
// Servos pinout
// Wyjścia serw podłączone do pinów
int servos_pinout[DOF] = {4,5,6,7};

/************* DEFINITIONS OF FUNCTIONS *************/

// Set up all servos - attach servo output to pins
// Ustaw wszyskie serwa - podłącz serwa do odpowiednich pinów
void setup_servos();

// Set up all pads - attach Controllers pads to analog INPUT
// Ustaw wszyskie pady - podłącz pady do odpowiednich pinów analogowych
void setup_potentiometers();

// Get values from all pads
// Pobierz wartości położenia wszystkich padów
void get_potentiometers();

// Set all servos to curent pads position
// Ustaw położenie wszystkich serw zgodnie z aktualnym położeniem padów
void set_servos();


/****************** MAIN FUNCTIONS ******************/
void setup() {
  // Default function: setup all needed modules during start
  // Domyślna funkcja: która służy do ustawienia wszystkich modułów podczas startu
  setup_servos();
  setup_potentiometers();
}

void loop() {
  // Default function: put your main code here, to run repeatedly:
  // Domyślna funkcja: umieść tu wszystkie zadania, które powinny się powtarzać
  get_potentiometers();
  set_servos();
}


/**************** BODY OF FUNCTIONS *****************/
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
      PAD_ANALOG_VAL_MIN, PAD_ANALOG_VAL_MAX, 0, 180);
  }  
}

void set_servos(){
  for(int i=0; i<DOF; i++){
    servo[i].write(current_pos[i]);
  }
}
