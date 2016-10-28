
#ifndef __AI_CHANNEL_H__
#define __AI_CHANNEL_H__

#include "AISynch.h"
#include "AIChunkEx.h"
#include "AISocket.h"
#include "AITypeObject.h"
#include "AITime.h"

#define AI_SYNC_TRACE( format, ... )
#define AI_SYNC_DEBUG( format, ... )
#define AI_SYNC_ERROR( format, ... ) fprintf( stderr, format "\n", __VA_ARGS__ );

typedef int AISocketHandle;
const int   AI_INVALID_HANDLE = -1;

// Error no define
const int AI_SYNC_NO_ERROR           = 0;
const int AI_SYNC_ERROR_SOCKET       = -80201;
const int AI_SYNC_ERROR_SEND         = -80202;
const int AI_SYNC_ERROR_RECV         = -80203;
const int AI_SYNC_ERROR_BROKEN       = -80204;
const int AI_SYNC_ERROR_TIMEOUT      = -80207;
const int AI_SYNC_ERROR_LENGTH       = -80208;
const int AI_SYNC_ERROR_ACTIVE       = -80209;
const int AI_SYNC_ERROR_THREAD       = -80210;
const int AI_SYNC_ERROR_DECODE       = -80211;

// Const Declare
// Time out value
const int AI_SYNC_TIMEOUT = 1;

// Channel buffer time out value
const int CHANNEL_CONN_TIMEOUT = 10;

const int CHANNEL_PENDING_TIMEOUT = 10;

const int AI_SYNC_MAX_BUFFER_SIZE = 4096;
// End const

/////////////////////////////////////////////////////////////////////////////////////
//AIBaseChannel
class AIBaseChannel
{
public:
    enum CHANNEL_MODE{
        //NORMAL : 
        NORMAL   = 0,
        
        //REACTIVE : 
        REACTIVE = 1 
    };
    
    struct stState
    {
        // Stat.
        stState();
        AITime_t GetStartTime()           { return ciStartTime; }
        AITime_t GetLastSendTime()        { return ciLastSendTime; }
        AITime_t GetLastRecvTime()        { return ciLastRecvTime; }
        size_t   GetSendCount()           { return ciSendCount; }
        size_t   GetRecvCount()           { return ciRecvCount; }
        
        AITime_t ciStartTime;
        AITime_t ciLastSendTime;
        AITime_t ciLastRecvTime;
        size_t   ciSendCount;
        size_t   ciRecvCount;
    };
    
    typedef stState TState;
    
public:
    AIBaseChannel( CHANNEL_MODE aiMode = REACTIVE );
    virtual ~AIBaseChannel();
    
    int  Initialize( AISocketHandle atHandle, int aiChannelID );
    void CloseHandle();
    void Close();
    
    int Send( const char* apcData, size_t aiSize );
    int Recv( AIChunkEx& aoResponse, int aiTimeout = AI_SYNC_TIMEOUT );

    bool IsClosed();
    bool IsReactive();
    
    void Reference();
    void Release();
    
    void Ready()                                    { cbIsReady = true; }
    bool IsReady()                                  { return cbIsReady; } 
    
    AISocketHandle GetHandle()                      { return ctHandle; }
    const char* GetRemoteIp()                       { return csRemoteIp; }
    int  GetRemotePort()                            { return ciRemotePort; }
    int  GetChannelID()                             { return ciChannelID; }
    void GetState( TState& aoState )                { aoState = coState; }
    
protected:
    /// Channel Hook method
    virtual int  Input( const char* apcData, size_t aiSize );
    virtual int  Error( int aiErrno );
    virtual int  Exit();
    
    /// State Event
    virtual void TimerEvent( TState aoState );
    virtual void ChangeEvent( TState aoState );
    
    /// Reserve transmit interface for protocol
    virtual int  SendTo( AISocketHandle atHandle, const char* apcData, size_t aiSize ) = 0;
    virtual int  RecvFrom( AISocketHandle atHandle, AIChunkEx& aoBuffer, int aiTimeout = AI_SYNC_TIMEOUT ) = 0;
    
    /// 
    virtual void InputHandle();
    
    // Thread handle method
    static void* InputHandleThread( void* apParam );

protected:
    AISocketHandle ctHandle;
    char           csRemoteIp[20];
    int            ciRemotePort;
    int            ciChannelID;
    CHANNEL_MODE   ciMode;
    
