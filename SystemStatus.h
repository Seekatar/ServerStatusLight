#ifndef __SYSTEMSTATUS_H__
#define __SYSTEMSTATUS_H__

#define CONF_WINC_DEBUG 1
#include <SPI.h>
#include <WiFi101.h>

class SystemStatus 
{
private:
  unsigned long _lastCheck = 0;
  WiFiClient _client;

  const static int BUFFER_LEN = 5000;
  char _buffer[BUFFER_LEN];
  char _sprintfBuffer[200];
  char _twoDaysAgo[12];
  
  void checkServers();
  void checkBuilds();
  void (*_logMsg)(const char *);
  void printWifiStatus();
  void getTwoDaysAgo();
  int getWebContent( char *&output, const char *server, const char *path, const char * headers, int port );
  int getWebPage( char *&output, IPAddress server, const char *path, const char * headers = NULL, int port = 80);
  int getWebPage( char *&output, const char *server, const char *path, const char * headers = NULL, int port = 80);
  
public:  
      unsigned long CHECK_THRESHOLD = 10000;
      
      SystemStatus(void (*logMsg)(const char *) ) : _logMsg( logMsg )
      {
         _twoDaysAgo[0] = '\0';
      }
                                                                  
      bool initialize();

      enum ServerStatus 
      {
        Unknown,
        Green,
        Orange,
        Red
      };

      enum BuildStatus
      {
        BuildUnknown,
        Staged,
        Processing,
        Success,
        Failure
      };

      const static int STATUS_COUNT = 12;
      ServerStatus ServerStatuses[STATUS_COUNT];
      BuildStatus BuildStatuses[STATUS_COUNT];
      
      inline bool process()
      {
        unsigned long now = millis();
        
        if ( now - _lastCheck > CHECK_THRESHOLD )
        {
          checkServers();
          checkBuilds();
          _lastCheck = now;
          return true;
        }
        return false;
      }
};


#endif
