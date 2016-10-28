
#include "../include/AICacheTcp.h"

AI_CACHE_NAMESPACE_START

/////////////////////////////////////////////////////////////////////////////////////
//AIBaseChannel Implement
AIBaseChannel::stState::stState() :
    ciStartTime(0),
    ciLastSendTime(0),
    ciLastRecvTime(0),
    ciSendCount(0),
    ciRecvCount(0)
{
}

AIBaseChannel::AIBaseChannel( CHANNEL_MODE aiMode /*= REACTIVE*/ ) :
    ctHandle(AI_INVALID_HANDLE),
    ciRemotePort(-1),
    ciChannelID(-1),
    ciMode(aiMode),
    cbIsClosed(true),
    cbIsExited(true),
    cbIsReady(false),
    ciReferenceCount(0)
{
    this->ctHandle = AI_INVALID_HANDLE;
}

AIBaseChannel::~AIBaseChannel()
{
    this->Close();
}

int AIBaseChannel::Initialize( AISocketHandle atHandle, int aiChannelID  )
{
    int liRetCode = 0;

    if ( !this->IsClosed() )
    {
        return AI_SYNC_ERROR_ACTIVE;
    }
    
    // Set state variable
    this->ctHandle     = atHandle;
    this->ciChannelID  = aiChannelID;
    this->cbIsClosed   = false;
    this->cbIsExited   = true;
    
    AIBC::ai_socket_remote_addr( this->ctHandle, this->csRemoteIp, this->ciRemotePort );

    // Start deamon thread
    if ( this->ciMode == REACTIVE )
    {
        // Client single channel do not need InputHandle
        if ( ( liRetCode = ::pthread_create( &ciThrd, NULL, InputHandleThread, this ) ) != 0 )
        {
            this->cbIsClosed = true;
            return AI_SYNC_ERROR_THREAD;
        }
        
        while( this->cbIsExited )
        {
            AIBC::AISleepFor( AI_TIME_MSEC * 10 );
        }
    }
    
    this->coState.ciStartTime    = AIBC::AICurTime();
    this->coState.ciLastSendTime = AIBC::AICurTime();
    this->coState.ciLastRecvTime = AIBC::AICurTime();
    
    return liRetCode;
}

void AIBaseChannel::CloseHandle()
{
    this->cbIsClosed = true;
    
    AIBC::ai_socket_close( this->ctHandle );
}

void AIBaseChannel::Close()
{
    this->CloseHandle();
    
    while( true )
    {
        if ( cbIsExited == true 
            || ( pthread_kill( ciThrd, 0 ) == ESRCH )  )
        {
            cbIsExited = true;
            break;
        }
        AIBC::AISleepFor( AI_TIME_MSEC * 100 );
    }
}

bool AIBaseChannel::IsClosed()
{
    return this->cbIsClosed;
}


bool AIBaseChannel::IsReactive()
{
    return this->ciMode == REACTIVE;
}

void AIBaseChannel::Reference()
{
    ++this->ciReferenceCount;
}

void AIBaseChannel::Release()
{
    if ( --this->ciReferenceCount <= 0 )
    {
        AIBaseChannel* lpoChannel = this;
        AI_DELETE(lpoChannel);
    }
}

int AIBaseChannel::Send( const char* apcData, size_t aiSize )
{
    int  liRetCode = 0;
    
    AIBC::AISmartLock loLock( coSendLock );
    
    // Send request
    if ( ( liRetCode = this->SendTo( this->ctHandle, apcData, aiSize ) ) != 0 )
    {
        return liRetCode;
    }
    
    this->coState.ciLastSendTime = AIBC::AICurTime();
    this->coState.ciSendCount++;
    
    this->ChangeEvent( this->coState );
    
    return liRetCode;
}

int AIBaseChannel::Recv( AIBC::AIChunkEx& aoResponse, int aiTimeout /*=AI_SYNC_TIMEOUT*/ )
{
    int  liRetCode    = 0;  
    AIBC::AISmartLock loLock( coRecvLock );
    
    //Recv response
    if ( ( liRetCode = this->RecvFrom( this->ctHandle, aoResponse, AI_SYNC_TIMEOUT ) ) != 0 )
    {
        return liRetCode;
    }
    
    this->coState.ciLastRecvTime = AIBC::AICurTime();
    this->coState.ciRecvCount++;
    
    this->ChangeEvent( this->coState );
    
    return liRetCode;
}

