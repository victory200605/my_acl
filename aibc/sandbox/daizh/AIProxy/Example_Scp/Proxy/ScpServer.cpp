
#include "AIConfig.h"
#include "ScpEvent.h"
#include "ScpProtocol.h"
#include "ScpServer.h"

#define IS_EXIST_INI_S( section, name, var ) (loConfig.GetIniString( section, name, var, sizeof(var) ) <= 0?false:true)

#define GET_INI_S( section, name, var ) \
    if ( loConfig.GetIniString( section, name, var, sizeof(var) ) <= 0 ) \
	{ \
	    AISCPPROXY_LOG_ERROR( "Load variable [%s] from config file %s fail", name, apcConfigFile ); \
        return -1; \
	}
#define GET_INI_N( section, name, var, def ) var = loConfig.GetIniInt( section, name, def )

///////////////////////////////////////// SCP-InnerServer //////////////////////////////////////////
clsScpInnerServer::clsScpInnerServer( AIInnerProtocol* apoProtocol ) :
    AIProxy::AIInnerServer(apoProtocol)
{
}

int clsScpInnerServer::Initialize( const char* apcConfigFile )
{
    int       liRetCode = 0;
    AIConfig  loConfig;
    char      lsServerIp[AI_IP_ADDR_LEN] = {0};
    int       liServerPort = -1;
	
    if ( loConfig.LoadINI(apcConfigFile) != 0 )
    {
        AISCPPROXY_LOG_ERROR( "Load config file %s fail, [OuterServer]", apcConfigFile );
        return -1;
    }
    
    GET_INI_S( "SERVER", "AdminListenIpAddr", lsServerIp );
    GET_INI_N( "SERVER", "ProxyServerPort",   liServerPort, -1 );
    
    if ( ( liRetCode = this->Startup( lsServerIp, liServerPort ) ) != 0 )
    {
        AISCPPROXY_LOG_ERROR( "Startup server [%s:%d] fail, [RetCode=%d]", lsServerIp, liServerPort, liRetCode );
        return -1;
    }
    
    AISCPPROXY_LOG_INFO( "Startup server [%s:%d] success", lsServerIp, liServerPort );
    
    return 0;
}

int clsScpInnerServer::ChannelInput( AIChannelPtr aoChannelPtr, const char* apcData, size_t aiSize )
{
    if ( clsScpProtocol::IsHandshake( apcData, aiSize ) )
    {
        //hold up handshake
        SCPMsgHeader loRequest;
        AIChunkEx    loBuffer(0);
        AIChunkEx    loData( apcData, aiSize );

        loRequest.Decode(loData);
        
        SCPMsgHeader loResponse(MSG_TYPE_SHAKEHAND_RSP);
        loResponse.GeneralHeader()->SetCSeq( loRequest.GeneralHeader()->GetCSeq() );
        loResponse.GeneralHeader()->SetFrom( loRequest.GeneralHeader()->GetTo() );
        loResponse.GeneralHeader()->SetTo( loRequest.GeneralHeader()->GetFrom() );
        loResponse.GeneralHeader()->SetServiceKey( loRequest.GeneralHeader()->GetServiceKey() );
        loResponse.Encode( loBuffer );
        
        aoChannelPtr->Send( loBuffer.BasePtr(), loBuffer.GetSize() );
        return 0;
    }
    
    return AIProxy::AIInnerServer::ChannelInput( aoChannelPtr, apcData, aiSize );
}

///////////////////////////////////////// SCP-OutnerServer //////////////////////////////////////////
clsScpOuterServer::clsScpOuterServer( AIOuterProtocol* apoProtocol ) :
    AIProxy::AIOuterServer(apoProtocol),
    ciActiveTestInterval(60*10)
{
}

clsScpOuterServer::~clsScpOuterServer()
{
}
    
