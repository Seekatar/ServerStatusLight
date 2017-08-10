#ifndef __STATUSWHEEL_H__
#define __STATUSWHEEL_H__

#include "StatusLight.h"
#include "NeoPixelWheel.h"

class StatusWheel : public StatusLight
{
private:
  NeoPixelWheel _wheel;
  static const int DATA_PIN = 12;
  static const int PIXEL_COUNT = 24;

public:
  StatusWheel(SystemStatus &systemStatus, void (*logMsg)(const char *,...), int brightnessPin = 0 ) : StatusLight( systemStatus, logMsg ),
       _wheel(PIXEL_COUNT, DATA_PIN, 0, brightnessPin )
  {
    _strip = &_wheel;
  }

  virtual bool initialize();

  virtual bool process(bool newValues);
};


#endif
