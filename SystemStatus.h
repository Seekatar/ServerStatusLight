#ifndef __SYSTEMSTATUS_H__
#define __SYSTEMSTATUS_H__

#define CONF_WINC_DEBUG 1
#include <SPI.h>
#include <WiFi101.h>

class SystemStatus 
{
public:
    enum ServerStatus 
    {
      Unknown,
      Blue,
      Green,
      Yellow,
      Orange,
      Red,
      BrightRed
    };

    enum BuildStatus
    {
      BuildUnknown,
      Staged,
      Processing,
      Success,
      Failure
    };

private:
  unsigned long _lastCheck = 0;
  WiFiClient _client;

  const static int BUFFER_LEN = 5000;
  char _buffer[BUFFER_LEN];
  char _sprintfBuffer[500];
  char _twoDaysAgo[12];
  unsigned int _timet2DaysAgo;

  
  char _sessionId[40];
  char _triggers[600];
  void checkNewRelicServers();
  void checkZabbixServers();
  void checkBuilds();
  void (*logMsg)(const char *, ...);
  void printWifiStatus();
  void getTwoDaysAgo();
  int getWebContent( char *&output, const char *server, const char *path, const char * headers, int port, bool getMethod, const char *body );
  int getWebPage( char *&output, IPAddress server, const char *path, const char * headers = NULL, int port = 80);
  int getWebPage( char *&output, const char *server, const char *path, const char * headers = NULL, int port = 80);
  int postWebPage( char *&output, const char *server, const char *path, const char * headers, int port, bool getMethod = true, const char *body = NULL );
  int postWebPage( char *&output, IPAddress server, const char *path, const char * headers, int port, bool getMethod = true, const char *body = NULL );
  SystemStatus::ServerStatus mapZabbixStatus(short objectId, char reventid );

  
public:  
      unsigned long CHECK_THRESHOLD = 10000;
      
      SystemStatus(void (*logMsg)(const char *,...) ) : logMsg( logMsg )
      {
         _twoDaysAgo[0] = '\0';
         _sessionId[0] = '\0';
      }
                                                                  
      bool initialize();

      const static int STATUS_COUNT = 12;
      ServerStatus ServerStatuses[STATUS_COUNT];
      BuildStatus BuildStatuses[STATUS_COUNT];
      
      inline bool process()
      {
        unsigned long now = millis();
        
        if ( now - _lastCheck > CHECK_THRESHOLD )
        {
          // checkBuilds();
          checkZabbixServers();
          _lastCheck = now;
          return true;
        }
        return false;
      }
};


#endif
