
#ifndef ACL_REACTOR_H
#define ACL_REACTOR_H

#include "acl/Utility.h"
#include "acl/Poll.h"
#include "acl/Synch.h"
#include "acl/EventHandler.h"
#include "acl/TimerQueue.h"
#include "acl/stl/map.h"

ACL_NAMESPACE_START

/////////////////////////////////////// handle repository ///////////////////////////////////////////
class CHandleRepository
{
private:
    struct CEventNode
    {
        CEventNode( IEventHandler* apoEvent = NULL, apl_int_t aiMask = 0 ) 
            : miMask(aiMask)
            , mbIsSuspended(false)
            , mpoEvent(apoEvent)
        {
        }
        
        IEventHandler* GetEventHandler(void)                  { return this->mpoEvent; }
        apl_int_t      GetMask(void)                          { return this->miMask; }
        bool           IsSuspended(void)                      { return this->mbIsSuspended; }
        void           SetMask(apl_int_t aiMask)              { this->miMask = aiMask; }
        void           SetSuspended(bool abIsSuspended)       { this->mbIsSuspended = abIsSuspended; }
        
        apl_int_t miMask;
        bool      mbIsSuspended;
        IEventHandler* mpoEvent;
    };

public:
    typedef CEventNode NodeType;
    
public:
    void Bind( apl_int_t aiHandle, IEventHandler* apoEvent, apl_int_t aiMask );
    
    void Unbind( apl_int_t aiHandle );
    
    NodeType* Find( apl_int_t aiHandle );
    
    apl_int_t Mask( apl_int_t aiHandle );
    
    void Mask( apl_int_t aiHandle, apl_int_t aiMask );
    
    apl_int_t HandleState( apl_int_t aiHandle );
    
    void HandleState( apl_int_t aiHandle, apl_int_t aiState );
    
    apl_size_t Size(void);
    
protected:
    std::map<apl_int_t, CEventNode> moHandlerRep;
};

//////////////////////////////////////////////////////////////////////////////////
class CReactor
{
public:
    static const apl_int_t DEFAULT_TIMER_SIZE = 1024;
    
public:
    CReactor(void);
    
    ~CReactor(void);
    
    apl_int_t Initialize( apl_size_t auSize, CTimerQueue* apoTimerQueue = NULL );
    
    apl_int_t Close(void);
    
    apl_int_t RunEventLoop(void);
    
    apl_ssize_t HandleEvent( CTimeValue const& aoTimeout = CTimeValue::MAXTIME );
    
    apl_ssize_t Dispatch(void);
    
    apl_ssize_t WorkPending( CTimeValue const& aoTimeout );
    
    /**
     * @brief Schedule the timer according to the semantics of the <TimerQueue>; wakes
     * up the dispatching thread.
     */
    apl_int_t ScheduleTimer(
        IEventHandler* apoEventHandler,
        void const* apvAct,
        CTimestamp const& aoFutureTime,
        CTimeValue const& aoInterval = CTimeValue::ZERO );
    
    /**
     * @brief Schedule the timer according to the semantics of the <TimerQueue>; wakes
     * up the dispatching thread.
     */
    apl_int_t ScheduleTimer(
        IEventHandler* apoEventHandler,
        void const* apvAct,
        CTimeValue const& aoAfter,
        CTimeValue const& aoInterval = CTimeValue::ZERO );
    
    /**
     * @brief Cancel the <aiTimerID> and return the <appvAct> parameter if an
     * address is passed in. Also wakes up the dispatching thread.
     */
    apl_int_t CancelTimer( apl_int_t aiTimerID, void const** appvAct = NULL );
    
    apl_int_t RegisterHandler( apl_int_t aiHandle, IEventHandler* apoEvent, apl_int_t aiMask );
    
    apl_int_t RemoveHandler( apl_int_t aiHandle, apl_int_t aiMask );
    
    apl_int_t SuspendHandler( apl_int_t aiHandle );
    
    apl_int_t ResumeHandler( apl_int_t aiHandle );
    
    void Deactivate(void);

protected:
    apl_ssize_t DispatchTimerEvent(void);
    
    apl_ssize_t DispatchIOEvent(void);
    
    apl_int_t UpdateMaskOpts( apl_int_t aiHandle, apl_int_t aiEvent );
    
    apl_int_t AddMaskOpts( apl_int_t aiHandle, apl_int_t aiEvent );
    
    apl_int_t Upcall( 
        IEventHandler* apoEventHandler,
        apl_int_t ( IEventHandler::*callback )( apl_int_t ),
        apl_int_t aiHandle );
                
    apl_int_t ReactorMaskToPollEvent( apl_int_t aiMask );

protected:
    apl_atom_t miDeactivated;
    
    CHandleRepository moHandlerRep;

    CPollee moPollee;
    
    CPollee::IteratorType moFirst;
        
    CPollee::IteratorType moLast;
        
    CTimerQueue* mpoTimerQueue;
};

ACL_NAMESPACE_END

#endif //ACL_REACTOR_H

