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

  #define PIN 6

  uint32_t _red;
  uint32_t _green;  
  uint32_t _blue;  
  uint32_t _yellow;  
  uint32_t _orange;
    
public:  
  StatusWheel(SystemStatus &systemStatus, void (*logMsg)(const char *) ) :  _logMsg( logMsg ),  _system(systemStatus), _wheel(24, PIN)
  {}
                                                              
  bool initialize();

  bool process();    
};


#endif
