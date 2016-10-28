
#ifndef ANF_IOSOCKACCEPTOR_H
#define ANF_IOSOCKACCEPTOR_H

#include "acl/SockAddr.h"
#include "acl/ThreadManager.h"
#include "anf/IoAcceptor.h"
#include "anf/WakeablePoll.h"
#include "anf/ConcurrentQueue.h"
#include "anf/future/AcceptorOperationFuture.h"

ANF_NAMESPACE_START

/**
 * This class handle the logic behind binding, accepting and disposing the server sockets.
 */
class CIoSockAcceptor : public CIoAcceptor
{
public:
    typedef std::map<apl_handle_t, acl::CSockAddr> BindingAddressMapType;
    typedef TConcurrentQueue<CAcceptorOperationFuture::PointerType> QueueType;
    
public:
    CIoSockAcceptor(void);
    
    virtual ~CIoSockAcceptor(void);
    
    virtual apl_int_t Dispose(void);
    
protected:
    /**
     * Binds to the specified local address and start to accept incoming
     * connections.
     *
     * @return If successful, shall return zero; otherwise -1 shall be returned, and apl_errno indicate the error.
     */
    virtual apl_int_t Bind0( acl::CSockAddr const& aoAddress );

    /**
     * Unbinds from the specified local address and stop to accept incoming
     * connections.
     *
     * @return If successful, shall return zero; otherwise -1 shall be returned, and apl_errno indicate the error.
     */
    virtual apl_int_t Unbind0( acl::CSockAddr const& aoAddress );

protected:
    /**
     * Startup acceptor service
     */
    apl_int_t StartupAcceptor(void);
    
    /**
     * Wakeup thread-scheduler
     */
    void Wakeup(void);
    
    CIoSession* NewSession(void);
    
//thread-scheduler
protected:
    apl_ssize_t RegisterHandles(void);
    
    apl_ssize_t ProcessHandles(void);
    
    apl_ssize_t UnregisterHandles(void);
    
    static void* Run(void* apvParam );

private:
    QueueType moRegisterQueue;
    
    QueueType moCancelQueue;
    
    BindingAddressMapType moBindingAddresses;
    
    CWakeablePoll moPoll;

    acl::CLock moLock;
        
    acl::CThreadManager moThreadManager;
};

ANF_NAMESPACE_END

#endif//ANF_IOACCEPTOR_H
