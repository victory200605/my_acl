
#include "AISyncTcp.h"

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
    
    ::ai_socket_remote_addr( this->ctHandle, this->csRemoteIp, this->ciRemotePort );
    
    // Start deamon thread
    if ( this->ciMode == REACTIVE )
    {
        // Client single channel do not need InputHandle
        if ( ( liRetCode = ::pthread_create( &ciThrd, NULL, InputHandleThread, this ) ) != 0 )
        {
            this->cbIsClosed = true;
            return AI_SYNC_ERROR_THREAD;
        }
        this->cbIsExited = false;
    }
    
    this->coState.ciStartTime    = AICurTime();
    this->coState.ciLastSendTime = AICurTime();
    this->coState.ciLastRecvTime = AICurTime();
    
    return liRetCode;
}

void AIBaseChannel::CloseHandle()
{
    this->cbIsClosed = true;
    
    ::ai_socket_close( this->ctHandle );
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
        AISleepFor( AI_TIME_MSEC * 100 );
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
    
    AISmartLock loLock( coSendLock );
    
    // Send request
    if ( ( liRetCode = this->SendTo( this->ctHandle, apcData, aiSize ) ) != 0 )
    {
        return liRetCode;
    }
    
    this->coState.ciLastSendTime = AICurTime();
    this->coState.ciSendCount++;
    
    this->ChangeEvent( this->coState );
    
    return liRetCode;
}

int AIBaseChannel::Recv( AIChunkEx& aoResponse, int aiTimeout /*=AI_SYNC_TIMEOUT*/ )
{
    int  liRetCode    = 0;  
    AISmartLock loLock( coRecvLock );
    
    //Recv response
    if ( ( liRetCode = this->RecvFrom( this->ctHandle, aoResponse, AI_SYNC_TIMEOUT ) ) != 0 )
    {
        return liRetCode;
    }
    
    this->coState.ciLastRecvTime = AICurTime();
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
    AIChunkEx  loChunk;

    do
    {
        while( !this->IsClosed() && !this->IsReady() )
        {
            AISleepFor( AI_TIME_MSEC * 100 );
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
    if ( this->cpoChannel != NULL ) cpoChannel->Release();
}

AIChannelPtr::AIChannelPtr( AIBaseChannel* apoChannel ) :
    cpoChannel(apoChannel)
{
    if ( this->cpoChannel != NULL ) cpoChannel->Reference();
}

AIChannelPtr::AIChannelPtr( const AIChannelPtr& aoRhs ) :
    cpoChannel(aoRhs.cpoChannel)
{
    if ( this->cpoChannel != NULL ) cpoChannel->Reference();
}
    
void AIChannelPtr::operator = ( AIBaseChannel* apoChannel )
{
    this->cpoChannel = apoChannel;
    if ( this->cpoChannel != NULL ) cpoChannel->Reference();
}

void AIChannelPtr::operator = ( const AIChannelPtr& aoRhs )
{
    this->cpoChannel = aoRhs.cpoChannel;
    if ( this->cpoChannel != NULL ) cpoChannel->Reference();
}

bool AIChannelPtr::operator == ( AIBaseChannel* apoChannel )
{
    return this->cpoChannel == apoChannel;
}

bool AIChannelPtr::operator == ( const AIChannelPtr& aoRhs )
{
    return this->cpoChannel == aoRhs.cpoChannel;
}

bool AIChannelPtr::operator != ( AIBaseChannel* apoChannel )
{
    return !(*this == apoChannel);
}

bool AIChannelPtr::operator != ( const AIChannelPtr& aoRhs )
{
    return !(*this == aoRhs.cpoChannel);
}
    
AIBaseChannel* AIChannelPtr::operator -> ()
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
    
    this->ctListenHandle = ::ai_socket_listen( apsIp, aiPort, aiBackLog );
    if ( this->ctListenHandle <= 0 )
    {
        AI_SYNC_ERROR( "[Server]:Listen [Ip=%s/Port=%d] fail, [MSG=%s]",apsIp, aiPort, strerror(errno) );
        
        this->ctListenHandle = AI_INVALID_HANDLE;
        
        return -1;
    }
        
    if ( ( liRetCode = ::pthread_create( &ciThrd, NULL, AcceptHandleThread, this ) ) != 0 )
    {
        AI_SYNC_ERROR( "[Server]:Create AcceptHandle thread fail, [Ip=%s/Port=%d]/[MSG=%s]", apsIp, aiPort, strerror(errno) );
        
        return liRetCode;
    }
    
    this->cbIsClosed = false;
    
    return liRetCode;
}

void AIBaseAcceptor::Close()
{
    this->cbIsClosed = true;
    ::ai_socket_close( this->ctListenHandle );
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
        if( (ltRemoteHandle = ::ai_socket_accept(
            this->ctListenHandle, lsRemoteIp, &liRemotePort )) < 0)
        {
            if ( !this->IsClosed() )
            {
                AI_SYNC_ERROR( "[Server]:Accept first connection fail, [ListenHandle=%d]/[RemoteHandle=%d]/[MSG=%s]", 
                    this->ctListenHandle, ltRemoteHandle, strerror(errno) );
            }
            continue;
        }

        lpoChannel = this->CreateChannel();
        if ( ( liRetCode = lpoChannel->Initialize( ltRemoteHandle, this->ciChannelID++ ) ) != 0 )
        {
            ::ai_socket_close(ltRemoteHandle);
            
            AI_SYNC_ERROR( "[Server]:  initialize channel for new connection fail, [ListenHandle=%d]/[RemoteHandle=%d]/[MSG=%s]", 
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
    int       liRetCode = 0;
    AISocketHandle ltHandle  = AI_INVALID_HANDLE;

    do
    {
        if ( (ltHandle = ::ai_socket_connect( apsIp, aiPort, aiTimeout )) < 0 )
        {
            AI_SYNC_ERROR( "[Client]:Connection to [Ip=%s/Port=%d] fail, [MSG=%s]", apsIp, aiPort, strerror(errno) );
            liRetCode = AI_SYNC_ERROR_SOCKET;
            break;
        }
        
        if ( ( liRetCode = aoChannel.Initialize( ltHandle, this->ciChannelID++ ) ) != 0 )
        {
            AI_SYNC_ERROR(  "[Client]:Initialize channel for connection fail, [Ip=%s]/[Port=%d]/[MSGCODE=%d]", 
                apsIp, aiPort, liRetCode );
    
            break;
        }
        
        return AI_SYNC_NO_ERROR;
    }
    while( false );
    
    ::ai_socket_close(ltHandle);
                    
    return liRetCode;
}

void AIBaseConnector::Close()
{
}
