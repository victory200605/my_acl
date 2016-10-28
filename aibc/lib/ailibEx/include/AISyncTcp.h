
#ifndef __AI_SYNC_TCP_H__
#define __AI_SYNC_TCP_H__

#include "AISynch.h"
#include "AIChunkEx.h"
#include "AISocket.h"

///start namespace
AIBC_NAMESPACE_START

#define AI_SYNC_TRACE( format, ... )
#define AI_SYNC_INFO( format, ... ) __SyncWriteLog( "INFO: " format "\n", __VA_ARGS__ )
#define AI_SYNC_ERROR( format, ... ) __SyncWriteLog( "ERROR: " format "\n", __VA_ARGS__ )

#define AI_SYNCHANDLE               int
#define AI_INVALID_SYNCHANDLE       (-1)

// Error no define
const int AI_SYNC_NO_ERROR            = (0);
const int AI_SYNC_ERROR_SOCKET        = (-1);
const int AI_SYNC_ERROR_SEND          = (-2);
const int AI_SYNC_ERROR_RECV          = (-3);
const int AI_SYNC_ERROR_BROKEN        = (-4);
const int AI_SYNC_ERROR_NO_CONN_SLOT  = (-5);
const int AI_SYNC_ERROR_INVALID_SLOT  = (-6);
const int AI_SYNC_ERROR_TIMEOUT       = (-7);
const int AI_SYNC_ERROR_LENGTH        = (-8);
const int AI_SYNC_ERROR_ACTIVE        = (-9);
const int AI_SYNC_ERROR_THREAD        = (-10);
const int AI_SYNC_ERROR_DECODE        = (-11);

// Const Declare
// Time out value
extern const int AI_SYNC_TIMEOUT;

// Channel buffer time out value
extern const int CHANNEL_CONN_TIMEOUT;

// Max line lenght
extern const int AI_MAX_LINE_LEN;
// End const

template<class TChannel> class AISyncBaseServer;
template<class TChannel> class AISyncClient;

void __SyncWriteLog( const char* apcFormat, ... );

/////////////////////////////////////////////////////////////////////////////////////
//AIBaseChannel
class AIBaseChannel
{
public:
    enum { T_UNKNOW = 0, T_SINGLE, T_DUAL };

public:
    AIBaseChannel();
    virtual ~AIBaseChannel();
    
    int  Initialize( AI_SYNCHANDLE atHandle[], const char* apsRemoteIp, int aiRemotePort, 
        int aiChannelID, bool abIsClient = false );
    void CloseHandle();
    void Close();
    
    int Request( const char* apsData, size_t aiSize );
    int Request( const char* apsData, size_t aiSize, AIChunkEx& aoResponse, int aiTimeout = AI_SYNC_TIMEOUT );
    int Response( const char* apsData, size_t aiSize );

    bool IsClosed();
    bool IsDual();
    
    void Ready()                                   { cbIsReady = true; }
    bool IsReady()                                 { return cbIsReady; } 
    
    const char* GetRemoteIp()                      { return csRemoteIp; }
    int GetRemotePort()                            { return ciRemotePort; }
    int GetChannelID()                             { return ciChannelID; }
    
protected:
    // Channel Hook method
    virtual int  Input( const char* apsData, size_t aiSize );
    virtual int  Error( AI_SYNCHANDLE atHandle, int aiErrno );
    virtual int  Exit();
    
    // Thread handle method
    static void* InputHandle( void* apParam );
    
    // Overwriteable method
    virtual int Send( AI_SYNCHANDLE atHandle, const char* apsData, size_t aiSize ) = 0;
    virtual int Recv( AI_SYNCHANDLE atHandle, AIChunkEx& aoChunk, int aiTimeout ) = 0;

protected:
    AI_SYNCHANDLE ctHandle[2];
    char          csRemoteIp[20];
    int           ciRemotePort;
    int           ciChannelID;
    
    // Status flag
    bool          cbIsClosed;
    bool          cbIsExited;
    bool          cbIsClient;
    bool          cbIsReady;

