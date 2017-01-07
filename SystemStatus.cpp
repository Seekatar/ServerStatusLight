#include "SystemStatus.h"

#include <ArduinoJson.h>
#include <time.h>

#include "my_keys.h"
char ssid[] = WLAN_SSID; //  your network SSID (name)
char pass[] = WLAN_PASS; // your network password (use for WPA, or use as key for WEP)

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

int SystemStatus::getWebPage( char *&output, const char *server, const char *path, const char * headers, int port )
{
  return postWebPage( output, server, path, headers, port );
}

int SystemStatus::getWebPage( char *&output, IPAddress server, const char *path, const char * headers, int port )
{
  return postWebPage( output, server, path, headers, port );
}

int SystemStatus::postWebPage( char *&output, IPAddress server, const char *path, const char * headers, int port, bool getMethod, const char *body )
{
  Serial.print("Connecting to IP: ");
  Serial.print(server);
  Serial.print(":");
  Serial.print(port);
  Serial.println(path);

  // if you get a connection, report back via serial:
  if (_client.connect(server, port))
  {
  Serial.println( "...connected111");    
    return getWebContent( output, "localhost", path, headers, port, getMethod, body );
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
    _client.println(server);
  }
  if ( headers != NULL )
    _client.println(headers);
  _client.println("Connection:close");
  _client.println();
  if ( body != NULL && !getMethod )
    _client.println(body);
  logMsg("Getting content...");

  do
  {
    while (_client.available())
    {
      char c = _client.read();
      if ( i < BUFFER_LEN - 1 )
      {
        _buffer[i] = c;
        i++;
      }
    }
  }
  while (_client.connected());
  
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
  // http://www.timeapi.org/utc/2+days+ago?format=%25m-%25d-%25Y
  if ( _twoDaysAgo[0] == '\0' )
  {
    
//GET /utc/2+days+ago?format=%25m-%25d-%25Y HTTP/1.1
//>> not neededAccept:text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8
//Host:www.timeapi.org
//User-Agent:Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/55.0.2883.75 Safari/537.36
//Connection:close

    char * headers = "User-Agent:Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/55.0.2883.75 Safari/537.36";

    char *output;
    if ( getWebPage( output, "www.timeapi.org", "/utc/3+days+ago?format=%25m-%25d-%25Y", headers ) > 10 )
    // if ( getWebPage( "www.timeapi.org", "/utc" ) > 10 )
    {
      strncpy( _twoDaysAgo, output, sizeof(_twoDaysAgo));
      logMsg( "Got time %s!", output );

      int m, d, y;
      
      sscanf( output, "%d-%d-%d", &m, &d, &y );
      struct tm time_info;
      time_t time_raw_format;
  
      time_info.tm_year = y-1900;
      time_info.tm_mon = m-1;
      time_info.tm_mday = d;
      time_info.tm_hour = 0;
      time_info.tm_min = 0;
      time_info.tm_sec = 0;
      time_info.tm_isdst = 0;
  
      _timet2DaysAgo = (unsigned int)mktime(&time_info);
      logMsg( "Got timet %d!", _timet2DaysAgo );
    }
    else
    {
      logMsg("Didn't get time!");
      Serial.println(output);
    }
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

  getTwoDaysAgo();

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
      index++;
    }
    line = strtok( NULL, "\r\n" );
  }
  for ( int i = index; i < STATUS_COUNT; i++ )
  {
    BuildStatuses[i] = BuildStatus::BuildUnknown;
  }
  logMsg( "G=%d  B=%d  P=%d S=%d", good, bad, progress, staged );
}

typedef struct trigger_struct {
  short triggerid;
  char priority;
};

trigger_struct triggers[600];

