
#include "AIProxyServer.h"

START_AIPROXY_NAMESPACE

///implement
AIProxyChannel::AIProxyChannel( CHANNEL_MODE aiMode /*= AIBaseChannel::REACTIVE*/ ) :
    AIBaseChannel(aiMode)
{
    memset( this->cpoTimerEvent, 0, sizeof(this->cpoTimerEvent) );
    memset( this->cpoChangeEvent, 0, sizeof(this->cpoChangeEvent) );
}

AIProxyChannel::~AIProxyChannel()
{
    this->ClearEvent( this->cpoTimerEvent );
    this->ClearEvent( this->cpoChangeEvent );
}

void AIProxyChannel::SetServer( AIServer* apoServer )
{
    this->cpoServer = apoServer;
}

void AIProxyChannel::SetProtocol( AIProtocol* apoProtocol )
{
    this->cpoProtocol = apoProtocol;
}

void AIProxyChannel::SetGroupName( const char* apcGroupName )
{
    ::snprintf( this->csGroupName, sizeof(this->csGroupName), "%s", apcGroupName );
}

const char* AIProxyChannel::GetGroupName()
{
    return this->csGroupName;
}

int AIProxyChannel::Input( const char* apcData, size_t aiSize )
{
    this->cpoServer->ChannelInput( this, apcData, aiSize );
    return 0;
}

int AIProxyChannel::Error( int aiErrno )
{
    this->cpoServer->ChannelError( this, aiErrno );
    return 0;
}

int AIProxyChannel::Exit()
{
    this->cpoServer->ChannelExit( this );
    return 0;
}

void AIProxyChannel::TimerEvent( AIBaseChannel::TState aoState )
{
    this->DoEvent( this->cpoTimerEvent, aoState );
}

void AIProxyChannel::ChangeEvent( AIBaseChannel::TState aoState )
{
    this->DoEvent( this->cpoChangeEvent, aoState );
}

void AIProxyChannel::RegisterTimerEvent( AIChannelEvent* apoEvent )
{
    this->RegisterEvent( this->cpoTimerEvent, apoEvent );
}

void AIProxyChannel::RegisterChangeEvent( AIChannelEvent* apoEvent )
{
    this->RegisterEvent( this->cpoChangeEvent, apoEvent );
}

void AIProxyChannel::RegisterEvent( AIChannelEvent** apoEventArray, AIChannelEvent* apoEvent )
{
    int liIdx = 0;
    while ( liIdx < AI_MAX_EVENT_CNT && apoEventArray[liIdx] != NULL ) liIdx++;
    if ( liIdx < AI_MAX_EVENT_CNT )
    {
        apoEventArray[liIdx] = apoEvent;
    }
    else
    {
        assert( false );
    }
}

void AIProxyChannel::DoEvent( AIChannelEvent** apoEventArray, AIBaseChannel::TState aoState )
{
    for ( int liN = 0; liN < AI_MAX_EVENT_CNT && apoEventArray[liN] != NULL; liN++ )
    {
        if ( apoEventArray[liN]->Condition( aoState ) )
        {
            apoEventArray[liN]->Action( this );
        }
    }
}

void AIProxyChannel::ClearEvent( AIChannelEvent** apoEventArray )
{
    for ( int liN = 0; liN < AI_MAX_EVENT_CNT && apoEventArray[liN] != NULL; liN++ )
    {
        AI_DELETE(apoEventArray[liN]);
    }
}

int AIProxyChannel::SendTo( AISocketHandle atHandle, const char* apsData, size_t aiSize )
{
    return this->cpoProtocol->Send( atHandle, apsData, aiSize );
}

int AIProxyChannel::RecvFrom( AISocketHandle atHandle, AIChunkEx& aoBuffer, int aiTimeout )
{
    return this->cpoProtocol->Recv( atHandle, aoBuffer, aiTimeout );
}

/////////////////////////////////////////////// input server ///////////////////////////////////////
AIInnerServer::AIInnerServer( AIInnerProtocol* apoProtocol ) :
    cpoProtocol(apoProtocol)
{
}

AIInnerServer::~AIInnerServer()
{
    AI_DELETE( this->cpoProtocol );
}

AIProtocol* AIInnerServer::GetProtocol( void )
{
    return this->cpoProtocol;
}

int AIInnerServer::Send( const char* apcAddressee, const char* apcData, size_t aiSize )
{
    AIChannelPtr loChannelPtr = this->coChannelPool.GetChannel( apcAddressee );
    
    if ( loChannelPtr == NULL )
    {
        return AI_ERROR_CONN_NO_FOUND;
    }
    
    if ( loChannelPtr->Send( apcData, aiSize ) != 0 )
    {
        return AI_ERROR_SEND;
    }
    
    return AI_NO_ERROR;
}

