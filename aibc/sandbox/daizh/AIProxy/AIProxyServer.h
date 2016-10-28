
#ifndef __AIPROXY_SERVER_H__
#define __AIPROXY_SERVER_H__

#include "AISyncTcp.h"
#include "AISynch.h"
#include "AITypeObject.h"
#include "AIChannelPool.h"
#include "AIProxyRouter.h"
#include "AIProxyProtocol.h"

START_AIPROXY_NAMESPACE

////////////////////////////////////////// base server /////////////////////////////////
class AIServer
{    
public:
    virtual AIProtocol* GetProtocol( void ) = 0;
    virtual int Send( const char* apcAddressee, const char* apcData, size_t aiSize ) = 0;
    
    virtual int ChannelInput( AIChannelPtr aoChannelPtr, const char* apcData, size_t aiSize ) = 0;
    virtual int ChannelError( AIChannelPtr aoChannelPtr, int aiErrno ) = 0;
    virtual int ChannelExit( AIChannelPtr aoChannelPtr ) = 0;
};

////////////////////////////////////////// input server ////////////////////////////////
class AIProxyChannel : public AIBaseChannel
{
public:
    AIProxyChannel( CHANNEL_MODE aiMode = AIBaseChannel::REACTIVE );
    virtual ~AIProxyChannel();
    
    /// Channel Data Event
    virtual int Input( const char* apcData, size_t aiSize );
    virtual int Error( int aiErrno );
    virtual int Exit();
    
    /// Channel State Event
    virtual void TimerEvent( AIBaseChannel::TState aoState );
    virtual void ChangeEvent( AIBaseChannel::TState aoState );
    
    void RegisterTimerEvent( AIChannelEvent* apoEvent );
    void RegisterChangeEvent( AIChannelEvent* apoEvent );
    
    /// Channel Protocol Interface
    virtual int SendTo( AISocketHandle atHandle, const char* apsData, size_t aiSize );
    virtual int RecvFrom( AISocketHandle atHandle, AIChunkEx& aoBuffer, int aiTimeout = AI_SYNC_TIMEOUT );
    
    /// Set/Get
    void SetServer( AIServer* apoServer );
    void SetProtocol( AIProtocol* apoProtocol );
    void SetGroupName( const char* apcName );
    
    const char* GetGroupName();

protected:
    void RegisterEvent( AIChannelEvent** apoEventArray, AIChannelEvent* apoEvent );
    void DoEvent( AIChannelEvent** apoEventArray, AIBaseChannel::TState aoState );
    void ClearEvent( AIChannelEvent** apoEventArray );
    
protected:
    AIServer*       cpoServer;
    AIProtocol*     cpoProtocol;
    
    AIChannelEvent* cpoTimerEvent[AI_MAX_EVENT_CNT];
    AIChannelEvent* cpoChangeEvent[AI_MAX_EVENT_CNT];
    
    char            csGroupName[AI_MAX_NAME_LEN];
};

/////////////////////////////////////////////////////////////////////////////////////////
class AIInnerServer : public AIServer, public AIBaseAcceptor
{
public:
    AIInnerServer( AIInnerProtocol* apoProtocol );
    virtual ~AIInnerServer();

    virtual AIProtocol* GetProtocol( void );
    virtual int Send( const char* apcAddressee, const char* apcData, size_t aiSize );

    /// Process Channel request
    virtual int ChannelInput( AIChannelPtr aoChannelPtr, const char* apcData, size_t aiSize );
    virtual int ChannelError( AIChannelPtr aoChannelPtr, int aiErrno );
    virtual int ChannelExit( AIChannelPtr aoChannelPtr );
    virtual int ChannelAccept( AIChannelPtr aoChannelPtr );
    
//operator
protected:
    ///Over write AIBaseAcceptor interface
    virtual AIBaseChannel* CreateChannel();
    virtual int Accept( AIBaseChannel* apoChannel );
    
//attribute
protected:
    AIChannelPool    coChannelPool;
    AIInnerProtocol* cpoProtocol;
};

////////////////////////////////////////// output server ////////////////////////////////
class AIOuterServer : public AIServer
{
public:
    AIOuterServer( AIOuterProtocol* apoProtocol );
    virtual ~AIOuterServer();
    
    virtual AIProtocol* GetProtocol( void );
    virtual int Send( const char* apcAddressee, const char* apcData, size_t aiSize );
    
    int AddConnection( const char* apcGroupName, const char* apcIpAddr, int aiPort, size_t aiCount = 1, int aiTimeOut = 10 );
    int UpdateConnection( const char* apcGroupName, const char* apcIpAddr, int aiPort, size_t aiCount, int aiTimeOut = 10 );
    size_t GetConnectionCnt( const char* apcGroupName );
    AIChannelPtr GetConnection( const char* apcGroupName );

    /// Process Channel request
    virtual int ChannelInput( AIChannelPtr aoChannelPtr, const char* apcData, size_t aiSize );
    virtual int ChannelError( AIChannelPtr aoChannelPtr, int aiErrno );
    virtual int ChannelExit( AIChannelPtr aoChannelPtr );
    virtual int ChannelClose( AIChannelPtr aoChannelPtr );
    virtual int ChannelConnected( AIChannelPtr aoChannelPtr );

//operator
protected:
    virtual AIProxyChannel* CreateChannel();

//attribute
protected:
    AIChannelPool    coChannelPool;
    
    AIBaseConnector  coConnector;
    AIOuterProtocol* cpoProtocol;
};

////////////////////////////////////////////// sync output server ////////////////////////////////////////
class AISyncOuterServer : public AIOuterServer
{
public:
    AISyncOuterServer( AIOuterProtocol* apoProtocol );
    virtual ~AISyncOuterServer();
    
    virtual int Send( const char* apcAddressee, const char* apcMsgID, const char* apcData, size_t aiSize );

//operator
protected:
    virtual AIProxyChannel* CreateChannel();
};

////////////////////////////////////////// Global interface ////////////////////////////////////////
template< class TProtocolPtr >
TProtocolPtr GetOuterProtocol() { return dynamic_cast<TProtocolPtr>( GetOuterServer<AIOuterServer*>()->GetProtocol() ); }

class AIInnerServer;
template< class TProtocolPtr >
TProtocolPtr GetInnerProtocol() { return dynamic_cast<TProtocolPtr>( GetInnerServer<AIInnerServer*>()->GetProtocol() ); }


END_AIPROXY_NAMESPACE

#endif // __AIPROXY_SERVER_H__
