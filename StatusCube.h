#ifndef __STATUSCUBE_H__
#define __STATUSCUBE_H__

#include "StatusLight.h"

class StatusCube : public StatusLight
{
private:
  Adafruit_NeoPixel _cube;

  const int CUBE_DIMENSION = 3;
  void setLayer( byte layer, uint32_t c );

  static const int DATA_PIN = 18;
  static const int PIXEL_COUNT = 27;
  bool _haveRed = false;

public:
  StatusCube(SystemStatus &systemStatus, void (*logMsg)(const char *,...), int brightnessPin = 0 ) : StatusLight( systemStatus, logMsg ),
       _cube(PIXEL_COUNT, DATA_PIN, NEO_RGB + NEO_KHZ800)
  {
    _strip = &_cube;
  }

  virtual bool initialize();

  virtual bool process(bool newValues);
};


#endif
