#ifndef __STATUSWHEEL_H__
#define __STATUSWHEEL_H__

#include "NeoPixelWheel.h"
#include "SystemStatus.h"

class StatusWheel
{
private:
  NeoPixelWheel _wheel;  
  SystemStatus &_system;
  void (*_logMsg)(const char *);

  uint32_t mapServerColor(SystemStatus::ServerStatus stat );
  uint32_t mapBuildColor(SystemStatus::BuildStatus stat );

  static const int DATA_PIN = 6;
  static const int PIXEL_COUNT = 24;

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
    
public:  
  StatusWheel(SystemStatus &systemStatus, void (*logMsg)(const char *) ) :  _logMsg( logMsg ),  _system(systemStatus), _wheel(PIXEL_COUNT, DATA_PIN)
  {}
                                                              
  bool initialize();

  bool process(bool newValues);    
};


#endif