    // Status flag
    bool          cbIsClosed;
    bool          cbIsExited;
    bool          cbIsReady;
    
    // Reference counter
    AITypeObject<int, AIMutexLock> ciReferenceCount;
    
    // Stat.
    TState        coState;
    
    // Input handle thread
    pthread_t     ciThrd;
    
    // Channel mutex
    AIMutexLock   coSendLock;
    AIMutexLock   coRecvLock;
};

/////////////////////////////////////////////////////////////////////////////////////
class AIChannelPtr
{
public:
    template< class T >
    class To
    {
    public:
        To( AIChannelPtr& aoChannelPtr ) :
            coChannelPtr(aoChannelPtr)
        {
        }
        T operator -> ()
        {
            return static_cast<T>(this->coChannelPtr.operator->());
        }
    protected:
        AIChannelPtr& coChannelPtr;
    };
    
public:
    AIChannelPtr( );
    AIChannelPtr( AIBaseChannel* apoChannel );
    AIChannelPtr( const AIChannelPtr& aoRhs );
    ~AIChannelPtr( );
    
    void operator = ( AIBaseChannel* apoChannel );
    void operator = ( const AIChannelPtr& aoRhs );
    bool operator == ( AIBaseChannel* apoChannel );
    bool operator == ( const AIChannelPtr& aoRhs );
    bool operator != ( AIBaseChannel* apoChannel );
    bool operator != ( const AIChannelPtr& aoRhs );
    
    AIBaseChannel* operator -> ();
    
protected:
    AIBaseChannel* cpoChannel;
};

class AIChannelEvent
{
public:
    virtual bool Condition( AIBaseChannel::TState aoState ) = 0;
    virtual void Action( AIChannelPtr aoChannelPtr )     = 0;
};

/////////////////////////////////////////////////////////////////////////////////////
//AIBaseAcceptor
class AIBaseAcceptor
{
public:
    AIBaseAcceptor();
    virtual ~AIBaseAcceptor();
    
    int  Startup( const char* apsIp, int aiPort, int aiBackLog = 10 );
    void Close();
    bool IsClosed();
    
protected:
    static void* AcceptHandleThread( void* apParam );
    
    /// Acceptor Hook method
    virtual int Accept( AIBaseChannel* apoChannel ) = 0;
    virtual AIBaseChannel* CreateChannel() = 0;
    
    // Overwiteable interface
    virtual void AcceptHandle();

public:
    AISocketHandle ctListenHandle;
    
    bool          cbIsClosed;
    
    // Accept handle thread
    pthread_t     ciThrd;
    
    size_t        ciChannelID;
};


/////////////////////////////////////////////////////////////////////////////////////
//AIBaseConnector
class AIBaseConnector
{
public:
    AIBaseConnector();
    virtual ~AIBaseConnector();
    
    virtual int Connect( const char* apsIp, int aiPort, AIBaseChannel& aoChannel, int aiTimeout = AI_SYNC_TIMEOUT );

    void Close();
    
protected:
    AITypeObject<size_t, AIMutexLock> ciChannelID;
};

#define HANDLE_WRITE( handle, dataptr, datasize, timeout ) \
    if ( (::ai_socket_senddata( handle, (dataptr), (int)(datasize), timeout )) != (int)datasize ) \
    { \
        return AI_SYNC_ERROR_SEND; \
    }

#define HANDLE_READ( handle, bufferptr, buffersize, timeout ) \
    { \
        int __liRetCode = 0; \
        if ( (__liRetCode = ::ai_socket_recvdata( handle, (bufferptr), (int)(buffersize), timeout )) != (int)buffersize ) \
        { \
            if ( __liRetCode > 0 || __liRetCode < 0 ) \
            { \
                return AI_SYNC_ERROR_RECV; \
            } \
            else \
            { \
                return AI_SYNC_ERROR_TIMEOUT; \
            } \
        } \
    }

#define HANDLE_READ_LINE( handle, bufferptr, buffersize, timeout ) \
    { \
        int __liRetCode = 0; \
        if ( (__liRetCode = ::ai_socket_recvline( handle, (bufferptr), (int)(buffersize), timeout )) <= 0 ) \
        { \
            if ( __liRetCode < 0 ) \
            { \
                return AI_SYNC_ERROR_RECV; \
            } \
            else \
            { \
                return AI_SYNC_ERROR_TIMEOUT; \
            } \
        } \
    }

#endif //#__AI_CHANNEL_H__
