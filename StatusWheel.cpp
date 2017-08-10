#include "StatusWheel.h"

bool StatusWheel::initialize()
{
  if ( StatusLight::initialize() )
  {
    _wheel.initialize();

    _wheel.setBrightness(20);

    return true;
  }
}

bool StatusWheel::process( bool newValues )
{
  bool show =  false; // TEMP since setBrightness lossy _wheel.checkColorChange();
  if ( newValues )
  {
    int i = 0;
    for ( ; i < SystemStatus::STATUS_COUNT && _system.ServerStatuses[i] != SystemStatus::ServerStatus::Unknown; i++ )
    {
      uint32_t color =  mapServerColor(_system.ServerStatuses[i]);
      if ( color == _red && _redServers[i] == Blinking::NotRed )
        _redServers[i] = Blinking::UnAcked;

      _wheel.setPixelColor(i,color);
    }

    for ( int j = i; j < SystemStatus::STATUS_COUNT; j++ )
    {
      _wheel.setPixelColor(j,_gray);
    }

    int j = PIXEL_COUNT-1; // start at end and work back
    for ( int i = 0; i < SystemStatus::STATUS_COUNT && _system.BuildStatuses[i] != SystemStatus::BuildStatus::BuildUnknown; i++ )
    {
      _wheel.setPixelColor(j--, mapBuildColor(_system.BuildStatuses[i]));
    }

   show = true;
  }
  if ( show )
     _wheel.show();

  return true;
}