int AIBaseChannel::Input( const char* apcData, size_t aiSize )
{
    return 0;
}

int AIBaseChannel::Error( int aiErrno )
{
    this->CloseHandle();
    return 0;
}

int AIBaseChannel::Exit()
{
    return 0;
}

void AIBaseChannel::TimerEvent( TState aoState )
{
}

void AIBaseChannel::ChangeEvent( TState aoState )
{
}

void* AIBaseChannel::InputHandleThread( void* apParam )
{
    pthread_detach(pthread_self());
    
    AIChannelPtr loChannelPtr = (AIBaseChannel*)apParam;
    loChannelPtr->InputHandle();
    return NULL;
}

void AIBaseChannel::InputHandle()
{
    int        liRetCode  = 0;
    int        liTimeWhisper = 0; //unit usec
    AIBC::AIChunkEx  loChunk;
    
    this->cbIsExited = false;

    do
    {
        while( !this->IsClosed() && !this->IsReady() )
        {
            AIBC::AISleepFor( AI_TIME_MSEC * 100 );
            if ( ( ( liTimeWhisper = liTimeWhisper + 100 ) / 1000 ) /*sec*/ > CHANNEL_PENDING_TIMEOUT )
            {
                break;
            }
            continue;
        }
        
        while( !this->IsClosed() )
        {
            liRetCode = this->Recv( loChunk, AI_SYNC_TIMEOUT );
            if ( liRetCode == AI_SYNC_NO_ERROR )
            {
                this->Input( loChunk.BasePtr(), loChunk.GetSize() );
            }
            else if ( liRetCode == AI_SYNC_ERROR_BROKEN )
            {
                break;
            }
            else if ( liRetCode != AI_SYNC_ERROR_TIMEOUT )
            {
                this->Error( liRetCode );
            }
            
            this->TimerEvent( this->coState );
        }
        
    }while(false);

    this->cbIsExited = true;
    this->Exit();
}

/////////////////////////////////////////////////////////////////////////////////////
//AIChannelPtr Implement
AIChannelPtr::AIChannelPtr( ) :
    cpoChannel(NULL)
{
}

AIChannelPtr::~AIChannelPtr( )
{
    if ( this->cpoChannel != NULL ) this->cpoChannel->Release();
}

AIChannelPtr::AIChannelPtr( TPtrType apoChannel ) :
    cpoChannel(apoChannel)
{
    if ( this->cpoChannel != NULL ) this->cpoChannel->Reference();
}

AIChannelPtr::AIChannelPtr( const AIChannelPtr& aoRhs ) :
    cpoChannel(aoRhs.cpoChannel)
{
    if ( this->cpoChannel != NULL ) this->cpoChannel->Reference();
}

void AIChannelPtr::operator = ( TPtrType apoChannel )
{
    if ( apoChannel != NULL ) apoChannel->Reference();
    if ( this->cpoChannel != NULL ) this->cpoChannel->Release();

    this->cpoChannel = apoChannel;
}

void AIChannelPtr::operator = ( const AIChannelPtr& aoRhs )
{
    *this = aoRhs.cpoChannel;
}

bool AIChannelPtr::operator == ( TPtrType apoChannel )
{
    return this->cpoChannel == apoChannel;
}

bool AIChannelPtr::operator == ( const AIChannelPtr& aoRhs )
{
    return this->cpoChannel == aoRhs.cpoChannel;
}

bool AIChannelPtr::operator != ( TPtrType apoChannel )
{
    return !(*this == apoChannel);
}

bool AIChannelPtr::operator != ( const AIChannelPtr& aoRhs )
{
    return !(*this == aoRhs.cpoChannel);
}

AIChannelPtr::TPtrType AIChannelPtr::operator -> ()
{
    return this->cpoChannel;
}

AIChannelPtr::operator TPtrType ()
{
    return this->cpoChannel;
}

/////////////////////////////////////////////////////////////////////////////////////
//AIBaseAcceptor Implement
AIBaseAcceptor::AIBaseAcceptor() : 
    ctListenHandle(AI_INVALID_HANDLE),
    cbIsClosed(true),
    ciThrd(0),
    ciChannelID(0)
{
}

AIBaseAcceptor::~AIBaseAcceptor()
{
    this->Close();
}