    // Input handle thread
    pthread_t     ciThrd;
    
    // Channel mutex
    AIMutexLock   coRequestLock;
    AIMutexLock   coResponseLock;
};

/////////////////////////////////////// *Protocol* //////////////////////////////////////////////
// Connected
class clsCmdLineConnection
{
public:
    static const char* S_SINGLE;
    static const char* S_DUAL;

public:
    clsCmdLineConnection( int aiType = AIBaseChannel::T_UNKNOW );

    int Encode( AIChunkEx& aoChunk );
    int Decode( AIChunkEx& aoChunk );
    size_t GetSize();
    
    void SetType( int aiType );
    int  GetType();
    
protected:
    int  ciType;
};

// Connected
class clsDefaultConnection
{
public:
    clsDefaultConnection( int aiType = AIBaseChannel::T_UNKNOW );

    int Encode( AIChunkEx& aoChunk );
    int Decode( AIChunkEx& aoChunk );
    size_t GetSize();
    
    void SetType( int aiType );
    int  GetType();
    
protected:
    int  ciType;
};

//clsHandShake
class clsHandShake
{
public:
    clsHandShake();
    
    int Encode( AIChunkEx& aoChunk );
    int Decode( AIChunkEx& aoChunk );
    size_t GetSize();

    void SetChannelID( int aiID )                     { ciChannelID = aiID; }
    void SetConnSlot( int aiSlot )                    { ciConnSlot = aiSlot; }
    void SetStat( int aiStat )                        { ciStat = aiStat; }
    
    int GetChannelID()                                { return ciChannelID; }
    int GetConnSlot()                                 { return ciConnSlot; }
    int GetStat()                                     { return ciStat; }
    
protected:
    int ciChannelID;
    int ciConnSlot;
    int ciStat;
};

/////////////////////////////////// Send / Recv function for Protocol ///////////////////////////////
template< class TProtocol >
int Send( AI_SYNCHANDLE atHandle, TProtocol& aoProtocol )
{
    int       liRetCode   = 0;
    AIChunkEx loChunk(aoProtocol.GetSize());
    
    aoProtocol.Encode(loChunk);
    
    // Send handshake command
    if ( loChunk.GetSize() > 0 && (liRetCode = ai_socket_senddata( atHandle, loChunk.BasePtr(), (int)loChunk.GetSize() )) != (int)loChunk.GetSize() )
    {
        return AI_SYNC_ERROR_SEND;
    }
    
    return AI_SYNC_NO_ERROR;
}
template< class TProtocol >
int Recv( AI_SYNCHANDLE atHandle, TProtocol& aoProtocol )
{
    int       liRetCode = 0;
    AIChunkEx loChunk( aoProtocol.GetSize() );
    
    // Resv handshake command
    liRetCode = ai_socket_recvdata( atHandle, loChunk.BasePtr(), (int)loChunk.GetSize(), AI_SYNC_TIMEOUT );
    if( liRetCode != (int)loChunk.GetSize() )
    {
        return AI_SYNC_ERROR_RECV;
    }
    
    loChunk.WritePtr( loChunk.GetSize() );
    
    AI_RETURN_IF( AI_SYNC_ERROR_DECODE, aoProtocol.Decode(loChunk) != 0 );
    
    return AI_SYNC_NO_ERROR;
}
int Recv( AI_SYNCHANDLE atHandle, clsCmdLineConnection& aoProtocol );

////////////////////////////////////////////////////////////////////////////////
// AICmdLineChannel
class AICmdLineChannel : public AIBaseChannel
{
public:
    static const char* STAG;
    
    typedef clsCmdLineConnection TConnection;
    typedef clsHandShake THandShake;
    
public:
    // Overwriteable method
    virtual int Send( AI_SYNCHANDLE atHandle, const char* apsData, size_t aiSize );
    virtual int Recv( AI_SYNCHANDLE atHandle, AIChunkEx& aoChunk, int aiTimeout );

protected:
    size_t DecodeDataSize( AIChunkEx& aoChunk );
};