int AIInnerServer::ChannelInput( AIChannelPtr aoChannelPtr, const char* apcData, size_t aiSize )
{
    char lsSender[AI_MAX_NAME_LEN] = {0};
    AIChannelPtr::To<AIProxyChannel*> loProxyChannel(aoChannelPtr);
    
    ::snprintf( lsSender, sizeof(lsSender), "%s/%d", loProxyChannel->GetGroupName(), loProxyChannel->GetChannelID() );
    
    return AIRouter::Instance()->Output( lsSender, apcData, aiSize );
}

int AIInnerServer::ChannelError( AIChannelPtr aoChannelPtr, int aiErrno )
{
    aoChannelPtr->CloseHandle();
    return 0;
}

int AIInnerServer::ChannelExit( AIChannelPtr aoChannelPtr )
{
    AIChannelPtr::To<AIProxyChannel*> loProxyChannel(aoChannelPtr);
        
    this->coChannelPool.EraseChannel( loProxyChannel->GetGroupName(), aoChannelPtr );
    
    return 0;
}

int AIInnerServer::ChannelAccept( AIChannelPtr aoChannelPtr )
{
    return 0;
}

///for server
AIBaseChannel* AIInnerServer::CreateChannel()
{
    AIBaseChannel* lpoChannel = NULL;
    AI_NEW_ASSERT( lpoChannel, AIProxyChannel );
    return lpoChannel;
}

int AIInnerServer::Accept( AIBaseChannel* apoChannel )
{
    int  liRetCode = 0;
    char lsName[AI_MAX_NAME_LEN] = {0};
    AIProxyChannel* lpoChannel = static_cast< AIProxyChannel* >(apoChannel);
    
    lpoChannel->SetServer( this );
    lpoChannel->SetProtocol( this->cpoProtocol );
    
    if ( ( liRetCode = this->cpoProtocol->Authenticate( lpoChannel->GetHandle(), lsName, sizeof(lsName) ) ) != 0 )
    {///Authenticate fail
        lpoChannel->CloseHandle();
        AI_PROXY_ERROR( "Inner server authenticate fail, [Handle=%d]", lpoChannel->GetHandle() );
        return -1;
    }
    else
    {///Authenticate succes
        lpoChannel->SetGroupName( lsName );
        
        this->coChannelPool.PutChannel( lsName, apoChannel );

        this->ChannelAccept( lpoChannel );
        
        lpoChannel->Ready();
        
        AI_PROXY_TRACE( "Inner server accept connection, [GroupName=%s]/[Handle=%d]", lsName, lpoChannel->GetHandle() );
        
        return 0;
    }
}

/////////////////////////////////////////////// output server ///////////////////////////////////////
AIOuterServer::AIOuterServer( AIOuterProtocol* apoProtocol ) :
    cpoProtocol(apoProtocol)
{
}

AIOuterServer::~AIOuterServer()
{
    AI_DELETE( this->cpoProtocol );
}
    
int AIOuterServer::AddConnection( const char* apcGroupName, const char* apcIpAddr, int aiPort, size_t aiCount, int aiTimeOut )
{
    int             liRetCode  = 0;
    AIProxyChannel* lpoChannel = NULL;

    for ( size_t liN = 0; liN < aiCount; liN++ )
    {
        lpoChannel = this->CreateChannel();
        lpoChannel->SetServer( this );
        lpoChannel->SetProtocol( this->cpoProtocol );
        lpoChannel->SetGroupName(apcGroupName);
    
        if ( ( liRetCode = this->coConnector.Connect(apcIpAddr, aiPort, *lpoChannel, aiTimeOut) ) != 0 )
        {
            AI_DELETE( lpoChannel );
            AI_PROXY_ERROR( "Outer server Connect fail, [Server=%s:%d]/[SYNCCode=%d]", apcIpAddr, aiPort, liRetCode );
            return AI_ERROR_CONNECTE;
        }
        
        if ( ( liRetCode = this->cpoProtocol->Authenticate( lpoChannel->GetHandle(), apcGroupName ) ) != 0 )
        {///Authenticate fail
            lpoChannel->CloseHandle();
            AI_PROXY_ERROR( "Outer server authenticate fail, [GroupName=%s]/[Handle=%d]/[RetCode=%d]", 
                apcGroupName, lpoChannel->GetHandle(), liRetCode );
            return AI_ERROR_AUTHENTICATE;
        }
        
        this->coChannelPool.PutChannel( apcGroupName, lpoChannel );
        this->ChannelConnected( lpoChannel );
        lpoChannel->Ready();
    }
    
    return AI_NO_ERROR;
}

int AIOuterServer::UpdateConnection( const char* apcGroupName, const char* apcIpAddr, int aiPort, size_t aiCount, int aiTimeOut )
{
    while ( true )
    {
        size_t liConnCnt = this->coChannelPool.GetChannelCnt(apcGroupName);
        AIChannelPtr loChannelPtr = this->coChannelPool.GetChannel(apcGroupName);
        if ( liConnCnt > aiCount )
        {
            //Erase channel from channelpool
            this->coChannelPool.EraseChannel( apcGroupName, loChannelPtr );
            this->ChannelClose( loChannelPtr );
        }
        else if ( liConnCnt < aiCount )
        {
            return this->AddConnection( apcGroupName, apcIpAddr, aiPort, aiCount - liConnCnt, aiTimeOut );
        }
        else //liConnCnt == aiCount
        {
            return 0;
        }
    }
    
    return 0;
}

