
#ifndef COUNTSERVER_API_H
#define COUNTSERVER_API_H

class ICountServerStream
{
public:
    virtual ~ICountServerStream() {};
    
    virtual int ConnectTo( char const* apcIpAddress, unsigned short ai16Port, int aiTimeout ) = 0;
    
    virtual int RequestConnToken( char const* apcKey, int aiNum, int aiTimeout ) = 0;
    
    virtual int RequestSpeedToken( char const* apcKey, int aiNum, int aiTimeout ) = 0;
};

class CCountServerApi
{
public:
    CCountServerApi();
    
    ~CCountServerApi();
    
    int Initialize( const char* apcIpAddr, unsigned short ai16Port, int aiTimeout );
    
    int Close(void);
    
    int AcquireConnToken( char const* apcKey, size_t auNum, int aiTimeout );
    
    int ReleaseConnToken( char const* apcKey, size_t auNum, int aiTimeout );
    
    int AcquireSpeedToken( char const* apcKey, size_t auNum, int aiTimeout );

protected:
    ICountServerStream* mpoImpl;
};

#endif// COUNTSERVER_API_H