////////////////////////////////////////////////////////////////////////////////
// AISyncChannel
class AISyncChannel : public AIBaseChannel
{
public:
    struct stHeader
    {
        int ciMsgID;
        int ciLenght;
    };
    
    typedef clsDefaultConnection TConnection;
    typedef clsHandShake THandShake;
    
public:
    // Overwriteable method
    virtual int Send( AI_SYNCHANDLE atHandle, const char* apsData, size_t aiSize );
    virtual int Recv( AI_SYNCHANDLE atHandle, AIChunkEx& aoChunk, int aiTimeout );
};

/////////////////////////////////////////////////////////////////////////////////////
//AISyncBaseServer
template< class TChannel = AISyncChannel >
class AISyncServer
{
public:
    struct stChannelConn
    {
        stChannelConn();
        
        AI_SYNCHANDLE ciHandle[2];
        int ciChannelID;
        int ciTimestamp;
    };
    
    enum { CHANNEL_CONN_BUFF_SIZE = 100 };
    
public:
    AISyncServer();
    virtual ~AISyncServer();
    
    int  Startup( const char* apsIp, int aiPort, int aiBackLog = 10 );
    void Close();
    bool IsClosed();
    
protected:
    static void* AcceptHandle( void* apParam );

    // Overwiteable interface
    virtual int Accept( AIBaseChannel* apoChannel ) = 0;

    stChannelConn* GetIdleChannelConn( int* apiSlot );
    AIBaseChannel* CreateChannel();
    
public:
    AI_SYNCHANDLE ctListenHandle;
    
    bool          cbIsClosed;
    
    // Accept handle thread
    pthread_t     ciThrd;
    
    // Channel connection array
    stChannelConn coChannelConn[CHANNEL_CONN_BUFF_SIZE];
    int           ciCurrSlot;
    int           ciChannelID;
};

/////////////////////////////////////////////////////////////////////////////////////
//AISyncClient
template< class TChannel = AISyncChannel >
class AISyncClient
{
public:
    AISyncClient();
    ~AISyncClient();
    
    int Connect( 
        const char* apsIp, int aiPort, TChannel& aoChannel, 
        int aiChannelType = AIBaseChannel::T_DUAL, int aiTimeout = AI_SYNC_TIMEOUT );

    void Close();
};

/////////////////////////////////////////////////////////////////////////////////////
//AISyncBaseServer Implement
template< class TChannel >
AISyncServer<TChannel>::stChannelConn::stChannelConn()
{
    memset( this, 0, sizeof( stChannelConn ) );
}

template< class TChannel >
AISyncServer<TChannel>::AISyncServer() : 
    ctListenHandle(AI_INVALID_SYNCHANDLE),
    cbIsClosed(true),
    ciThrd(0),
    ciCurrSlot(0),
    ciChannelID(0)
{
}

template< class TChannel >
AISyncServer<TChannel>::~AISyncServer()
{
    this->Close();
}

template< class TChannel >
int AISyncServer<TChannel>::Startup( const char* apsIp, int aiPort, int aiBackLog /* = 10 */ )
{
    int liRetCode = 0;
    
    if ( !this->IsClosed() )
    {
        return AI_SYNC_ERROR_ACTIVE;
    }
    
    ctListenHandle = ai_socket_listen( apsIp, aiPort, aiBackLog );
    if ( ctListenHandle <= 0 )
    {
        AI_SYNC_ERROR( "[Server]:Listen [Ip=%s/Port=%d] fail, [MSG=%s]",apsIp, aiPort, strerror(errno) );
        
        ctListenHandle = AI_INVALID_SYNCHANDLE;
        
        return -1;
    }
    
    cbIsClosed = false;
    
    if ( ( liRetCode = pthread_create( &ciThrd, NULL, AcceptHandle, this ) ) != 0 )
    {
        AI_SYNC_ERROR( "[Server]:Create AcceptHandle thread fail, [Ip=%s/Port=%d]/[MSG=%s]", apsIp, aiPort, strerror(errno) );
        
        return liRetCode;
    }

    return liRetCode;
}

