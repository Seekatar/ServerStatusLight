#ifndef __LOCATION_H__
#define __LOCATION_H__

class Location
{
public:
    Location( const char *ssid, const char *password, 
        IPAddress continuum, const char * continuumPath, int continuumPort, 
        IPAddress zabbix ) :
        Continuum(continuum), Zabbix(zabbix)
    {
        SSID = ssid;
        Password = password;
        ContinuumPort = continuumPort;
        ContinuumPath = continuumPath;
    }
    const char * Password;
    const char * SSID;
    const char * ContinuumPath;
    IPAddress Continuum;
    IPAddress Zabbix;
    int ContinuumPort;
};

#endif
