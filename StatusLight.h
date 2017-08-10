#ifndef __STATUSLIGHT_H__
#define __STATUSLIGHT_H__

#include "SystemStatus.h"
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

class StatusLight
{
protected:
  SystemStatus &_system;
  void (*logMsg)(const char *, ...);

  uint32_t mapServerColor(SystemStatus::ServerStatus stat );
  uint32_t mapBuildColor(SystemStatus::BuildStatus stat );

  enum Blinking
  {
    NotRed,
    UnAcked,
    Acked
  };

  Blinking _redServers[SystemStatus::STATUS_COUNT];
  uint32_t _red;
  uint32_t _green;
  uint32_t _blue;
  uint32_t _yellow;
  uint32_t _orange;
  uint32_t _gray;
  uint32_t _cyan;
  uint32_t _brightred;
  uint32_t _purple;

  Adafruit_NeoPixel *_strip;
  StatusLight(SystemStatus &systemStatus, void (*logMsg)(const char *,...), int brightnessPin = 0 ) :  logMsg( logMsg ),  _system(systemStatus)
  {}

public:
  virtual bool initialize();

  virtual bool process(bool newValues);
};


#endif
