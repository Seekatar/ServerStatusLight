#include "SystemStatus.h"

#include <ArduinoJson.h>

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

int SystemStatus::getWebPage( char *&output, const char *server, const char *path, const char * headers, int port )
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
    return getWebContent( output, server, path, headers, port );
  }
  {
    Serial.println( "Failed to connect" );
    return -1;
  }
}

int SystemStatus::getWebPage( char *&output, IPAddress server, const char *path, const char * headers, int port )
{
  Serial.print("Connecting to IP: ");
  Serial.print(server);
  Serial.print(":");
  Serial.print(port);
  Serial.println(path);

  // if you get a connection, report back via serial:
  if (_client.connect(server, port))
  {
    return getWebContent( output, "localhost", path, headers, port );
  }
  else
  {
    Serial.println( "Failed to connect" );
    return -1;
  }
}

int SystemStatus::getWebContent( char *&output, const char *server, const char *path, const char * headers, int port )
{
  int i = 0;

  Serial.println( "...connected");
  // Make a HTTP request:
  _client.print("GET ");
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
  while (_client.connected())
    ;
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

void SystemStatus::checkServers()
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
