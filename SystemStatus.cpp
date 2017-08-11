#include "SystemStatus.h"

#include <ArduinoJson.h>
#include <time.h>

#include "my_keys.h"

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

#define WAIT_TIMEOUT_MS 10000

bool SystemStatus::initialize()
{
#ifdef ARDUINO_SAMD_FEATHER_M0
  //Configure pins for Adafruit ATWINC1500 Feather
  WiFi.setPins(8, 7, 4, 2);
#endif

  // check for the presence of the shield:
#ifndef ESP_PLATFORM
  if (WiFi.status() == WL_NO_SHIELD)
  {
    logMsg("WiFi shield not present");
    return false;
  }
#endif

  // attempt to connect to Wifi network:
  int attempt = 1;
  while (WiFi.status() != WL_CONNECTED)
  {
    _locationIndex ^= 1;
    logMsg("Try %d connecting to SSID: %s %s", attempt++, Locations[_locationIndex].SSID,Locations[_locationIndex].Password);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    WiFi.begin(Locations[_locationIndex].SSID, Locations[_locationIndex].Password);

    ulong start = millis();
    while ( WiFi.status() != WL_CONNECTED && (millis() - start) < WAIT_TIMEOUT_MS)
    {
      // wait for connection:
      Serial.print(WiFi.status());
      Serial.print(".");
      delay(500);
    }
    Serial.println("");
  }
  logMsg("Connected to wifi");
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
#ifdef ARDUINO_SAMD_FEATHER_M0
    ok = _client.connectSSL(server,port);
#endif
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

void SystemStatus::checkStatus()
{
  for ( int i = 0; i < STATUS_COUNT; i++ )
  {
    BuildStatuses[i] = SystemStatus::BuildStatus::BuildUnknown;
    ServerStatuses[i] = SystemStatus::ServerStatus::Unknown;
  }

  // http://localhost:5000/api/status?count=1

  sprintf( _sprintfBuffer, Locations[_locationIndex].ServerStatusPath );

  char *output;
  int i = getWebPage( output, Locations[_locationIndex].ServerStatusAddress, _sprintfBuffer, NULL, Locations[_locationIndex].ServerStatusPort);
  Serial.print("got serverStatus bytes count: ");
  Serial.println(i);
  if ( i <= 0 )
    return;

  // from https://bblanchon.github.io/ArduinoJson/assistant/
  const size_t bufferSize = 2*JSON_ARRAY_SIZE(40) + JSON_OBJECT_SIZE(2) + 1188;  // 1188 was addl bytes for ESP8266
  DynamicJsonBuffer jsonBuffer(bufferSize);

  output = strchr( output, '{' );
  if ( output != NULL )
  {
    JsonObject& root = jsonBuffer.parseObject(output);
    if ( root.success() )
    {
      logMsg("Got Result, Ctm len %d Zabbix len %d", root["item1"].size(), root["item2"].size());
      JsonArray &item1 = root["item1"];
      for ( int i = 0; i < item1.size() && i < STATUS_COUNT; i++ )
      {
        int severity = item1[i];
        logMsg( "Ctm %d", severity);
        BuildStatuses[i] = (SystemStatus::BuildStatus)severity;
      }
      for ( int j = i; i < item1.size() && i < STATUS_COUNT; i++ )
      {
        BuildStatuses[i] = SystemStatus::BuildStatus::BuildUnknown;
      }
      JsonArray &item2 = root["item2"];
      for ( int i = 0; i < item2.size() && i < STATUS_COUNT; i++ )
      {
        int priority = item2[i]["priority"];
        logMsg( "Zabbix %d", priority);
        ServerStatuses[i] = (SystemStatus::ServerStatus)priority;
      }
      for ( int j = i; i < item1.size() && i < STATUS_COUNT; i++ )
      {
        ServerStatuses[i] = SystemStatus::ServerStatus::Unknown;
      }
    }
    else
    {
      logMsg( "Failed to parse JSON: %s", output );
    }
  }
  else
  {
      logMsg( "Didn't find open brace in output: %s", output );
  }
}



