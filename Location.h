#ifndef __LOCATION_H__
#define __LOCATION_H__

class Location
{
public:
    Location( const char *ssid, const char *password,
        IPAddress v1Status, const char * v1StatusPath, int v1StatusPort ) :
        V1StatusAddress(v1Status)
    {
        SSID = ssid;
        Password = password;
        V1StatusPort = v1StatusPort;
        V1StatusPath = v1StatusPath;
    }
    const char * Password;
    const char * SSID;
    const char * V1StatusPath;
    IPAddress V1StatusAddress;
    int V1StatusPort;
};

#endif