int AIBaseAcceptor::Startup( const char* apsIp, int aiPort, int aiBackLog /* = 10 */ )
{
    int liRetCode = 0;
    
    if ( !this->IsClosed() )
    {
        return AI_SYNC_ERROR_ACTIVE;
    }

    this->ctListenHandle = AIBC::ai_socket_listen( apsIp, aiPort, aiBackLog );
    if ( this->ctListenHandle <= 0 )
    {
        this->ctListenHandle = AI_INVALID_HANDLE;
        
        return AI_SYNC_ERROR_LISTEN;
    }
    
    this->cbIsClosed = false;
    
    if ( ( liRetCode = ::pthread_create( &ciThrd, NULL, AcceptHandleThread, this ) ) != 0 )
    {
        return AI_SYNC_ERROR_THREAD;
    }

    return liRetCode;
}

void AIBaseAcceptor::Close()
{
    this->cbIsClosed = true;
    AIBC::ai_socket_close( this->ctListenHandle );
    ::pthread_join( this->ciThrd, NULL );
}

bool AIBaseAcceptor::IsClosed()
{
    return this->cbIsClosed;
}

void* AIBaseAcceptor::AcceptHandleThread( void* apParam )
{
    AIBaseAcceptor* lpoServer = (AIBaseAcceptor*)apParam;
    lpoServer->AcceptHandle();
    return NULL;
}

void AIBaseAcceptor::AcceptHandle()
{
    int            liRetCode         = 0;
    int            liRemotePort      = 0;
    AIBaseChannel* lpoChannel        = NULL;
    char           lsRemoteIp[32];
    AISocketHandle ltRemoteHandle    = AI_INVALID_HANDLE;
    
    while( !this->IsClosed() )
    {
        if( (ltRemoteHandle = AIBC::ai_socket_accept(
            this->ctListenHandle, lsRemoteIp, &liRemotePort )) < 0)
        {
            if ( !this->IsClosed() )
            {
                AIBC::AI_SYNC_ERROR( "[SYNC-ACCEPTOR]:Accept first connection fail, [ListenHandle=%d]/[RemoteHandle=%d]/[MSG=%s]", 
                    this->ctListenHandle, ltRemoteHandle, strerror(errno) );
            }
            continue;
        }

        lpoChannel = this->CreateChannel();
        if ( ( liRetCode = lpoChannel->Initialize( ltRemoteHandle, this->GenChannelID() ) ) != 0 )
        {
            AIBC::ai_socket_close(ltRemoteHandle);
            
            AIBC::AI_SYNC_ERROR( "[SYNC-ACCEPTOR]:  initialize channel for new connection fail, [ListenHandle=%d]/[RemoteHandle=%d]/[MSG=%s]", 
                this->ctListenHandle, ltRemoteHandle, strerror(errno) );

            AI_DELETE( lpoChannel );
            
            continue;
        }

        this->Accept( lpoChannel );
    }
}


/////////////////////////////////////////////////////////////////////////////////////
//AISyncClient
AIBaseConnector::AIBaseConnector()
{
}

AIBaseConnector::~AIBaseConnector()
{
}

int AIBaseConnector::Connect( 
        const char* apsIp, int aiPort, AIBaseChannel& aoChannel, int aiTimeout /* = 0 */ )
{
    int liRetCode = 0;
    AISocketHandle ltHandle  = AI_INVALID_HANDLE;

    do
    {
        if ( (ltHandle = AIBC::ai_socket_connect( apsIp, aiPort, aiTimeout )) < 0 )
        {
            AIBC::AI_SYNC_ERROR( "[AYNC-CONNECTOR]:Connection to [Ip=%s/Port=%d] fail, [MSG=%s]", apsIp, aiPort, strerror(errno) );
            liRetCode = AI_SYNC_ERROR_CONN;
            break;
        }
        
        if ( ( liRetCode = aoChannel.Initialize( ltHandle, this->GenChannelID() ) ) != 0 )
        {
            AIBC::AI_SYNC_ERROR(  "[AYNC-CONNECTOR]:Initialize channel for connection fail, [Ip=%s]/[Port=%d]/[MSGCODE=%d]", apsIp, aiPort, liRetCode );
            break;
        }
        
        return AI_SYNC_NO_ERROR;
    }
    while( false );
    
    AIBC::ai_socket_close(ltHandle);
                    
    return liRetCode;
}

void AIBaseConnector::Close()
{
}

AI_CACHE_NAMESPACE_END
