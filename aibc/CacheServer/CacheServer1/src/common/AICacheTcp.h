
#ifndef __AILIB_CACHETCP_H__
#define __AILIB_CACHETCP_H__

#include "AISynch.h"
#include "AIChunkEx.h"
#include "AISocket.h"
#include "AITypeObject.h"
#include "AITime.h"
#include "cacheserver/CacheUtility.h"

AI_CACHE_NAMESPACE_START

typedef int AISocketHandle;

#if !defined(AI_INVALID_HANDLE)
const   int AI_INVALID_HANDLE = -1;
#endif

// Error no define
const int AI_SYNC_NO_ERROR           = 0;
const int AI_SYNC_ERROR_SOCKET       = -80201;
const int AI_SYNC_ERROR_LISTEN       = -80202;
const int AI_SYNC_ERROR_CONN         = -80203;
const int AI_SYNC_ERROR_SEND         = -80204;
const int AI_SYNC_ERROR_RECV         = -80205;
const int AI_SYNC_ERROR_BROKEN       = -80206;
const int AI_SYNC_ERROR_TIMEOUT      = -80207;
const int AI_SYNC_ERROR_LENGTH       = -80208;
const int AI_SYNC_ERROR_ACTIVE       = -80209;
const int AI_SYNC_ERROR_THREAD       = -80210;
const int AI_SYNC_ERROR_DECODE       = -80211;

// Const Declare
// Time out value
const int AI_SYNC_TIMEOUT = 50;

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
        AIBC::AITime_t GetStartTime()           { return ciStartTime; }
        AIBC::AITime_t GetLastSendTime()        { return ciLastSendTime; }
        AIBC::AITime_t GetLastRecvTime()        { return ciLastRecvTime; }
        size_t   GetSendCount()           { return ciSendCount; }
        size_t   GetRecvCount()           { return ciRecvCount; }
        
        AIBC::AITime_t ciStartTime;
        AIBC::AITime_t ciLastSendTime;
        AIBC::AITime_t ciLastRecvTime;
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
    int Recv( AIBC::AIChunkEx& aoResponse, int aiTimeout = AI_SYNC_TIMEOUT );

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
    virtual int  RecvFrom( AISocketHandle atHandle, AIBC::AIChunkEx& aoBuffer, int aiTimeout = AI_SYNC_TIMEOUT ) = 0;
    
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
    AIBC::AITypeObject<int, AIBC::AIMutexLock> ciReferenceCount;
    
    // Stat.
    TState        coState;
    
    // Input handle thread
    pthread_t     ciThrd;
    
    // Channel mutex
    AIBC::AIMutexLock   coSendLock;
    AIBC::AIMutexLock   coRecvLock;
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
    
    typedef AIBaseChannel* TPtrType;
    
public:
    AIChannelPtr( );
    AIChannelPtr( TPtrType apoChannel );
    AIChannelPtr( const AIChannelPtr& aoRhs );
    ~AIChannelPtr( );
    
    void operator = ( TPtrType apoChannel );
    void operator = ( const AIChannelPtr& aoRhs );
    bool operator == ( TPtrType apoChannel );
    bool operator == ( const AIChannelPtr& aoRhs );
    bool operator != ( TPtrType apoChannel );
    bool operator != ( const AIChannelPtr& aoRhs );
    
    TPtrType operator -> ();
    operator TPtrType ();
    
protected:
    TPtrType cpoChannel;
};

////////////////////////////////////////////////////////////////////////////////////
class AIChannelEvent
{
public:
    virtual bool Condition( AIBaseChannel::TState aoState ) = 0;
    virtual void Action( AIChannelPtr aoChannelPtr )        = 0;
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
    
    AISocketHandle GetListenHandle()                       { return this->ctListenHandle; }

protected:
    static void* AcceptHandleThread( void* apParam );
    
    /// Acceptor Hook method
    virtual int Accept( AIChannelPtr aoChannelPtr ) = 0;
    virtual AIBaseChannel* CreateChannel() = 0;
    
    // Overwiteable interface
    virtual void AcceptHandle();
    
    int GenChannelID()                                      { return this->ciChannelID++ % 100000000; }

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
    int GenChannelID()                                     { return this->ciChannelID++ % 100000000; }
    
protected:
    AIBC::AITypeObject<size_t, AIBC::AIMutexLock> ciChannelID;
};

AI_CACHE_NAMESPACE_END

#endif //#__AILIB_SYNCTCP_H__
