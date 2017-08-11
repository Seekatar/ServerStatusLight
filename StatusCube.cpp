#include "StatusCube.h"

bool StatusCube::initialize()
{
  if ( StatusLight::initialize() )
  {
    // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
    #if defined (__AVR_ATtiny85__)
      if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
    #endif
    // End of trinket special code

    _cube.begin();
    _cube.show(); // Initialize all pixels to 'off'

    _cube.setBrightness(100);

    return true;
  }
}

void StatusCube::setLayer( byte layer, uint32_t c )
{
  byte start = layer*CUBE_DIMENSION*CUBE_DIMENSION;
  for ( int i = start; i < start + CUBE_DIMENSION*CUBE_DIMENSION; i++ )
  {
    _cube.setPixelColor(i,c);
  }
  logMsg( "Set all layer %d to %x", (int)layer, c );
  _cube.show();
}

bool StatusCube::process( bool newValues )
{
  StatusLight::process(newValues);

  if ( newValues )
  {
    bool newRed = false;
    for ( int i = 0; i < SystemStatus::STATUS_COUNT && i < CUBE_DIMENSION && _system.BuildStatuses[i] != SystemStatus::BuildStatus::BuildUnknown; i++ )
    {
      logMsg( "Checking for a red one %d %d %d", (int)(!_haveRed), _system.BuildStatuses[i], SystemStatus::BuildStatus::Failure );
      if ( _system.BuildStatuses[i] == SystemStatus::BuildStatus::Failure )
      {
        newRed = true;
        if ( !_haveRed )
        {
          for ( int j = 0; j < 3; j++ )
          {
            delay(500);
            setLayer(CUBE_DIMENSION-1, _brightred);
            delay(500);
            setLayer(CUBE_DIMENSION-1, 0);
          }
          break;
        }
      }
    }
    _haveRed = newRed;

    for ( int i = 0; i < SystemStatus::STATUS_COUNT && i < CUBE_DIMENSION && _system.BuildStatuses[i] != SystemStatus::BuildStatus::BuildUnknown; i++ )
    {
        setLayer((CUBE_DIMENSION-1)-i, mapBuildColor(_system.BuildStatuses[i]));
    }

    _cube.show();
  }

  return true;
}