template< class TChannel >
void AISyncServer<TChannel>::Close()
{
    cbIsClosed = true;
    ai_socket_close( ctListenHandle );
    pthread_join( ciThrd, NULL );
}

template< class TChannel >
bool AISyncServer<TChannel>::IsClosed()
{
    return cbIsClosed;
}

template< class TChannel >
typename AISyncServer<TChannel>::stChannelConn* AISyncServer<TChannel>::GetIdleChannelConn( int* apiSlot /* NULL */ )
{
    for ( int liIdx = ciCurrSlot; liIdx < CHANNEL_CONN_BUFF_SIZE; liIdx++ )
    {
        if ( coChannelConn[liIdx].ciTimestamp == 0 ||
             (time(NULL) - coChannelConn[liIdx].ciTimestamp ) > CHANNEL_CONN_TIMEOUT )
        {
            ciCurrSlot = ( liIdx + 1 ) % CHANNEL_CONN_BUFF_SIZE;
            
            if ( apiSlot != NULL ) *apiSlot = liIdx;
                
            return &coChannelConn[liIdx];
        }
    }
    
    return NULL;
}

template< class TChannel >
AIBaseChannel* AISyncServer<TChannel>::CreateChannel()
{
    AIBaseChannel* lpoChannel = NULL;
    AI_NEW_ASSERT( lpoChannel, TChannel );
    //printf( "new %p\n", lpoChannel );
    return lpoChannel;
}

