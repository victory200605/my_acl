
#ifndef __SCP_PROTOCOL_H__
#define __SCP_PROTOCOL_H__

#include "AIScpProtocol.h"
#include "../../AIProxyProtocol.h"
#include "../../AIProxyUtility.h"

class clsScpProtocol
{
public:
    static int Decode( const char* apcData, size_t aiSize, AIProxy::AIProtocol::THeader& aoHeader );
    static int Encode( const AIProxy::AIProtocol::THeader& aoHeader, AIChunkEx& aoBuffer );
    
    static int Send( AISocketHandle aoHandle, const char* apcData, size_t aiSize );
    static int Recv( AISocketHandle aoHandle, AIChunkEx& aoBuffer, int aiTimeOut );
    
    static bool IsHandshake( const char* apcData, size_t aiSize );
};

//////////////////////////////////// SCP-InnerProtocol //////////////////////////////////////////
class clsScpInnerProtocol : public AIProxy::AIInnerProtocol
{
public:
    virtual int Authenticate( AISocketHandle aoHandle, char* apcName, size_t aiSize );
    
    virtual int Decode( const char* apcData, size_t aiSize, THeader& aoHeader );
    virtual int Encode( const THeader& aoHeader, AIChunkEx& aoBuffer );
    
    virtual int Send( AISocketHandle aoHandle, const char* apcData, size_t aiSize );
    virtual int Recv( AISocketHandle aoHandle, AIChunkEx& aoBuffer, int aiTimeOut );
    
protected:
    int HandleLogin( AISocketHandle aoHandle );
    int HandleAuth( AISocketHandle aoHandle );
};

//////////////////////////////////// SCP-OuterProtocol //////////////////////////////////////////
class clsScpOuterProtocol : public AIProxy::AIOuterProtocol
{
public:
    clsScpOuterProtocol();
    virtual ~clsScpOuterProtocol();
    
    virtual int Authenticate( AISocketHandle aoHandle, const char* apcName );
    
    virtual int Decode( const char* apcData, size_t aiSize, THeader& aoHeader );
    virtual int Encode( const THeader& aoHeader, AIChunkEx& aoBuffer );
    
    virtual int Send( AISocketHandle aoHandle, const char* apcData, size_t aiSize );
    virtual int Recv( AISocketHandle aoHandle, AIChunkEx& aoBuffer, int aiTimeOut );
    
public:
    int Login( 
        AISocketHandle aoHandle, 
        const char* apcClientIp, 
        const char* apcServerIp, 
        int aiForP2P, 
        SCPMsgHeader& aoResponseMsgHeader );
        
    int Auth( 
        AISocketHandle aoHandle, 
        const char* apcClientIp, 
        const char* apcServerIp, 
        int aiForP2P,
        const char* apcGwID,
        const char* apcPassword,
        SCPMsgHeader& aoResponseMsgHeader );
};

#endif //__SCP_PROTOCOL_H__
