// Based on:
//   * ReceiveAndSend.cpp
//   * ReceiveDump.cpp


#include <IRremote.hpp>

#define IR_RECEIVE_PIN 2  // To be compatible with interrupt example, pin 2 is chosen here.
#define IR_SEND_PIN 3

#define MODULATION_KHZ 33  // NEC_KHZ
#define REPEAT_COUNT 1

const uint16_t rawThomsonTurnOnOff[103] = { 4030, 4070, 480, 2020, 530, 1970, 530, 1020, 530, 970, 530, 1020, 530, 970, 530, 2020, 480, 1020, 530, 1970, 580, 970, 530, 1970, 530, 1020, 480, 1020, 530, 970, 530, 2020, 530, 1970, 530, 2020, 530, 1970, 530, 1020, 530, 1970, 530, 970, 580, 1970, 530, 970, 530, 2020, 430, 7420, 4030, 4020, 530, 1970, 580, 1970, 480, 1020, 530, 1020, 480, 1020, 530, 970, 530, 2020, 530, 970, 530, 2020, 480, 1020, 530, 2020, 480, 1020, 530, 970, 530, 1020, 530, 1970, 530, 2020, 480, 2020, 530, 1970, 530, 1020, 480, 2020, 530, 1020, 480, 2020, 530, 1020, 480, 2020, 430 };  // Protocol=UNKNOWN Hash=0x96FCD9FB 52 bits (incl. gap and start) received
const uint16_t rawThomsonMute[103] = { 4080, 3970, 580, 1970, 530, 1970, 580, 920, 580, 970, 580, 920, 580, 970, 530, 1970, 580, 1970, 530, 1970, 530, 1970, 580, 1970, 530, 1970, 580, 970, 530, 970, 580, 1920, 580, 1970, 530, 1970, 580, 1970, 530, 970, 580, 970, 530, 970, 530, 1020, 530, 970, 530, 1020, 480, 7370, 4030, 4020, 480, 2020, 530, 2020, 480, 1020, 530, 1020, 480, 1020, 480, 1070, 480, 2020, 480, 2020, 530, 2020, 480, 2020, 530, 2020, 480, 2020, 480, 1020, 530, 1020, 480, 2020, 530, 2020, 480, 2020, 530, 2020, 480, 1020, 480, 1020, 530, 1020, 480, 1020, 530, 1020, 480, 1020, 480 };        // Protocol=UNKNOWN Hash=0xE09FC8B7 52 bits (incl. gap and start) received
const uint16_t rawThomsonVolumeUp[103] = { 4080, 3970, 580, 1970, 530, 1970, 580, 970, 530, 970, 580, 920, 580, 970, 530, 1970, 580, 970, 530, 1970, 580, 1920, 580, 1970, 580, 1920, 580, 970, 530, 970, 580, 1970, 530, 1970, 580, 1920, 580, 1970, 530, 970, 580, 1970, 530, 970, 580, 970, 530, 970, 530, 1020, 480, 7370, 4030, 4020, 530, 2020, 480, 2020, 480, 1020, 530, 1020, 480, 1020, 530, 1020, 480, 2020, 480, 1070, 480, 2020, 480, 2020, 530, 2020, 480, 2020, 530, 1020, 480, 1020, 480, 2020, 530, 2020, 480, 2020, 530, 2020, 480, 1020, 530, 2020, 480, 1020, 480, 1020, 530, 1020, 480, 1020, 480 };     // Protocol=UNKNOWN Hash=0xF9EA922F 52 bits (incl. gap and start) received
const uint16_t rawThomsonVolumeDown[103] = { 4130, 3970, 580, 1920, 580, 1970, 530, 970, 580, 970, 530, 970, 580, 920, 580, 1970, 530, 970, 580, 1970, 530, 1970, 580, 1920, 580, 970, 580, 920, 580, 970, 530, 1970, 580, 1970, 530, 1970, 580, 1920, 580, 970, 530, 1970, 580, 970, 530, 1020, 480, 1020, 480, 2020, 480, 7370, 4030, 4070, 480, 2020, 530, 2020, 480, 1020, 480, 1020, 530, 1020, 480, 1020, 530, 2020, 480, 1020, 480, 2020, 530, 2020, 480, 2020, 530, 1020, 480, 1020, 530, 1020, 480, 2020, 480, 2020, 530, 2020, 480, 2020, 530, 1020, 480, 2020, 530, 1020, 480, 1020, 480, 1020, 530, 2020, 480 };  // Protocol=UNKNOWN Hash=0x5C17CCBB 52 bits (incl. gap and start) received

#define decodedRawDataChiqMute 0xEF10BF40
#define decodedRawDataChiqVolumeUp 0xE51ABF40
#define decodedRawDataChiqVolumeDown 0xE11EBF40

#define decodedRawDataSamsungAudioTurnOnOff 0x4000401
#define decodedRawDataSamsungAudioMute 0x5100401
#define decodedRawDataSamsungAudioVolumeUp 0x6200401
#define decodedRawDataSamsungAudioVolumeDown 0x7300401