int clsScpOuterServer::Initialize( const char* apcConfigFile )
{
    int          liRetCode = 0;
    stServerNode loServerNode;
    AIConfig     loConfig;
    char         lsSection[AI_MAX_NAME_LEN] = {0};
	
    if ( loConfig.LoadINI(apcConfigFile) != 0 )
    {
        AISCPPROXY_LOG_ERROR( "Load config file %s fail, [OuterServer]", apcConfigFile );
        return -1;
    }
    
    GET_INI_S( "SERVER", "AdminListenIpAddr", loServerNode.csClientIp );
    GET_INI_S( "PROXY", "GWID", loServerNode.csGwID );
    
    this->cvoServers.clear();
    for ( int liN = 0; true ; liN++ )
    {
        ::snprintf( lsSection, sizeof(lsSection), "SCPSERVER_%d", liN );
        if ( !IS_EXIST_INI_S(lsSection, "ServerName", loServerNode.csServerName) )
        {
            break;
        }
        GET_INI_S( lsSection, "ServerIpAddr",  loServerNode.csServerIp );
        GET_INI_N( lsSection, "ServerPort",    loServerNode.ciServerPort, -1 );
        GET_INI_N( lsSection, "ForP2P",        loServerNode.ciForP2P, 0 );
        GET_INI_S( lsSection, "Password",      loServerNode.csPassword );
        GET_INI_N( lsSection, "ConnectionCnt", loServerNode.ciConnectionCnt, 1 );
        
        {
            AISmartLock loLock(coLock);
            this->cvoServers.push_back(loServerNode);
        }
    }
    
    GET_INI_N( "PROXY", "ActiveTestInterval", this->ciActiveTestInterval, 60 );
    
    ///connect to scp server
    for ( size_t liN = 0; liN < this->cvoServers.size(); liN++ )
    {
        if ( ( liRetCode = 
             this->UpdateConnection( 
                this->cvoServers[liN].csServerName, 
                this->cvoServers[liN].csServerIp, 
                this->cvoServers[liN].ciServerPort,
                this->cvoServers[liN].ciConnectionCnt, 3 /*timeout*/ ) ) != 0 )
        {
            AISCPPROXY_LOG_ERROR( "Connect to scp server [%s:%d] fail, [RetCode=%d]", 
                this->cvoServers[liN].csServerIp, this->cvoServers[liN].ciServerPort, liRetCode );
            return -1;
        }
        else
        {
            AISCPPROXY_LOG_INFO( "Connect to scp server [%s:%d] success, [ConnCnt=%d]", 
                this->cvoServers[liN].csServerIp, this->cvoServers[liN].ciServerPort, this->cvoServers[liN].ciConnectionCnt );
        }
    }
    
    return 0;
}

int clsScpOuterServer::ChannelInput( AIChannelPtr aoChannelPtr, const char* apcData, size_t aiSize )
{
    if ( clsScpProtocol::IsHandshake( apcData, aiSize ) )
    {
        //hold up handshake
        return 0;
    }
    
    return AIProxy::AIOuterServer::ChannelInput( aoChannelPtr, apcData, aiSize );
}

int clsScpOuterServer::ChannelConnected( AIChannelPtr aoChannelPtr )
{
    AIChannelPtr::To<AIProxyChannel*> loChannelPtr(aoChannelPtr);
        
    loChannelPtr->RegisterTimerEvent( new clsScpActiveEvent( this->ciActiveTestInterval ) );
    
    return 0;
}

int clsScpOuterServer::DoHandshake( AIChannelPtr aoChannelPtr )
{
    int          liRetCode = 0;
    AIChunkEx    loBuffer(0);
    stServerNode loServerNode;
    AIChannelPtr::To<AIProxyChannel*> loProxyChannelPtr(aoChannelPtr);

    if ( !this->GetServerNode( loProxyChannelPtr->GetGroupName(), loServerNode ) )
    {
        AISCPPROXY_LOG_ERROR( "Encode Handshake request fail, can't found servernode, [Group=%s]/[ChannelID=%d]", 
            loProxyChannelPtr->GetGroupName(), loProxyChannelPtr->GetChannelID() );
        return -1;
    }
    
    SCPMsgHeader loMsgHeader(MSG_TYPE_SHAKEHAND);
    loMsgHeader.GeneralHeader()->SetCSeq( 0 );
    loMsgHeader.GeneralHeader()->SetFrom( loServerNode.csClientIp );
    loMsgHeader.GeneralHeader()->SetTo( loServerNode.csServerIp );
    loMsgHeader.GeneralHeader()->SetServiceKey( ( loServerNode.ciForP2P != 0 ? 13 : 12 ) );
    loMsgHeader.Encode(loBuffer);
    
    if ( ( liRetCode = aoChannelPtr->Send( loBuffer.BasePtr(), loBuffer.GetSize() ) ) != 0 )
    {
        AISCPPROXY_LOG_ERROR( "Send Handshake request fail, [Group=%s]/[ChannelID=%d]/[SYNCCode=%d]", 
            loProxyChannelPtr->GetGroupName(), loProxyChannelPtr->GetChannelID(), liRetCode );
    }
    else
    {
        AISCPPROXY_LOG_INFO( "Send Handshake request success, [Group=%s]/[ChannelID=%d]", 
            loProxyChannelPtr->GetGroupName(), loProxyChannelPtr->GetChannelID());
    }
    
    return liRetCode;
}

bool clsScpOuterServer::GetServerNode( const char* apcName, stServerNode& aoServerNode )
{
    AISmartLock loLock(coLock);
    for ( size_t liN = 0; liN < this->cvoServers.size(); liN++ )
    {
        if ( ::strcmp( this->cvoServers[liN].csServerName, apcName ) == 0 )
        {
            aoServerNode = this->cvoServers[liN];
            return true;
        }
    }
    return false;
}
