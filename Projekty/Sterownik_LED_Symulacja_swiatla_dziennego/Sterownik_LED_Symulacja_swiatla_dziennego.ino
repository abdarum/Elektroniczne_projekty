
/*

This Arduino device controls the brightness of a 12V LED strip based on the light level detected by a photoresistor. The brightness of the LED strip will correspond to the light intensity received by the photoresistor. The Arduino reads the voltage level from a voltage divider that includes the photoresistor and adjusts the PWM signal accordingly.

The device operates on three important variables:
- Maximum PWM value (e.g., 50% to avoid too bright light) - set using a potentiometer after entering the maximum LED brightness setting mode.
- Minimum brightness value for the photoresistor (corresponding to darkness - PWM 0%) - set based on the current value of the photoresistor after entering the minimum LED brightness setting mode (the value for which the LEDs will not light).
- Maximum brightness value for the photoresistor - full daylight - corresponds to full brightness - max PWM. The maximum value will be tracked continuously and will be taken as the maximum brightness value from the last 7 days.

There will be one button used to set two parameters. Two types of presses will be possible: SP (Shortpress) - 1 sec and LP (Longpress) - 3 sec.

Modes activated by pressing:
- Shortpress: Activates the mode for setting the maximum LED brightness. The LED will light up to the maximum available brightness, and the potentiometer can be used to set the max brightness (PWM). The LED brightness will be updated in real-time during adjustment. After entering the max brightness setting mode, there will be 3 minutes to confirm the new brightness by pressing shortpress. If shortpress is not pressed, the device will return to the standard mode without setting the current value.

- Longpress: Activates the mode for setting the minimum value. After entering this mode, the user will have 3 minutes to confirm the current value read from the photoresistor as the minimum value. Confirmation is done by shortpress, and if there is no response, the device will return to standard mode. This mode will be signaled by displaying three LED brightness values sequentially, each for 1.5 seconds:
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

#define LED_PWM_OUT_PIN 0    // PWM pin for the LED strip
#define PHOTORESISTOR_PIN A1 // Pin for the photoresistor
#define MAX_PWM_POT_PIN A3   // Potentiometer for setting max PWM
#define BUTTON_PIN 1         // Pin for the button

#define MIN_VOLTAGE_DEFAULT_THRESHOLD 0.1  // 10%~, ~0.5V , ~650k Ohm
#define MAX_VOLTAGE_DEFAULT_THRESHOLD 0.85 // 85%~, ~1.5k Ohm
#define MAX_ADC_VALUE 1024
#define MAX_PWM_VALUE 255

enum Mode
{
  STANDARD,
  SETTING_MIN_BRIGHTNESS,
  SETTING_MAX_PWM
};

enum ButtonPress
{
  NO_PRESS,
  SHORT_PRESS,
  LONG_PRESS
};

Mode currentMode = STANDARD;
ButtonPress buttonPress = NO_PRESS;

INT maxPwm = 0;
INT photoresistorAdcValueMin = MIN_VOLTAGE_DEFAULT_THRESHOLD * MAX_ADC_VALUE;
INT photoresistorAdcValueMax = MAX_VOLTAGE_DEFAULT_THRESHOLD * MAX_ADC_VALUE;
ULONG lastPressTime = 0;
ULONG lastButtonReadTime = 0;
ULONG lastModeChangeTime = 0;

#if SERIAL_ENA
ULONG lastSerialSendTime = 0;
#endif

UINT map_uint(UINT x, UINT in_min, UINT in_max, UINT out_min, UINT out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

UINT getAdjustmentValuePwmMapped()
{
  return analogRead(MAX_PWM_POT_PIN) * MAX_PWM_VALUE / MAX_ADC_VALUE;
}

UINT getSensorValue()
{
  return analogRead(PHOTORESISTOR_PIN);
}

ButtonPress checkButtonPress()
{
  BOOL buttonState = digitalRead(BUTTON_PIN);
  ULONG currentTime = millis();

  if (buttonState == HIGH)
  { // Button pressed (pulldown configuration)
    if (currentTime - lastPressTime > 6000)
    {
      return NO_PRESS;
    }
    else if (currentTime - lastPressTime > 2000)
    {
      return LONG_PRESS;
    }
    else if (currentTime - lastPressTime > 500)
    {
      return SHORT_PRESS;
    }
  }
  else
  {
    lastPressTime = currentTime;
  }

  return NO_PRESS;
}

void updateLedBrightness()
{
  ULONG currentTime = millis();
  INT pwmValue = 0;
  INT sensorValue = getSensorValue();
  if (sensorValue > photoresistorAdcValueMin)
  {
    pwmValue = map_uint(sensorValue, photoresistorAdcValueMin, photoresistorAdcValueMax, 0, maxPwm);
    pwmValue = constrain(pwmValue, 0, maxPwm);
  }

  switch (currentMode)
  {
  case STANDARD:
  {
    analogWrite(LED_PWM_OUT_PIN, pwmValue);
  }
  break;

  case SETTING_MAX_PWM:
    INT tmpMaxPwm = getAdjustmentValuePwmMapped();
    analogWrite(LED_PWM_OUT_PIN, tmpMaxPwm);
    break;

  case SETTING_MIN_BRIGHTNESS:
    if (currentTime - lastModeChangeTime < 1500)
    {
      analogWrite(LED_PWM_OUT_PIN, 0);
    }
    else if (currentTime - lastModeChangeTime < 3000)
    {
      analogWrite(LED_PWM_OUT_PIN, pwmValue);
    }
    else if (currentTime - lastModeChangeTime < 4500)
    {
      analogWrite(LED_PWM_OUT_PIN, maxPwm);
    }
    else
    {
      lastModeChangeTime = currentTime;
    }
    break;
  }

#if SERIAL_ENA
  if (currentTime - lastSerialSendTime > 500)
  {
    Serial.print("Sensor Value: ");
    Serial.print(sensorValue);
    Serial.print(" | PWM Value: ");
    Serial.println(pwmValue);
    Serial.print(" | Min Value: ");
    Serial.println(photoresistorAdcValueMin);
    Serial.print(" | Max Value: ");
    Serial.println(photoresistorAdcValueMax);
    lastSerialSendTime = currentTime;
  }
#endif
}

void setup()
{
  pinMode(LED_PWM_OUT_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
#if SERIAL_ENA
  Serial.begin(9600);
#endif
}

void loop()
{
  ULONG currentTime = millis();
  buttonPress = checkButtonPress();

  switch (currentMode)
  {
  case STANDARD:
    if (buttonPress == SHORT_PRESS)
    {
      currentMode = SETTING_MAX_PWM;
      lastModeChangeTime = currentTime;
    }
    else if (buttonPress == LONG_PRESS)
    {
      currentMode = SETTING_MIN_BRIGHTNESS;
      lastModeChangeTime = currentTime;
    }
    break;

  case SETTING_MAX_PWM:
    if (currentTime - lastModeChangeTime > 180000)
    { // 3 minutes timeout
      currentMode = STANDARD;
    }
    else if (buttonPress == SHORT_PRESS)
    {
      maxPwm = getAdjustmentValuePwmMapped();
      currentMode = STANDARD;
    }
    break;

  case SETTING_MIN_BRIGHTNESS:
    if (currentTime - lastModeChangeTime > 180000)
    { // 3 minutes timeout
      currentMode = STANDARD;
    }
    else if (buttonPress == SHORT_PRESS)
    {
      photoresistorAdcValueMin = getSensorValue();
      currentMode = STANDARD;
    }
    break;
  }

  updateLedBrightness();
}