bool handleReceivedIrCode() {
  bool command_sent = false;
  for (int i = 0; i < REPEAT_COUNT; i++) {
    if (IrReceiver.decodedIRData.decodedRawData == decodedRawDataChiqMute || IrReceiver.decodedIRData.decodedRawData == decodedRawDataSamsungAudioMute) {
      IrSender.sendRaw(rawThomsonMute, sizeof(rawThomsonMute) / sizeof(rawThomsonMute[0]), MODULATION_KHZ);  // Note the approach used to automatically calculate the size of the array.
      Serial.println("Mute sent to Sub-woofer Thompson DPL80HT");
      command_sent = true;
    }
    if (IrReceiver.decodedIRData.decodedRawData == decodedRawDataChiqVolumeUp || IrReceiver.decodedIRData.decodedRawData == decodedRawDataSamsungAudioVolumeUp) {
      IrSender.sendRaw(rawThomsonVolumeUp, sizeof(rawThomsonVolumeUp) / sizeof(rawThomsonVolumeUp[0]), MODULATION_KHZ);  // Note the approach used to automatically calculate the size of the array.
      Serial.println("Volume Up sent to Sub-woofer Thompson DPL80HT");
      command_sent = true;
    }
    if (IrReceiver.decodedIRData.decodedRawData == decodedRawDataChiqVolumeDown || IrReceiver.decodedIRData.decodedRawData == decodedRawDataSamsungAudioVolumeDown) {
      IrSender.sendRaw(rawThomsonVolumeDown, sizeof(rawThomsonVolumeDown) / sizeof(rawThomsonVolumeDown[0]), MODULATION_KHZ);  // Note the approach used to automatically calculate the size of the array.
      Serial.println("Volume Down sent to Sub-woofer Thompson DPL80HT");
      command_sent = true;
    }
    if (IrReceiver.decodedIRData.decodedRawData == decodedRawDataSamsungAudioTurnOnOff) {
      IrSender.sendRaw(rawThomsonTurnOnOff, sizeof(rawThomsonTurnOnOff) / sizeof(rawThomsonTurnOnOff[0]), MODULATION_KHZ);  // Note the approach used to automatically calculate the size of the array.
      Serial.println("Power On/Off sent to Sub-woofer Thompson DPL80HT");
      command_sent = true;
    }
  }
  return command_sent;
}

void printReceiveDumpInfo() {
  IrReceiver.printIRResultShort(&Serial);
  // Check if the buffer overflowed

  if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
    Serial.println(F("Received noise or an unknown (or not yet enabled) protocol"));
  }
  Serial.println();
  IrReceiver.printIRSendUsage(&Serial);
  Serial.println();
  Serial.println(F("Raw result in internal ticks (50 us) - with leading gap"));
  IrReceiver.printIRResultRawFormatted(&Serial, false);  // Output the results in RAW format
  Serial.println(F("Raw result in microseconds - with leading gap"));
  IrReceiver.printIRResultRawFormatted(&Serial, true);  // Output the results in RAW format
  Serial.println();                                     // blank line between entries
  Serial.print(F("Result as internal 8bit ticks (50 us) array - compensated with MARK_EXCESS_MICROS="));
  Serial.println(MARK_EXCESS_MICROS);
  IrReceiver.compensateAndPrintIRResultAsCArray(&Serial, false);  // Output the results as uint8_t source code array of ticks
  Serial.print(F("Result as microseconds array - compensated with MARK_EXCESS_MICROS="));
  Serial.println(MARK_EXCESS_MICROS);
  IrReceiver.compensateAndPrintIRResultAsCArray(&Serial, true);  // Output the results as uint16_t source code array of micros
  IrReceiver.printIRResultAsCVariables(&Serial);                 // Output address and data as source code variables
  Serial.println();                                              // blank line between entries

  IrReceiver.compensateAndPrintIRResultAsPronto(&Serial);

  /*
   * Example for using the compensateAndStorePronto() function.
   * Creating this String requires 2210 bytes program memory and 10 bytes RAM for the String class.
   * The String object itself requires additional 440 bytes RAM from the heap.
   * This values are for an Arduino Uno.
   */
  //        Serial.println();                                     // blank line between entries
  //        String ProntoHEX = F("Pronto HEX contains: ");        // Assign string to ProtoHex string object
  //        if (int size = IrReceiver.compensateAndStorePronto(&ProntoHEX)) {   // Dump the content of the IReceiver Pronto HEX to the String object
  //            // Append compensateAndStorePronto() size information to the String object (requires 50 bytes heap)
  //            ProntoHEX += F("\r\nProntoHEX is ");              // Add codes size information to the String object
  //            ProntoHEX += size;
  //            ProntoHEX += F(" characters long and contains "); // Add codes count information to the String object
  //            ProntoHEX += size / 5;
  //            ProntoHEX += F(" codes");
  //            Serial.println(ProntoHEX.c_str());                // Print to the serial console the whole String object
  //            Serial.println();                                 // blank line between entries
  //        }
  IrReceiver.resume();  // Prepare for the next IR frame
}

void printReceiveShortInfo() {
  Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);  // Print "old" raw data
  Serial.println(IrReceiver.decodedIRData.decodedRawData);       // Print "old" raw data
  IrReceiver.printIRResultShort(&Serial);                        // Print complete received data in one line
  IrReceiver.printIRSendUsage(&Serial);                          // Print the statement required to send this data
}

void setup() {
  Serial.begin(9600);
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);  // Start the receiver
  Serial.println("IR reader startup");

  // IR send
  IrSender.begin(IR_SEND_PIN);  // Start with IR_SEND_PIN -which is defined in PinDefinitionsAndMore.h- as send pin and enable feedback LED at default feedback LED pin
}

void loop() {
  if (IrReceiver.decode()) {
    if (!handleReceivedIrCode()) {
      printReceiveDumpInfo();
      // printReceiveShortInfo();
    }

    IrReceiver.resume();  // Enable receiving of the next value
  }
}