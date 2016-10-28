
#ifndef ACL_TIMEQUEUEADAPTER_H
#define ACL_TIMEQUEUEADAPTER_H

#include "acl/Utility.h"
#include "acl/TimerQueue.h"
#include "acl/ThreadManager.h"

ACL_NAMESPACE_START

/**
 * @class CTimerQueueAdapter
 *
 * @brief Adapts a CTimerQueue using a separate thread for dispatching.
 *
 * This implementation of a CTimerQueue uses a separate thread to
 * dispatch the timers. The base queue need not be thread safe,
 * this class takes all the necessary locks.
 *
 */
class CTimerQueueAdapter
{
public:
    /**
     * @brief Creates the timer queue. Activation of the task is the user's
     * responsibility. timer queue size default is 1024, a TimerQueue is dynamically created
     */
    CTimerQueueAdapter(void);

    /**
     * @brief Creates the timer queue. Activation of the task is the user's
     * responsibility. a pointer to a timer queue can be passed
     *
     * @param [in] apoTimerQueue : passed timer queue
     * @param [in] abIsDoDelete :  whether we should delete the timer queue
     *
     */
    CTimerQueueAdapter( CTimerQueue* apoTimerQueue, bool abIsDoDelete );
    
    /**
     * @brief Creates the timer queue. Activation of the task is the user's
     * responsibility. timer queue <auSize> can be passed, a TimerQueue is dynamically created
     *
     * @param [in] auSize : passed timer queue capacity
     */
    CTimerQueueAdapter( apl_size_t auSize );
    
    /**
     * Initialize TimerQueue Adapter and only a single thread is ever spawned.
     */
    virtual apl_int_t Activate(void);

    /**
     * @brief Inform the dispatching thread that it should terminate.
     */
    virtual void Deactivate(void);
    
    /**
     * @brief Destructor, make virtual for proper destruction of inherited
     * classes.
     */
    virtual ~CTimerQueueAdapter(void);
    
    /**
     * @brief Schedule the timer according to the semantics of the <TimerQueue>; wakes
     * up the dispatching thread.
     */
    apl_int_t Schedule(
        IEventHandler* apoEventHandler,
        void const* apvAct,
        CTimestamp const& aoFutureTime,
        CTimeValue const& aoInterval = CTimeValue::ZERO );
    
    /**
     * @brief Schedule the timer according to the semantics of the <TimerQueue>; wakes
     * up the dispatching thread.
     */
    apl_int_t Schedule(
        IEventHandler* apoEventHandler,
        void const* apvAct,
        CTimeValue const& aoAfter,
        CTimeValue const& aoInterval = CTimeValue::ZERO );
    
    /**
     * @brief Cancel the <aiTimerID> and return the <appvAct> parameter if an
     * address is passed in. Also wakes up the dispatching thread.
     */
    apl_int_t Cancel( apl_int_t aiTimerID, void const** appvAct = NULL );
    
    /**
     * @brief Wait the dispatching thread.
     */
    apl_int_t Wait( CTimeValue const& aoTimeout = CTimeValue::MAXTIME );

protected:
    static void* Svc( void* apvParam );
    
private:
    /// The underlying Timer_Queue.
    CTimerQueue* mpoTimerQueue;

    /// Keeps track of whether we should delete the timer queue (if we
    /// didn't create it, then we don't delete it).
    bool mbIsDeleteTimerQueue;

    CLock moLock;

    /**
     * The dispatching thread sleeps on this condition while waiting to
     * dispatch the next timer; it is used to wake it up if there is a
     * change on the timer queue.
     */
    CCondition moCondition;

    /// When deactivate is called this variable turns to false and the
    /// dispatching thread is signalled, to terminate its main loop.
    apl_atom_t miActive;
    
    CThreadManager moThreadManager;
};

ACL_NAMESPACE_END

#endif //ACL_TIMEQUEUEADAPTER_H
