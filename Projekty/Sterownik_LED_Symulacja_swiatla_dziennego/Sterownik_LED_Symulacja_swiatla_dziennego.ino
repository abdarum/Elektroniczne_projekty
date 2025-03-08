
/*

Prompt:
Mam fotorezystor Spectral peak (nm): 540 Light Resistance (10Lux) (KΩ) :10-20 Dark Resistance (MΩ): 1

Potrzebuję zrobić urządzenie na arduino, które na podstawie poziomu naświetlenia(fotorezystor) będzie sterowało sygnałem PWM, który zasili pasek diod LED 12V, tak aby jasność paska led odpowiadała jasności naświetlenia odbieranego przez fotorezystor.

Arduino będzie odczytywało poziom napięcia z dzielnika napięcia, który będzie oparty na fotorezystorze i na podstawie tego napięcia będzie ustawiało sygnał PWM. Będą ustawione 3 granice(przy pomocy potencjometrów) - minimalna wartość napięcia dla fotodiody(odpowiadająca ciemności na fotodiodzie - PWM 0%), maksymalna wartość jaką może przyjąć PWM (np 50%, żeby nie razić zbyt mocnym światłem) i maksymalna wartość jasności na fotodiodzie - pełen dzień w pełnym słońcu - będzie odpowiadała pełnej jasności - max PWM

Wygeneruj kod źródłowy dla arduino, który na to pozwoli



Układ dzielnika napięcia:

5V
  |
Fotorezystor
  |
  |---- Wyjście dzielnika
  |
R1 - 10k Ohm
  |
GND

Wyjście napięcia dzielnika dla poziomów oświetlenia (dla 5V):

* V(dzielnika): 4.9V Fotorezystor: 180 Ohm - Pełna jasność - przyłożone do żarówki
* V(dzielnika): 3.7V Fotorezystor: 3.5k Ohm - leżące w pokoju na łóżku(światło z górnej żarówki)
* V(dzielnika): 1.7V Fotorezystor: 20k Ohm - Słabe światło
* V(dzielnika): 0.1V Fotorezystor: 5M(do 50M) Ohm - ciemność


*/

#include "global.h"
#include <arduino.h>

#define SERIAL_ENA 0

#define LED_PWM_OUT_PIN 0 // Pin PWM dla paska LED
#define PHOTORESISTOR_PIN A2 // Pin fotorezystora
#define MAX_PWM_POT_PIN A3 // Potencjometry do ustawiania max PWM


#define MIN_VOLTAGE_TRESCHOLD 0.1 // 10%~, ~0.5V , ~650k Ohm 
#define MAX_ADC_VALUE 1024
#define MAX_PWM_VALUE 255

INT maxPwm = 0;
INT photoresAdcValueMin = MIN_VOLTAGE_TRESCHOLD*MAX_ADC_VALUE;
INT photoresAdcValueMax = MIN_VOLTAGE_TRESCHOLD;

UINT map_uint(UINT x, UINT in_min, UINT in_max, UINT out_min, UINT out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void setup() {
  pinMode(LED_PWM_OUT_PIN, OUTPUT);
  #if SERIAL_ENA
  Serial.begin(9600);
  #endif
}

void loop() {
  // Odczyt wartości z potencjometru max wypełnienie PWM + Adaptacja do max dla PWM
  maxPwm = analogRead(MAX_PWM_POT_PIN) * MAX_PWM_VALUE / MAX_ADC_VALUE; // Odczyt max wypełnienie PWM

  // Odczyt wartości z fotorezystora
  INT sensorValue = analogRead(PHOTORESISTOR_PIN);

  INT pwmValue = 0;
  if (sensorValue > photoresAdcValueMin)
  {
    // Mapowanie wartości fotorezystora na zakres PWM
    pwmValue = map_uint(sensorValue, photoresAdcValueMin, photoresAdcValueMax, 0, maxPwm);
    pwmValue = constrain(pwmValue, 0, maxPwm); // Ograniczenie wartości PWM do maxPwm
  }

  analogWrite(LED_PWM_OUT_PIN, pwmValue); // Ustawienie sygnału PWM na pinie LED

  #if SERIAL_ENA
  // Debugowanie
  Serial.print("Sensor Value: ");
  Serial.print(sensorValue);
  Serial.print(" | PWM Value: ");
  Serial.println(pwmValue);
  Serial.print(" | Min Value: ");
  Serial.println(photoresAdcValueMin);
  Serial.print(" | Max Value: ");
  Serial.println(photoresAdcValueMax);
  #endif

  delay(100); // Małe opóźnienie dla stabilności odczytów
}
