
#ifndef __AIPROXY_SCPSERVER_H__
#define __AIPROXY_SCPSERVER_H__

#include "../../AIProxyServer.h"

using namespace AIProxy;

#define AISCPPROXY_LOG_TRACE( format, ... )
#define AISCPPROXY_LOG_INFO( format, ... ) fprintf( stderr, "INFO: " format "\n", __VA_ARGS__ );
#define AISCPPROXY_LOG_ERROR( format, ... ) fprintf( stderr, "ERROR: " format "\n", __VA_ARGS__ );

///////////////////////////////////////// SCP-InnerServer //////////////////////////////////////////
class clsScpInnerServer : public AIProxy::AIInnerServer
{
public:
    clsScpInnerServer( AIInnerProtocol* apoProtocol );
    
    int Initialize( const char* apcConfigFile );
    
    virtual int ChannelInput( AIChannelPtr aoChannelPtr, const char* apcData, size_t aiSize );
};

///////////////////////////////////////// SCP-OuterServer //////////////////////////////////////////
class clsScpOuterServer : public AIProxy::AIOuterServer
{
public:
    struct stServerNode
    {
        char csServerName[AI_MAX_NAME_LEN];
        char csClientIp[AI_IP_ADDR_LEN];
        char csServerIp[AI_IP_ADDR_LEN];
        char csGwID[AI_MAX_NAME_LEN];
        int  ciServerPort;
        int  ciForP2P;
        char csPassword[AI_MAX_PASSWORD_LEN];
        int  ciConnectionCnt;
    };
    
public:
    clsScpOuterServer( AIOuterProtocol* apoProtocol );
    ~clsScpOuterServer();
    
    int Initialize( const char* apcConfigFile );
    
    virtual int ChannelInput( AIChannelPtr aoChannelPtr, const char* apcData, size_t aiSize );
    virtual int ChannelConnected( AIChannelPtr aoChannelPtr );
    
    int DoHandshake( AIChannelPtr aoChannelPtr );
    
    bool GetServerNode( const char* apcName, stServerNode& aoServerNode );

protected:
    int ciActiveTestInterval;
    AI_STD::vector<stServerNode> cvoServers;
    AIMutexLock                  coLock;
};

#endif //__AIPROXY_SCPSERVER_H__
