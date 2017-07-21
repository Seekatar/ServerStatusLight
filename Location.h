#ifndef __LOCATION_H__
#define __LOCATION_H__

class Location
{
public:
    Location( const char *ssid, const char *password,
        IPAddress serverStatus, const char * serverStatusPath, int serverStatusPort ) :
        ServerStatusAddress(serverStatus)
    {
        SSID = ssid;
        Password = password;
        ServerStatusPort = serverStatusPort;
        ServerStatusPath = serverStatusPath;
    }
    const char * Password;
    const char * SSID;
    const char * ServerStatusPath;
    IPAddress ServerStatusAddress;
    int ServerStatusPort;
};

#endif