size_t AIOuterServer::GetConnectionCnt( const char* apcGroupName )
{
    return this->coChannelPool.GetChannelCnt(apcGroupName);
}

AIChannelPtr AIOuterServer::GetConnection( const char* apcGroupName )
{
    return this->coChannelPool.GetChannel(apcGroupName);
}

AIProtocol* AIOuterServer::GetProtocol( void )
{
    return this->cpoProtocol;
}

int AIOuterServer::Send( const char* apcAddressee, const char* apcData, size_t aiSize )
{
    AIChannelPtr loChannelPtr = this->coChannelPool.GetChannel( apcAddressee );
    
    if ( loChannelPtr == NULL )
    {
        return AI_ERROR_CONN_NO_FOUND;
    }
    
    if ( loChannelPtr->Send( apcData, aiSize ) != 0 )
    {
        return AI_ERROR_SEND;
    }
    
    return AI_NO_ERROR;
}

int AIOuterServer::ChannelInput( AIChannelPtr aoChannelPtr, const char* apcData, size_t aiSize )
{
    AIChannelPtr::To<AIProxyChannel*> loProxyChannel(aoChannelPtr);
    char      lsSender[AI_MAX_NAME_LEN] = {0};
    
    ::snprintf( lsSender, sizeof(lsSender), "%s/%d", loProxyChannel->GetGroupName(), loProxyChannel->GetChannelID() );

    return AIRouter::Instance()->Input( lsSender, apcData, aiSize );
}

int AIOuterServer::ChannelError( AIChannelPtr aoChannelPtr, int aiErrno )
{
    aoChannelPtr->CloseHandle();
    return 0;
}

int AIOuterServer::ChannelExit( AIChannelPtr aoChannelPtr )
{
    AIChannelPtr::To<AIProxyChannel*> loProxyChannel(aoChannelPtr);
        
    this->coChannelPool.EraseChannel( loProxyChannel->GetGroupName(), aoChannelPtr );
    return 0;
}

int AIOuterServer::ChannelClose( AIChannelPtr aoChannelPtr )
{
    aoChannelPtr->CloseHandle();
    return 0;
}

int AIOuterServer::ChannelConnected( AIChannelPtr aoChannelPtr )
{
    return 0;
}

AIProxyChannel* AIOuterServer::CreateChannel()
{
    AIProxyChannel* lpoChannel = NULL;
    AI_NEW_ASSERT( lpoChannel, AIProxyChannel );
    return lpoChannel;
}

/*
////////////////////////////////////////////// sync output server ////////////////////////////////////////
AISyncOuterServer::AISyncOuterServer( AIProtocol* apoProtocol ) :
    AIOuterServer(apoProtocol)
{
}

AISyncOuterServer::~AISyncOuterServer()
{
}

AIProxyChannel* AISyncOuterServer::CreateChannel()
{
    AIProxyChannel* lpoChannel = NULL;
    AI_NEW_INIT_ASSERT( lpoChannel, AIProxyChannel, AIBaseChannel::NORMAL );
    return lpoChannel;
}

int AISyncOuterServer::GenMsgID( const TPackage& aoPackage, char* apcMsgID, size_t aiMsgIDSize )
{
    ::snprintf( apcMsgID, aiMsgIDSize, aoPackage.GenMsgID() );
    return 0;
}

int AISyncOuterServer::Send( const TPackage& aoPackage )
{
    stMapValue loValue;
    AIChunkEx  loBuffer;
    
    {
        AISmartLock loLock( this->coLock );
        TChannelMap::iterator loIter = this->coChannelMap.find( aoPackage.GetAddressee() );
        if ( loIter == this->coChannelMap.end() )
        {
            AI_PROXY_ERROR( "Inner server can't found Addressee, [Addressee=%s]/[MasID=%s]", 
                aoPackage.GetAddressee(), aoPackage.GenMsgID() );
            return ERROR_NO_FOUND;
        }
        loValue = loIter->second;
    }
    
    {
        AISmartLock loLock( *loValue.cpoLock );
        if ( loValue.cpoChannel->Send( aoPackage.GetData(), aoPackage.GetSize() ) != 0 )
        {
            return ERROR_SEND;
        }
        if ( loValue.cpoChannel->Recv( loBuffer, 10 ) != 0 )
        {
            return ERROR_RECV;
        }
        
        this->Input( static_cast<AIProxyChannel*>(loValue.cpoChannel)->GetChannelAlias(), 
            loBuffer.BasePtr(), loBuffer.GetSize() );
    }
    
    return NO_ERROR;
}
*/

END_AIPROXY_NAMESPACE
