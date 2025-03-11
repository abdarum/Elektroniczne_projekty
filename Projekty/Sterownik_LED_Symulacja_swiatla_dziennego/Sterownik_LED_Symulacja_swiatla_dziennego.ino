
/*

This Arduino device controls the brightness of a 12V LED strip based on the light level detected by a photoresistor. The brightness of the LED strip will correspond to the light intensity received by the photoresistor. The Arduino reads the voltage level from a voltage divider that includes the photoresistor and adjusts the PWM signal accordingly.

The device operates on three important variables:
- Maximum PWM value (e.g., 50% to avoid too bright light) - set using a potentiometer after entering the maximum LED brightness setting mode.
- Minimum brightness value for the photoresistor (corresponding to darkness - PWM 0%) - set based on the current value of the photoresistor after entering the minimum LED brightness setting mode (the value for which the LEDs will not light).
- Maximum brightness value for the photoresistor - full daylight - corresponds to full brightness - max PWM. The maximum value will be tracked continuously and will be taken as the maximum brightness value from the last 7 days.

There will be one button used to set two parameters. Two types of presses will be possible: SP (Shortpress) - 1 sec and LP (Longpress) - 3 sec.

Modes activated by pressing:
- Shortpress: Activates the mode for setting the maximum LED brightness. The LED will light up to the maximum available brightness, and the potentiometer can be used to set the max brightness (PWM). The LED brightness will be updated in real-time during adjustment. After entering the max brightness setting mode, there will be 3 minutes to confirm the new brightness by pressing shortpress. If shortpress is not pressed, the device will return to the standard mode without setting the current value.

- Longpress: Activates the mode for setting the minimum value. After entering this mode, the user will have 3 minutes to confirm the current value read from the photoresistor as the minimum value. Confirmation is done by shortpress, and if there is no response, the device will return to standard mode. This mode will be signaled by displaying three LED brightness values sequentially, each for 0.5 seconds:
  - 0% PWM
  - LED brightness corresponding to the current light level on the photoresistor, and the current minimum threshold
  - max LED brightness (max PWM)

Below the minimum brightness value for the photoresistor, the PWM will be at 0%. The values between the minimum and maximum brightness values for the photoresistor will be mapped from 0% to max PWM to reflect the brightness detected by the photoresistor.

------------------------------------------------------------------------

Voltage Divider Layout:

5V
  |
Photoresistor (Spectral peak (nm): 540 Light Resistance (10Lux) (KΩ): 10-20 Dark Resistance (MΩ): 1)
  |
  |---- Divider Output
  |
R1 - 10k Ohm
  |
GND

Voltage divider output for light levels (for 5V):

* V(divider): 4.9V Photoresistor: 180 Ohm - Full brightness - placed near a bulb
* V(divider): 3.7V Photoresistor: 3.5k Ohm - lying in the room on the bed (light from the ceiling bulb)
* V(divider): 1.7V Photoresistor: 20k Ohm - Low light
* V(divider): 0.1V Photoresistor: 5M (up to 50M) Ohm - darkness

Note:
- Max daylight value for photoresistor under balcony was: 1.3-1.8 k Ohm, to that was ~85% of Vcc voltage

For IR remote control handling, use the following code:
https://github.com/wagiminator/ATtiny13-TinyDecoder

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