SystemStatus::ServerStatus SystemStatus::mapZabbixStatus(short objectid, bool recovered )
{
  if ( recovered )
    return ServerStatus::Green;
    
  ServerStatus ret = ServerStatus::Unknown;
  // find it in triggers
  char priority = '0';
  for ( int i = 0; i < 600; i++ )
  {
    if ( triggers[i].triggerid == objectid )
    {
      priority = triggers[i].priority;
      break;
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
  //#define EVENTS 13 // add one since fail some times
  //const int BUFFER_SIZE = JSON_ARRAY_SIZE(EVENTS) + JSON_OBJECT_SIZE(3) + EVENTS*JSON_OBJECT_SIZE(4);
  #define TRIGGERS 100 // 600 crashes it when client tries to connect!
  const int BUFFER_SIZE = JSON_ARRAY_SIZE(TRIGGERS) + JSON_OBJECT_SIZE(3) + TRIGGERS*JSON_OBJECT_SIZE(2);
  
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
    Serial.println( _sprintfBuffer);
    int i = postWebPage( output, ZABBIX_SERVER, ZABBIX_LOGIN, NULL, ZABBIX_PORT, ZABBIX_GET, _sprintfBuffer );
    logMsg("Done calling.  Got %d bytes", i);
    if ( i > 0  )
    {
      logMsg("About to parse\n%s", output);
      JsonObject& root = jsonBuffer.parseObject(output);
      if ( root.success() )
      {
        strncpy(_sessionId, root["result"],sizeof(_sessionId));
        logMsg("Parse OK\nsessionId is %s", _sessionId);
      
        // cache triggers
        sprintf( _sprintfBuffer, "{\
            \"jsonrpc\": \"2.0\",\
            \"method\": \"trigger.get\",\
            \"params\": {\
              \"output\":[\"triggerid\",\"priority\"],\
              \"limit\":600\
            },\
            \"id\": 2,\
            \"auth\": \"%s\"\
          }", _sessionId );
          
        i = postWebPage( output, ZABBIX_SERVER, ZABBIX_TRIGGERS, "Content-Type:application/json", ZABBIX_PORT, ZABBIX_GET, _sprintfBuffer );
        logMsg("Done calling.  Got %d bytes", i);
        if ( i > 0  )
        {
          logMsg("About to parse\n%s", output);
          JsonObject& root = jsonBuffer.parseObject(output);
          if ( root.success() )
          {
            logMsg("Parse OK\nLength of triggers is %d", root["result"].size());
            for ( int i = 0; i < root["result"].size(); i++ )
            {
              triggers[i].triggerid = (short)atoi(root["result"][i]["triggerid"]);
              triggers[i].priority = ((const char*)root["result"][i]["priority"])[0];
            }
          }
        }
      }
    }
  }

  // get status
  sprintf( _sprintfBuffer, "{\
      \"jsonrpc\": \"2.0\",\
      \"method\": \"event.get\",\
      \"params\": {\
            \"output\": [\"objectid\",\"r_eventid\"],\
            \"time_from\":%d,\
            \"sortfield\":\"clock\",\
            \"sortorder\":\"DESC\",\
            \"value\":1,\
            \"limit\": 12\
      },\
      \"id\": 3,\
      \"auth\": \"%s\"\
    }", _timet2DaysAgo, _sessionId );
      
  int i = postWebPage( output, ZABBIX_SERVER, ZABBIX_EVENTS, "Content-Type:application/json", ZABBIX_PORT, ZABBIX_GET, _sprintfBuffer );
  logMsg("Done calling.  Got %d bytes", i);
  if ( i > 0  )
  {
    logMsg("About to parse\n%s", output);
    JsonObject& root = jsonBuffer.parseObject(output);
    if ( root.success() )
    {
      logMsg("Parse OK\nLength of events is %d", root["result"].size());
      for ( int i = 0; i < root["result"].size(); i++ )
      {
          ServerStatuses[i] = mapZabbixStatus(root["result"][i]["objectid"],strlen(root["result"][i]["r_eventId"]) > 0);
      }
      for ( int j = i; j < STATUS_COUNT; j++ )
      {
        ServerStatuses[j] = ServerStatus::Unknown;
      }
      
    }
  }
  
}

void SystemStatus::checkNewRelicServers()
{
  char * pages[] = { NEW_RELIC_PATHS, NULL };

  int p = 0;
  int green = 0;
  int orange = 0;
  int red = 0;

  int index = 0;
  char *output;
  while ( pages[p] != NULL )
  {
    sprintf( _sprintfBuffer, "X-Api-Key:%s", NEW_RELIC_KEY );
    int i = getWebPage( output, NEW_RELIC_SERVER, pages[p++], _sprintfBuffer, NEWRELIC_PORT );

    logMsg("Done calling.  Got %d bytes", i);
    if ( i <= 0 )
      return;

    Serial.println( "Output is ");
    Serial.println( output );
    
    // used the calculator at https://bblanchon.github.io/ArduinoJson/ to get this for  5 LCHOSTS
    #define SERVERS 6 // add one since fail some times
    const int BUFFER_SIZE = JSON_ARRAY_SIZE(SERVERS) + 6 * JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + SERVERS * JSON_OBJECT_SIZE(8) + SERVERS * JSON_OBJECT_SIZE(9);
    StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;

    JsonObject& root = jsonBuffer.parseObject(output);
    if ( root.success() )
    {
      logMsg("Parse OK\nLength of servers is %d", root["servers"].size());

      for ( int i = 0; i < root["servers"].size(); i++ )
      {
        if ( strncmp(root["servers"][i]["health_status"], "red", 3 ) == 0 )
        {
          ServerStatuses[index] = ServerStatus::Red;
          red++;
          index++;
        }
        else if ( strncmp(root["servers"][i]["health_status"], "green", 5 ) == 0 )
        {
          ServerStatuses[index] = ServerStatus::Green;
          green++;
          index++;
        }
        else if ( strncmp(root["servers"][i]["health_status"], "orange", 5  ) == 0 )
        {
          ServerStatuses[index] = ServerStatus::Orange;
          orange++;
          index++;
        }

      }

    }
    else
    {
      logMsg("Parse of JSON FAILED!");
      Serial.println(output);
      return;
    }
  }
  for ( int i = index; i < STATUS_COUNT; i++ )
  {
    ServerStatuses[i] = ServerStatus::Unknown;
  }

  logMsg( "R=%d  O=%d  G=%d", red, orange, green);
}
