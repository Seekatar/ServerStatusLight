#include "StatusWheel.h"

bool StatusWheel::initialize()
{
  _wheel.initialize();

  _blue = _wheel.Color(0, 0, 255);
  _green = _wheel.Color(0, 255, 0);
  _red = _wheel.Color(255, 0, 0);
  _yellow = _wheel.Color(255, 255, 0);
  _orange = _wheel.Color(255, 128, 0);

  _wheel.setBrightness(50);
  return true;
}

uint32_t StatusWheel::mapServerColor(SystemStatus::ServerStatus stat )
{
  switch ( stat )
  {
    case SystemStatus::ServerStatus::Red:
      return _red;
    case SystemStatus::ServerStatus::Green:
      return _green;
    case SystemStatus::ServerStatus::Orange:
      return _orange;
    default:
      return 0;  
  }
}

uint32_t StatusWheel::mapBuildColor(SystemStatus::BuildStatus stat )
{
  switch ( stat )
  {
    case SystemStatus::BuildStatus::Failure:
      return _red;
    case SystemStatus::BuildStatus::Success:
      return _green;
    case SystemStatus::BuildStatus::Staged:
    case SystemStatus::BuildStatus::Processing:
      return _blue;
    default:
      return 0;  
  }
}

bool StatusWheel::process( bool newValues )
{
  if ( newValues )
  {
    for ( int i = 0; i < SystemStatus::STATUS_COUNT && _system.ServerStatuses[i] != SystemStatus::ServerStatus::Unknown; i++ )
    {
      uint32_t color =  mapServerColor(_system.ServerStatuses[i]);
      if ( color == _red && _redServers[i] == Blinking::NotRed )
        _redServers[i] = Blinking::UnAcked;
        
      _wheel.setPixelColor(i,color);
    }
  
    int j = PIXEL_COUNT-1; // start at end and work back
    for ( int i = 0; i < SystemStatus::STATUS_COUNT && _system.BuildStatuses[i] != SystemStatus::BuildStatus::BuildUnknown; i++ )
    {
      _wheel.setPixelColor(j--, mapBuildColor(_system.BuildStatuses[i]));
    }
  
    _wheel.show(); // Initialize all pixels to 'off'
  }
  return true;
}


