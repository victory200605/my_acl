
#ifndef ANF_IOSOCKCONNECTOR_H
#define ANF_IOSOCKCONNECTOR_H

#include "acl/SockAddr.h"
#include "acl/ThreadManager.h"
#include "anf/IoConnector.h"
#include "anf/WakeablePoll.h"
#include "anf/ConcurrentQueue.h"
#include "anf/future/ConnectFuture.h"

ANF_NAMESPACE_START

/**
 * Connects to endpoint, communicates with the server, and fires events to IoHandler.
 * You should connect to the desired socket address to start communication,
 * and then events for incoming connections will be sent to the specified default IoHandler.
 */
class CIoSockConnector : public CIoConnector
{
public:
    struct CProgressNode
    {
        CConnectFuture::PointerType moFuture;
        acl::CTimestamp moTimestamp;
    };
    
    typedef std::map<apl_handle_t, CProgressNode> ProgressHandleMapType;
    typedef TConcurrentQueue<CConnectFuture::PointerType> QueueType;
    
public:
    /**
     * Default constructor
     */
    CIoSockConnector(void);
    
    /**
     * Default destructor
     */
    virtual ~CIoSockConnector(void);
    
    /**
     * Releases any resources allocated by this service.  Please note that
     * this method might block as long as there are any sessions managed by
     * this service.
     */
    virtual apl_int_t Dispose(void);
    
protected:
    /**
     * Connect to the specified local address and start to accept incoming
     * connections.
     *
     * @return connect future object, application can wait for completion synchronism
     */
    virtual CConnectFuture::PointerType Connect0( acl::CSockAddr const& aoAddress );

protected:
    /**
     * Startup connector service
     */
    apl_int_t StartupConnector(void);
    
    /**
     * Wakeup thread-scheduler
     */
    void Wakeup(void);
    
    CIoSession* NewSession(void);
    
//thread-scheduler
protected:
    apl_ssize_t RegisterNew(void);

    apl_ssize_t ProcessConnections(void);
    
    apl_ssize_t ProcessTimedOutSessions( acl::CTimeValue const& aoTimeout );
    
    void FinishConnect( CConnectFuture::PointerType& aoFuture, apl_handle_t aiHandle );
    
    static void* Run(void* apvParam );

private:
    QueueType moRegisterQueue;
    
    ProgressHandleMapType moProgressHandles;

    CWakeablePoll moPoll;

    acl::CLock moLock;
        
    acl::CThreadManager moThreadManager;
};

ANF_NAMESPACE_END

#endif//ANF_IOSOCKCONNECTOR_H