template< class TChannel >
void* AISyncServer<TChannel>::AcceptHandle( void* apParam )
{
    int               liRetCode         = 0;
    int               liRemotePort      = 0;
    AIBaseChannel*    lpoChannel        = NULL;
    AISyncServer*     lpoServer         = (AISyncServer*)apParam;
    stChannelConn*    lpoChannelConn    = NULL;
    char              lsRemoteIp[32];
    AI_SYNCHANDLE     ltRemoteHandle    = 0;
    typename TChannel::TConnection loConnection;
    typename TChannel::THandShake  loHandShake;
    
    while( !lpoServer->IsClosed() )
    {
        if( (ltRemoteHandle = ai_socket_accept(
            lpoServer->ctListenHandle, lsRemoteIp, &liRemotePort)) < 0)
        {
            if ( !lpoServer->IsClosed() )
            {
                AI_SYNC_ERROR( "[Server]:Accept first connection fail, [ListenHandle=%d]/[RemoteHandle=%d]/[MSG=%s]", 
                    lpoServer->ctListenHandle, ltRemoteHandle, strerror(errno) );
            }
            continue;
        }
        
        // Recv connection request
        if ( ( liRetCode = Recv( ltRemoteHandle, loConnection ) ) != 0 )
        {
            ai_socket_close(ltRemoteHandle);
            AI_SYNC_ERROR( "[Server]:Recv Connected request from [%s:%d] fail, MSGCODE:%d", 
                lsRemoteIp, liRemotePort, liRetCode );
            continue;
        }
        
        AI_SYNC_INFO( "[Server]:Recv Connected request from [%s:%d], [Type=%d]", 
            lsRemoteIp, liRemotePort, loConnection.GetType() );
                
        if ( loConnection.GetType() == AIBaseChannel::T_SINGLE )
        {
            // Single chunnel connection
            AI_SYNCHANDLE liHandle[2] = {ltRemoteHandle, ltRemoteHandle};
            lpoChannel = lpoServer->CreateChannel();
            if ( ( liRetCode = lpoChannel->Initialize( liHandle, lsRemoteIp, liRemotePort, lpoServer->ciChannelID++ ) ) != 0 )
            {
                ai_socket_close(ltRemoteHandle);
                
                AI_SYNC_ERROR( "[Server]:  initialize channel for new connection fail, [ListenHandle=%d]/[RemoteHandle=%d]/[MSG=%s]", 
                    lpoServer->ctListenHandle, ltRemoteHandle, strerror(errno) );
            }
            else
            {
                lpoServer->Accept( lpoChannel );
                lpoChannel->Ready();
            }
            continue;
        }
        
        // Double chunnel connection, and start handshake now
        // Recv handshake
        if ( ( liRetCode = Recv( ltRemoteHandle, loHandShake ) ) != 0 )
        {
            ai_socket_close(ltRemoteHandle);
            AI_SYNC_ERROR( "[Server]:Recv handshake request from [%s:%d] fail, MSGCODE:%d", 
                lsRemoteIp, liRemotePort, liRetCode );
            continue;
        }
        
        if ( loHandShake.GetChannelID() < 0 )
        {
            // Channel first connection here
            int liSlot = 0;
            if ( ( lpoChannelConn = lpoServer->GetIdleChannelConn( &liSlot ) ) == NULL )
            {
                loHandShake.SetStat( AI_SYNC_ERROR_NO_CONN_SLOT );
                if ( ( liRetCode = Send( ltRemoteHandle, loHandShake ) ) != 0 )
                {
                    ai_socket_close(ltRemoteHandle);
                    AI_SYNC_ERROR( "[Server]:Send handshake response [NO_ENOUGH_SLOT] to [%s:%d] fail, MSGCODE:%d", 
                        lsRemoteIp, liRemotePort, liRetCode );
                }
                AI_SYNC_ERROR( "%s", "[Server]:Allocate Idle Channel Connection slot fail" );
                continue;
            }

            loHandShake.SetChannelID( lpoServer->ciChannelID++ );
            loHandShake.SetConnSlot( liSlot );
            loHandShake.SetStat( AI_SYNC_NO_ERROR );
            
            if ( ( liRetCode = Send( ltRemoteHandle, loHandShake ) ) != 0 )
            {
                ai_socket_close(ltRemoteHandle);
                AI_SYNC_ERROR( "[Server]:Send handshake response [GET_SLOT_OK] to [%s:%d] fail, MSGCODE:%d", 
                    lsRemoteIp, liRemotePort, liRetCode );
                continue;
            }
            
            lpoChannelConn->ciHandle[0] = ltRemoteHandle;
            lpoChannelConn->ciChannelID = loHandShake.GetChannelID();
            lpoChannelConn->ciTimestamp = time(NULL);
            
            AI_SYNC_INFO( "[Server]:Allocate Idle Channel Connection slot success, [Handle=%d]/[ChannelID=%d]", 
                ltRemoteHandle, loHandShake.GetChannelID() );
        }
        else
        {
            if ( loHandShake.GetConnSlot() < 0 || loHandShake.GetConnSlot() >= CHANNEL_CONN_BUFF_SIZE )
            {
                loHandShake.SetStat( AI_SYNC_ERROR_INVALID_SLOT );
                if ( ( liRetCode = Send( ltRemoteHandle, loHandShake ) ) != 0 )
                {
                    AI_SYNC_ERROR( "[Server]:Send handshake response [INVALID_SLOT] to [%s:%d] fail", 
                        lsRemoteIp, liRemotePort );
                }
                
                ai_socket_close(ltRemoteHandle);
                
                AI_SYNC_ERROR( "[Server]:Authenticate Channel Connection fail, [Slot=%d]", loHandShake.GetConnSlot() );
                
                continue;
            }
            
            lpoChannelConn = &lpoServer->coChannelConn[loHandShake.GetConnSlot()];
            if ( lpoChannelConn->ciChannelID != loHandShake.GetChannelID() )
            {
                loHandShake.SetStat( AI_SYNC_ERROR_TIMEOUT );
                if ( ( liRetCode = Send( ltRemoteHandle, loHandShake ) ) != 0 )
                {
                    AI_SYNC_ERROR( "[Server]:Send handshake response [TIMEOUT] to [%s:%d] fail, MSGCODE:%d", 
                        lsRemoteIp, liRemotePort, liRetCode );
                }
                
                ai_socket_close(ltRemoteHandle);
                
                AI_SYNC_ERROR( "[Server]:Authenticate Channel Connection fail, [S_ChannelID=%d]/[C_ChannelID=%d]", 
                    lpoChannelConn->ciChannelID, loHandShake.GetChannelID() );
                
                continue;
            }
            
            lpoChannelConn->ciHandle[1] = ltRemoteHandle;
            lpoChannelConn->ciTimestamp = 0;
            lpoChannel = lpoServer->CreateChannel();
            if ( ( liRetCode = lpoChannel->Initialize( lpoChannelConn->ciHandle, lsRemoteIp, liRemotePort, lpoChannelConn->ciChannelID ) ) != 0 )
            {
                loHandShake.SetStat( liRetCode );
                if ( ( liRetCode = Send( ltRemoteHandle, loHandShake ) ) != 0 )
                {
                    AI_SYNC_ERROR( "[Server]:Send handshake response [INITIALIZE_FAIL] to [%s:%d] fail, MSGCODE:%d", 
                        lsRemoteIp, liRemotePort, liRetCode );
                }
                
                ai_socket_close(lpoChannelConn->ciHandle[0]);
                ai_socket_close(lpoChannelConn->ciHandle[1]);
                
                AI_SYNC_ERROR( "[Server]:  initialize channel for new connection fail, [ListenHandle=%d]/[RemoteHandle=%d]/[MSG=%s]", 
                    lpoServer->ctListenHandle, ltRemoteHandle, strerror(errno) );

                AI_DELETE( lpoChannel );
                
                continue;
            }
            
            // Send response confirm
            loHandShake.SetStat( AI_SYNC_NO_ERROR );
            if ( ( liRetCode = Send( ltRemoteHandle, loHandShake ) ) != 0 )
            {
                AI_SYNC_ERROR( "[Server]:Send handshake response [HANDSHAKE_OK] to [%s:%d] fail, MSGCODE:%d", 
                    lsRemoteIp, liRemotePort, liRetCode );

                AI_DELETE( lpoChannel );
                
                continue;
            }
            
            AI_SYNC_INFO( "[Server]:Channel Connection Handshake success, [Handle=%d]/[ChannelID=%d]", 
                ltRemoteHandle, lpoChannel->GetChannelID() );
            
            lpoServer->Accept( lpoChannel );
            lpoChannel->Ready();
        }
    }
    
    return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////
//AISyncClient
template< class TChannel >
AISyncClient<TChannel>::AISyncClient()
{
}

template< class TChannel >
AISyncClient<TChannel>::~AISyncClient()
{
}

template< class TChannel >
int AISyncClient<TChannel>::Connect( 
        const char* apsIp, int aiPort, TChannel& aoChannel, 
        int aiChannelType /* = AIBaseChannel::T_DUAL */, int aiTimeout /* = 0 */ )
{
    int           liRetCode   = 0;
    AI_SYNCHANDLE ltHandle[2] = {-1, -1};
    typename TChannel::TConnection loConnection;
    typename TChannel::THandShake  loHandShake;

    do
    {
        if ( (ltHandle[0] = ai_socket_connect( apsIp, aiPort, aiTimeout )) < 0 )
        {
            AI_SYNC_ERROR( "[Client]:Connection to [Ip=%s/Port=%d] fail, [MSG=%s]", apsIp, aiPort, strerror(errno) );
            liRetCode = AI_SYNC_ERROR_SOCKET;
            break;
        }
        
        loConnection.SetType(aiChannelType);
        if ( ( liRetCode = Send( ltHandle[0], loConnection ) ) != 0 )
        {
            AI_SYNC_ERROR( "[Client]:Send Connected request to [%s:%d][Handle 0] fail, MSGCODE:%d", apsIp, aiPort, liRetCode );
            
            liRetCode = AI_SYNC_ERROR_SEND;
            break;
        }
        
        if ( aiChannelType == AIBaseChannel::T_SINGLE )
        {
            // Connected by single channel
            ltHandle[1] = ltHandle[0];
            if ( ( liRetCode = aoChannel.Initialize( ltHandle, apsIp, aiPort, 0, true ) ) != 0 )
            {
                AI_SYNC_ERROR(  "[Client]:Initialize channel for connection fail, [Ip=%s]/[Port=%d]/[MSGCODE=%d]", 
                    apsIp, aiPort, liRetCode );
    
                break;
            }
            
            return AI_SYNC_NO_ERROR;
        }
        
        // Connected by double channel, and start handshake now
        // Send Handshake package
        loHandShake.SetChannelID(-1);
        if ( ( liRetCode = Send( ltHandle[0], loHandShake ) ) != 0 )
        {
            AI_SYNC_ERROR( "[Client]:Send handshake request to [%s:%d] fail, MSGCODE:%d", apsIp, aiPort, liRetCode );
            
            liRetCode = AI_SYNC_ERROR_SEND;
            break;
        }
        
        // Recv Handshake permission
        if ( ( liRetCode = Recv( ltHandle[0], loHandShake ) ) != 0 )
        {
            AI_SYNC_ERROR( "[Client]:Recv handshake permission from [%s:%d] fail", apsIp, aiPort );
            
            liRetCode = AI_SYNC_ERROR_RECV;
            break;
        }
        
        // Check Handshake stat
        if ( loHandShake.GetStat() != AI_SYNC_NO_ERROR )
        {           
            AI_SYNC_ERROR( "[Client]:Handshake reject, MSGCODE:%d", loHandShake.GetStat() );
            
            liRetCode = loHandShake.GetStat();
            break;
        }
        
        // Connect second handle
        if ( (ltHandle[1] = ai_socket_connect( apsIp, aiPort, aiTimeout )) <= 0 )
        {
            AI_SYNC_ERROR( "[Client]:Second connection to [Ip=%s/Port=%d] fail, [MSG=%s]", apsIp, aiPort, strerror(errno) );
            liRetCode = AI_SYNC_ERROR_SOCKET;
            break;
        }
        
        // Send connection type
        if ( ( liRetCode = Send( ltHandle[1], loConnection ) ) != 0 )
        {
            AI_SYNC_ERROR( "[Client]:Send Connected request to [%s:%d][Handle 1] fail, MSGCODE:%d", apsIp, aiPort, liRetCode );
            
            liRetCode = AI_SYNC_ERROR_SEND;
            break;
        }
        
        // Send Handshake permission
        if ( ( liRetCode = Send( ltHandle[1], loHandShake ) ) != 0 )
        {
            AI_SYNC_ERROR( "[Client]:Send handshake permission to [%s:%d] fail, MSGCODE:%d", apsIp, aiPort, liRetCode );
            
            liRetCode = AI_SYNC_ERROR_SEND;
            break;
        }
        
        // Recv Handshake permission
        if ( ( liRetCode = Recv( ltHandle[1], loHandShake ) ) != 0 )
        {
            AI_SYNC_ERROR( "[Client]:Recv handshake permission from [%s:%d] fail, MSGCODE:%d", apsIp, aiPort, liRetCode );
            
            liRetCode = AI_SYNC_ERROR_RECV;
            break;
        }
        
        if ( loHandShake.GetStat() != AI_SYNC_NO_ERROR )
        {
            AI_SYNC_ERROR( "[Client]:Handshake reject, MSGCODE:%d", loHandShake.GetStat() );
            
            liRetCode = loHandShake.GetStat();
            break;
        }
        
        AISwapValue( ltHandle[0], ltHandle[1] );
        if ( ( liRetCode = aoChannel.Initialize( ltHandle, apsIp, aiPort, 0, true ) ) != 0 )
        {
            AI_SYNC_ERROR(  "[Client]:Initialize channel for connection fail, [Ip=%s]/[Port=%d]/[MSGCODE=%d]", 
                apsIp, aiPort, liRetCode );
    
            break;
        }
        
        aoChannel.Ready();
        
        return AI_SYNC_NO_ERROR;
    }
    while( false );
    
    ai_socket_close(ltHandle[0]);
    ai_socket_close(ltHandle[1]);
                    
    return liRetCode;
}

template< class TChannel >
void AISyncClient<TChannel>::Close()
{
}

///end namespace
AIBC_NAMESPACE_END

#endif //__AI_SYNC_TCP_H__
