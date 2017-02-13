#include "SystemStatus.h"

#include <ArduinoJson.h>
#include <time.h>

#include "my_keys.h"
char ssid[] = WLAN_SSID; //  your network SSID (name)
char pass[] = WLAN_PASS; // your network password (use for WPA, or use as key for WEP)

// #define DUMP_CONTENT
// #define ZABBIX_DEBUG 1

int status = WL_IDLE_STATUS;

void SystemStatus::printWifiStatus()
{
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

bool SystemStatus::initialize()
{
  //Configure pins for Adafruit ATWINC1500 Feather
  WiFi.setPins(8, 7, 4, 2);

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD)
  {
    logMsg("WiFi shield not present");
    return false;
  }

  // attempt to connect to Wifi network:
  int i = 1;
  while (status != WL_CONNECTED)
  {
    logMsg("Try %d connecting to SSID: %s", i++, ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(2000);
  }
  Serial.println("Connected to wifi");
  printWifiStatus();

  return true;
}

int SystemStatus::getWebPage( char *&output, const char *server, const char *path, const char * headers, int port )
{
  return postWebPage( output, server, path, headers, port );
}

int SystemStatus::getWebPage( char *&output, IPAddress server, const char *path, const char * headers, int port )
{
  return postWebPage( output, server, path, headers, port );
}

int SystemStatus::postWebPage( char *&output, const char *server, const char *path, const char * headers, int port, bool getMethod, const char *body )
{
  Serial.print("Connecting to: ");
  Serial.print(server);
  Serial.print(":");
  Serial.print(port);
  Serial.println(path);

  bool ok = false;
  if ( port == 443 )
  {
    Serial.println( "Using SSL" );
    ok = _client.connectSSL(server,port);
  }
  else
    ok = _client.connect(server, port);

  if ( ok )
  {
    return getWebContent( output, server, path, headers, port, getMethod, body );
  }
  {
    Serial.println( "Failed to connect" );
    return -1;
  }
}

int SystemStatus::postWebPage( char *&output, IPAddress server, const char *path, const char * headers, int port, bool getMethod, const char *body )
{
  char serverName[25];
  snprintf( serverName, sizeof(serverName), "%d.%d.%d.%d", server[0], server[1], server[2], server[3] );
  Serial.print("Connecting to IP: ");
  Serial.print(serverName);
  Serial.print(":");
  Serial.print(port);
  Serial.println(path);

  if (_client.connect(server, port))
  {
    return getWebContent( output, serverName, path, headers, port, getMethod, body );
  }
  else
  {
    Serial.println( "Failed to connect" );
    return -1;
  }
}

int SystemStatus::getWebContent( char *&output, const char *server, const char *path, const char * headers, int port, bool getMethod, const char *body )
{
  int i = 0;

  Serial.println( "...connected");
  
  // Make a HTTP request:
  if ( getMethod )
    _client.print("GET ");
  else
    _client.print("POST ");
  _client.print(path);
  _client.println(" HTTP/1.1");

  if ( server != NULL )
  {
    _client.print("Host:");
    _client.print(server);
    _client.print(":");
    _client.println(port);
  }
  if ( headers != NULL )
    _client.println(headers);
  if ( body != NULL && !getMethod )
  {
    char s[100];
    sprintf( s, "Content-Length: %d", strlen(body) );
    _client.println(s);
    _client.println("Accept: */*"); // probably not needed
  }
  _client.println("Connection:close");
  _client.println();
  if ( body != NULL && !getMethod )
  {
    #ifdef DUMP_CONTENT
    logMsg("POSTING:");
    logMsg(body);
    #endif
    _client.println(body);
  }  
  logMsg("Getting content...");

  do
  {
    while (_client.available())
    {
      char c = _client.read();
      #ifdef DUMP_CONTENT
      Serial.print(c);
      #endif
      if ( i < BUFFER_LEN - 1 )
      {
        _buffer[i] = c;
        i++;
      }
    }
  }
  while (_client.connected());
  #ifdef DUMP_CONTENT
  Serial.println("");
  #endif DUMP_CONTENT
  
  _client.stop();
  _buffer[i] = '\0';

  logMsg("Got %d bytes", i);

  // get the data from the request
  if ( strncmp( _buffer, "HTTP/1.1 200", strlen("HTTP/1.1 200")) == 0 || strncmp( _buffer, "HTTP/1.1 304", strlen("HTTP/1.1 304")) == 0 )
  {
    char *data = strstr( _buffer, "\r\n\r\n" );
    if ( data == NULL )
    {
      data = strstr( _buffer, "\n\n" );
      if ( data == NULL )
      {
        data = strstr( _buffer, "\r\r" );
      }
      if ( data != NULL )
        data += 2;
    }
    else
    {
      data += 4;
    }
    output = data;
  }
  else
  {
    logMsg( "Didn't get 200, attach serial to see details" );
    Serial.println(_buffer);
    i = -1;
  }

  return i;
}

void SystemStatus::getTwoDaysAgo()
{
  // find the timestamp
  // Date →Sun, 12 Feb 2017 20:35:21 GMT
  char *data = strstr( _buffer, "Date: " );

  if ( data != NULL )
  {
    struct tm result;
    strptime( data + strlen("Date: "), "%a, %d %b %Y %H:%M:%S %Z", &result );
    logMsg( "Found current time of %d/%d/%d %d:%d:%d", result.tm_mon+1, result.tm_mday, result.tm_year+1900, 
                      result.tm_hour, result.tm_min, result.tm_sec );
    time_t t = mktime( &result );
    t -= 24*60*60*2; // two days
    _timet2DaysAgo = t;
    logMsg( "Got time_t %d!", _timet2DaysAgo );
  }
  else
  {
    logMsg( "No Date found in headers %.200s", _buffer );
  }
}

void SystemStatus::checkBuilds()
{
  // http://continuum/api/list_pipelineinstances?token=xxxxxxxxxxxxxx&since=12-1-2016&output_format=text&header=false
  // 584b081b7c1a7c1d91bd2bfd 17.0.0.1094 (HV3C3) Build, Deploy and Test Lifecycle  Lifecycle PR #2118  success

  int good = 0;
  int bad = 0;
  int progress = 0;
  int staged = 0;
  int canceled = 0;

  sprintf( _sprintfBuffer, CONTINUUM_PATH, CONTINUUM_KEY, _twoDaysAgo );

  char *output;
  int i = getWebPage( output, CONTINUUM_SERVER, _sprintfBuffer, NULL, CONTINUUM_PORT );
  Serial.print("got continuum bytes: ");
  Serial.println(i);
  if ( i <= 0 )
    return;

  char *line = strtok( output, "\r\n" );
  int index = 0;
  while ( line != NULL && index < STATUS_COUNT )
  {
    char *last = strrchr( line, '\t');
    if ( last != NULL )
    {
      ++last;
      if ( strncmp( last, "success", strlen("success") ) == 0 )
      {
        good++;
        BuildStatuses[index] = BuildStatus::Success;
      }
      else if ( strncmp( last, "failure", strlen("failure") ) == 0 )
      {
        bad++;
        BuildStatuses[index] = BuildStatus::Failure;
      }
      else if ( strncmp( last, "processing", strlen("processing") ) == 0 )
      {
        progress++;
        BuildStatuses[index] = BuildStatus::Processing;
      }
      else if ( strncmp( last, "staged", strlen("staged") ) == 0 )
      {
        staged++;
        BuildStatuses[index] = BuildStatus::Staged;
      }
      else if ( strncmp( last, "canceled", strlen("canceled") ) == 0 )
      {
        canceled++;
        BuildStatuses[index] = BuildStatus::Canceled;
      }
      else
      {
        BuildStatuses[index] = BuildStatus::BuildUnknown;
      }
      index++;
    }
    line = strtok( NULL, "\r\n" );
  }
  for ( int i = index; i < STATUS_COUNT; i++ )
  {
    BuildStatuses[i] = BuildStatus::BuildUnknown;
  }
  logMsg( "G=%d  B=%d  P=%d S=%d C=%d", good, bad, progress, staged, canceled );
}

SystemStatus::ServerStatus SystemStatus::mapZabbixStatus(short objectid, bool recovered )
{
  #if ZABBIX_DEBUG
  Serial.print(" Objectid: ");
  Serial.print(objectid);
  Serial.print(" recovered: ");
  Serial.println(recovered);
  #endif
  
  if ( recovered )
  {
    #if ZABBIX_DEBUG
    Serial.println("recovered!!");
    #endif
    return ServerStatus::Green;
  }

  ServerStatus ret = ServerStatus::Unknown;
  
  // find it in triggers
  char priority = ' ';
  for ( int i = 0; i < MAX_TRIGGERS; i++ )
  {
    if ( _triggers[i].triggerid == objectid )
    {
      priority = _triggers[i].priority;
      #if ZABBIX_DEBUG
      Serial.println( "Found trigger!");
      #endif
      break;
    }
  }

  if ( priority == ' ' )
  {
      #define TRIGGERS 30 // 600 crashes it when client tries to connect!
      const int BUFFER_SIZE = JSON_ARRAY_SIZE(TRIGGERS) + JSON_OBJECT_SIZE(3) + TRIGGERS*JSON_OBJECT_SIZE(2);

      StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;

    // cache triggers
    sprintf( _sprintfBuffer, "{\
        \"jsonrpc\": \"2.0\",\
        \"method\": \"trigger.get\",\
        \"params\": {\
          \"output\":[\"triggerid\",\"priority\"],\
          \"triggerids\":%d\
        },\
        \"id\": 2,\
        \"auth\": \"%s\"\
      }", objectid, _sessionId );
    Serial.println( _sprintfBuffer );

    char *output;
    int i = postWebPage( output, ZABBIX_SERVER, ZABBIX_TRIGGERS, "Content-Type:application/json", ZABBIX_PORT, ZABBIX_GET, _sprintfBuffer );
    if ( i > 0  )
    {
      JsonObject& root = jsonBuffer.parseObject(output);
      if ( root.success() )
      {
        logMsg("Trigger OK\nLength of triggers is %d", root["result"].size());
        for ( int i = 0; i < root["result"].size(); i++ )
        {
          short id = (short)atoi(root["result"][i]["triggerid"]);

          // already have it?
          int j;
          for ( j = 0; j < MAX_TRIGGERS && _triggers[j].triggerid != 0; j++ )
          {
            if ( _triggers[j].triggerid == id )
            {
              #if ZABBIX_DEBUG
              Serial.print("Already have trigger id ");
              Serial.println(id);
              #endif
              priority = _triggers[j].priority;
              break;
            }
          }
          if ( _triggers[j].triggerid == 0 )
          {
            #if ZABBIX_DEBUG
            Serial.print("Added trigger ");
            Serial.print(j);
            Serial.print(" id of ");
            Serial.println(id);
            #endif
            _triggers[j].triggerid = id;
            _triggers[j].priority = ((const char*)root["result"][i]["priority"])[0];
            if ( id == objectid )
            {
              #if ZABBIX_DEBUG
              Serial.println("Matched newly added one");
              #endif
              priority = _triggers[j].priority;
            }
           }
        }
      }
      else
      {
        logMsg( "Trigger failed: %s", output );
      }
    }
  }

  switch ( priority )
  {
    case '1' :
      ret = ServerStatus::Blue;
      break;
    case '2' :
      ret = ServerStatus::Yellow;
      break;
    case '3' :
      ret = ServerStatus::Orange;
      break;
    case '4' :
      ret = ServerStatus::Red;
      break;
    case '5' :
      ret = ServerStatus::BrightRed;
      break;
  }
  return ret;
}

void SystemStatus::checkZabbixServers()
{
  #define TRIGGERS 30 // 600 crashes it when client tries to connect!
  const int BUFFER_SIZE = JSON_ARRAY_SIZE(TRIGGERS) + JSON_OBJECT_SIZE(3) + TRIGGERS*JSON_OBJECT_SIZE(2);
  Serial.print("Buffer size is ");
  Serial.println(BUFFER_SIZE);

  StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;

  char *output;

  // login
  if ( strlen(_sessionId) == 0 )
  {
    sprintf( _sprintfBuffer, "{\
        \"jsonrpc\": \"2.0\",\
        \"method\": \"user.login\",\
        \"params\": {\
            \"user\": \"%s\",\
            \"password\": \"%s\"\
        },\
        \"id\": 1,\
        \"auth\": null\
    }", ZABBIX_USER, ZABBIX_PASSWORD );
    int i = postWebPage( output, ZABBIX_SERVER, ZABBIX_LOGIN, "Content-Type:application/json", ZABBIX_PORT, ZABBIX_GET, _sprintfBuffer );

    getTwoDaysAgo();

    if ( i > 0  )
    {
      JsonObject& root = jsonBuffer.parseObject(output);
      if ( root.success() )
      {
        strncpy(_sessionId, root["result"],sizeof(_sessionId));
        logMsg("Login OK\nsessionId is %s", _sessionId);
      }
      else
      {
        logMsg( "Login failed: %s", output );
      }
    }
  }

  // get status
  sprintf( _sprintfBuffer, "{\
      \"jsonrpc\": \"2.0\",\
      \"method\": \"event.get\",\
      \"params\": {\
            \"output\": [\"objectid\",\"r_eventid\"],\
            \"sortfield\":\"clock\",\
            \"sortorder\":\"DESC\",\
            \"value\":1,\
            \"limit\":12\
      },\
      \"id\": 3,\
      \"auth\": \"%s\"\
    }", _sessionId );

  int i = postWebPage( output, ZABBIX_SERVER, ZABBIX_EVENTS, "Content-Type:application/json", ZABBIX_PORT, ZABBIX_GET, _sprintfBuffer );
  if ( i > 0  )
  {
    strcpy( _buffer2, output );
    JsonObject& root = jsonBuffer.parseObject(_buffer2);
    if ( root.success() )
    {
      logMsg("Event OK\nLength of events is %d", root["result"].size());
      for ( int i = 0; i < root["result"].size(); i++ )
      {
        _events[i].eventid = (short)atoi(root["result"][i]["objectid"]);
        _events[i].recovered = *((const char *)(root["result"][i]["r_eventid"])) != '0';
      }
      for ( int j = i; j < STATUS_COUNT; j++ )
      {
        _events[j].eventid = 0;
        ServerStatuses[j] = ServerStatus::Unknown;
      }
      for ( int i = 0; i < MAX_EVENTS && _events[i].eventid != 0; i++ )
      {
        ServerStatuses[i] = mapZabbixStatus(_events[i].eventid,_events[i].recovered);
      }
    }
    else
    {
      logMsg( "Event failed: %s", output );
    }
  }

}

