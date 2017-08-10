/*
Adapted
 LEDC Software Fade

 This example shows how to software fade LED
 using the ledcWrite function.

 Code adapted from original Arduino Fade example:
 https://www.arduino.cc/en/Tutorial/Fade

 This example code is in the public domain.
 */
#ifndef _ESP32_ANALOG_WRITE_H_

#ifdef XESP_PLATFORM // SparkFun ESP32 Thing

#define _ESP32_ANALOG_WRITE_H_

// use first channel of 16 channels (started from zero)
#define LEDC_CHANNEL_0     0

// use 13 bit precission for LEDC timer
#define LEDC_TIMER_13_BIT  13

// use 5000 Hz as a LEDC base frequency
#define LEDC_BASE_FREQ     5000

// Arduino like analogWrite
// value has to be between 0 and valueMax
void analogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255) {
  // calculate duty, 8191 from 2 ^ 13 - 1
  uint32_t duty = (8191 / valueMax) * min(value, valueMax);

  // write duty to LEDC
  ledcWrite(channel, duty);
}

void analogWriteSetup(int ledPin ) {
  // Setup timer and attach timer to a led pin
  ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
  ledcAttachPin(ledPin, LEDC_CHANNEL_0);
}
#else
void analogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255) {}
void analogWriteSetup(int ledPin ) {}
#endif
#endif
