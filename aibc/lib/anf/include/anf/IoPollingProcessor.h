
#ifndef ANF_IOPOLLING_PROCESSOR_H
#define ANF_IOPOLLING_PROCESSOR_H

#include "acl/ThreadManager.h"
#include "acl/Synch.h"
#include "acl/SockStream.h"
#include "acl/SockDgram.h"
#include "anf/IoProcessor.h"
#include "anf/ConcurrentQueue.h"
#include "anf/ConcurrentMap.h"
#include "anf/WakeablePoll.h"
#include "anf/IoSessionAdapter.h"
#include "anf/future/WriteFuture.h"

ANF_NAMESPACE_START

class CIoPollingProcessor : public IoProcessor, protected CIoSessionAdapter
{
public:
    typedef TConcurrentQueue<SessionPtrType> QueueType;
    typedef std::map<apl_handle_t, SessionPtrType> SessionMapType;
        
public:
    CIoPollingProcessor(void);
    
    virtual ~CIoPollingProcessor(void);
    
    /**
     * Set the Configuration which provides I/O service to this processor
     */
    virtual void SetConfiguration( CIoConfiguration* apoConfiguration );
    
    /**
     * Set the Handler which provides I/O service to this processor
     */
    virtual void SetHandler( IoHandler* apoHandler );
    
    /** 
     * @brief Dispose the processor and release all related resource 
     * 
     * @returns 0 if successful, otherwise -1 shall be return 
     */
    virtual apl_int_t Dispose( bool abIsWait = true );
    
    /**
     * Adds the specified session to the I/O processor so that
     * the I/O processor starts to perform any I/O operations related
     * with the session.
     */
    virtual apl_int_t Add( SessionPtrType& aoSession );

    /**
     * Flushes the internal write request queue of the specified session.
     */
    virtual apl_int_t Flush( SessionPtrType& aoSession );

    /**
     * Controls the traffic of the specified session 
     */
    virtual apl_int_t UpdateTrafficControl( SessionPtrType& aoSession );

    /**
     * Removes and closes the specified session from the I/O
     * processor so that the I/O processor closes the connection
     * associated with the session and releases any other related
     * resources.
     */
    virtual apl_int_t Remove( SessionPtrType& aoSession );

    /** 
     * Return session size of processor
     */
    virtual apl_size_t GetSize(void);

//processor service
protected:
    apl_int_t StartupProcessor(void);
    
    void Wakeup(void);

//schedule-thread 
protected:
    struct CIdleNotifier
    {
        CIdleNotifier( acl::CTimestamp const& aoCurrTime );
            
        void operator () ( SessionMapType::iterator& aoIter );
        
        acl::CTimestamp const& moCurrTime;
    };

    struct CSuspendSessionWrite : public CIoSessionAdapter
    {
        CSuspendSessionWrite( SessionPtrType& aoSession, CIoPollingProcessor* apoProcessor );

        void operator () (void);

        SessionPtrType& moSession;
        CIoPollingProcessor* mpoProcessor;
    };
    
    apl_ssize_t HandleNewSessions(void);
        
    void HandleUpdateTrafficMask(void);
    
    void HandleFlush( acl::CTimestamp const& aoCurrTime );

    void HandleRemove(void);
    
    void Process( acl::CTimestamp const& aoCurrTime );
    
    void NotifyIdleSessions( acl::CTimestamp const& aoCurrTime );
    
    void ReadNow( SessionPtrType& aoSession, acl::CTimestamp const& aoCurrTime );
        
    void FlushNow( SessionPtrType& aoSession, acl::CTimestamp const& aoCurrTime );
    
    apl_int_t FlushRequestNow(
        SessionPtrType& aoSession,
        CWriteFuture::PointerType& aoFuture,
        acl::CTimestamp const& aoCurrTime );
            
    void UpdateTrafficMaskNow( SessionPtrType& aoSession, apl_int_t aiOld, apl_int_t aiNew );
    
    void RemoveNow( SessionPtrType& aoSession );

    void ClearWriteRequestQueue( SessionPtrType& aoSession );
    
    void ScheduleRemove( SessionPtrType& aoSession );
        
    void ScheduleFlush( SessionPtrType& aoSession );
        
    void ScheduleTrafficControl( SessionPtrType& aoSession );
    
    static void* Run( void* apvParam );

private:
    IoHandler* mpoHandler;
    
    CIoConfiguration* mpoConfiguration;

    QueueType moNewSessions;
    QueueType moRemovingSessions;
    QueueType moFlushingSessions;
    QueueType moTrafficControllingSessions;
    
    SessionMapType moSessions;
    
    CWakeablePoll moPoll;
    
    acl::CSockHandlePair moSockPair;
    
    bool mbIsDisposed;

    bool mbIsActivated;
    
    acl::CLock moLock;
    
    acl::CThreadManager moThreadManager;
    
    acl::CTimeValue moPollTimeout;
        
    acl::CTimestamp moLastIdleCheckTime;

    acl::CAny moRecvMessage;
};

ANF_NAMESPACE_END

#endif//ANF_IOPOLLING_PROCESSOR_H
