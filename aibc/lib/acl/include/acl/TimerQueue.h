
#ifndef ACL_TIMER_QUEUE_H
#define ACL_TIMER_QUEUE_H

#include "acl/Utility.h"
#include "acl/EventHandler.h"
#include "acl/Synch.h"
#include "acl/stl/list.h"

ACL_NAMESPACE_START

/**
 * @brief Maintains generated dispatch information for Timer nodes.
 */
class CTimerNodeDispatchInfo
{
public:
    /// The type of object held in the queue
    IEventHandler* mpoEventHandler;

    /// Asynchronous completion token associated with the timer.
    void const* mpvAct;

    /// Flag to check if the timer is recurring.
    apl_int_t miRecurringTimer;
};

/**
 * @brief Maintains the state associated with a Timer entry.
 */
class CTimerNode
{
public:
    /// Useful typedef ..
    typedef CTimerNodeDispatchInfo DispatchInfoType;
    
public:
    /// Default constructor
    CTimerNode (void);

    /// Destructor
    ~CTimerNode (void);

    void Set( 
        IEventHandler* apoEventHandler,
        void const* apvAct,
        CTimestamp const& aoTimerValue,
        CTimeValue const& aoInterval );

    /// Get the type.
    IEventHandler* GetEventHandler(void);

    /// Set the type.
    void SetEventHandler( IEventHandler* apoEventHandler );

    /// Get the asynchronous completion token.
    void const* GetAct(void);

    /// Set the asynchronous completion token.
    void SetAct( void* apvAct );

    /// Get the timer value.
    CTimestamp const& GetTimerValue(void) const;

    /// Set the timer value.
    void SetTimerValue( CTimestamp const& aoTimerValue );

    /// Get the timer interval.
    CTimeValue const& GetInterval(void) const;

    /// Set the timer interval.
    void SetInterval( CTimeValue const& aoInterval );

    /// Get the timer_id.
    apl_int_t GetTimerID(void) const;

    /// Set the timer_id.
    void SetTimerID( apl_int_t aiTimerID );

    /// Get the dispatch info.
    void GetDispatchInfo( DispatchInfoType& aoInfo);
    
    apl_int_t GetHeapSlot(void);
    
    void SetHeapSlot( apl_int_t aiSlot );

private:
    /// Type of object stored in the Queue
    IEventHandler* mpoEventHandler;

    /// Asynchronous completion token associated with the timer.
    void const* mpvAct;

    /// Time until the timer expires.
    CTimestamp moTimerValue;

    /// If this is a periodic timer this holds the time until the next
    /// timeout.
    CTimeValue moInterval;

    /// Id of this timer (used to cancel timers before they expire).
    apl_int_t miTimerID;
    
    apl_int_t miHeapSlot;
};

/**
 * @brief Provides a very fast and predictable timer implementation.
 *
 * This implementation uses a heap-based callout queue of
 * absolute times.  Therefore, in the average and worst case,
 * scheduling, canceling, and expiring timers is O(log N) (where
 * N is the total number of timers).  In addition, we can also
 * preallocate as many @c CTimerNode objects as there are slots
 * in the heap.  This allows us to completely remove the need for
 * dynamic memory allocation, which is important for real-time
 * systems.
 */
class CTimerQueue
{
public:
    typedef CTimerNode NodeType;
    
    //Type of DispatchInfo
    typedef CTimerNodeDispatchInfo DispatchInfoType;

public:
    /**
     * @brief The Constructor creates a heap with specified number of elements.
     *
     * @param [in] auSize The maximum number of timers that can be
     * inserted into the new object.
     */
    CTimerQueue( apl_size_t auSize );

    /**
     * @brief Destructor, make virtual for proper destruction of inherited
     * classes.
     */
    virtual ~CTimerQueue (void);
    
    /**
     * @brief Close timer queue, if no empty will cancel first.
     */
    virtual void Close(void);
    
    /**
     * @brief Get current timer queue size.
     */
    virtual apl_size_t GetSize(void);

    /**
     * @brief True if queue is empty, else false.
     */
    virtual bool IsEmpty (void) const;

    /**
     * @brief Returns the time of the earlier node in the Timer_Queue.    Must
     * be called on a non-empty queue.
     */
    virtual CTimestamp const& EarliestTime(void) const;

    /**
     * Schedule a timer
     * @param [in] apoEventHandler : Schedule event handle
     * @param [in] aoFutureTime    : Will expire at <aoFutureTime>, which is specified in absolute time
     * @param [in] apvAct          : If it expires then <apvAct> is passed in as the value to the <functor>
     * @param [in] aoInterval      : If <aoInterval> is != to <CTimeValue::zero> then it is used to 
     *                               reschedule the <apoEventHandler> automatically
     * @return     -1 on failure (which is guaranteed never to be a valid <timer_id>).
     */
    virtual apl_int_t Schedule(
        IEventHandler* apoEventHandler,
        void const* apvAct,
        CTimestamp const& aoFutureTime,
        CTimeValue const& aoInterval = CTimeValue::ZERO );

    /**
     * Resets the interval of the timer
     * @param [in] aiTimerID  : timer id
     * @param [in] aoInterval : which is specified in relative time to the current
     *                          If <aoInterval> is equal to <CTimeValue::zero>,
     *                          the timer will become a non-rescheduling timer
     * @return     0 if successful, -1 if not.
     */
    virtual apl_int_t ResetInterval( apl_int_t aiTimerID, CTimeValue const& aoInterval );

    /**
     * Cancel all timer associated with <apoEventHandler>
     * @param [in] apoEventHandler         : associated event handler
     * @param [in] abIsDontCallHandleClose : If is false then typically invokes the <handle_close> hook,
     *                                       the timer will become a non-rescheduling timer
     * @return     number of timers cancelled.
     */
    virtual apl_ssize_t Cancel( IEventHandler* apoEventHandler, bool abIsDontCallHandleClose = false );

    /**
     * Cancel the single timer that matches the <aiTimerID> value (which
     * was returned from the <Schedule> method).
     * @param [in] aiTimerID : timer id
     * @param [out] appvAct  : If act is non-NULL then it will be set to point to the ``magic cookie'' argument
     *                         passed in when the timer was registered. This makes it possible to 
     *                         free up the memory and avoid memory leaks.
     * @param [in] abIsDontCallHandleClose : If is false then typically invokes the <handle_close> hook,
     *                                       the timer will become a non-rescheduling timer
     * @return     1 if successful, -1 if not.
     */
    virtual apl_int_t Cancel( apl_int_t aiTimerID, void const** appvAct = 0, bool abIsDontCallHandleClose = true );
    
    /**
     * Get the dispatch information for a timer whose value is <= <aoCurrentTime>
     * @param [in] aoCurrentTime : Current time, This does not account for <timer_skew>
     * @param [in] aoInfo        : Return DispatchInfo if successful.
     * @return     1 if successful, 0 if not.
     */
    virtual apl_ssize_t DispatchInfo( CTimestamp const& aoCurrentTime, DispatchInfoType& aoInfo );

    /**
     * Run the <HandleTimeout> for all timers whose values are <= <aoCurrentTime>
     * @param [in] aoCurrentTime : Current time, This does not account for <TimerSkew>
     * @return     the number of timers expired.
     */
    virtual apl_ssize_t Expire ( CTimestamp const& aoCurrentTime );

    /**
     * Run the <functor> for all timers whose values are <CurrTime>.
     * Also accounts for <TimerSkew>.
     *
     * Depending on the resolution of the underlying OS the system calls
     * like select()/poll() might return at time different than that is
     * specified in the timeout. Suppose the OS guarantees a resolution of t ms.
     * The timeline will look like
     *
     *
     *             A                   B
     *             |                   |
     *             V                   V
     *  |-------------|-------------|-------------|-------------|
     *  t             t             t             t             t
     *
     *
     * If you specify a timeout value of A, then the timeout will not occur
     * at A but at the next interval of the timer, which is later than
     * that is expected. Similarly, if your timeout value is equal to B,
     * then the timeout will occur at interval after B. Now depending upon the
     * resolution of your timeouts and the accuracy of the timeouts
     * needed for your application, you should set the value of
     * <timer_skew>. In the above case, if you want the timeout A to fire
     * no later than A, then you should specify your <TimerSkew> to be
     * A % t.
     *
     * The default value is zero.
     *
     * Things get interesting if the t before the timeout value B is zero
     * i.e your timeout is less than the interval. In that case, you are
     * almost sure of not getting the desired timeout behaviour. Maybe you
     * should look for a better OS :-)
     *
     * @return     the number of timers expired.
     */

    apl_ssize_t Expire(void);

    /**
     * Determine the next event to timeout.
     * @param [in]  aoMaxWaitTime : Max wait time
     * @param [out] apoTheTimeout : should be a pointer to storage for the timeout value, 
     *                              and this value is also returned
     * @return     return <aoMaxWaitTime> if there are no pending timers or if all pending timers are longer than max
     *             else return the next event to timeout
     */
    virtual CTimeValue* CalculateTimeout( CTimeValue const& aoMaxWaitTime, CTimeValue* apoTheTimeout );
    
    /**
     * Set the timer skew for the Timer_Queue.
     * @param [in]  aoSkew : timer skew
     */
    void TimerSkew ( CTimeValue const& aoSkew );
    
    /**
     * Get the timer skew for the Timer_Queue.
     * @return     timer skew
     */
    CTimeValue const& TimerSkew(void) const;
    
    /// This method will call the timeout()
    void Upcall( DispatchInfoType& aoInfo, CTimestamp const& aoCurTime );
    
    void CancelEvent( IEventHandler* apoEventHandler );
    
protected:
    /// Removes the earliest node from the queue and returns it
    virtual CTimerNode* RemoveFirst(void);

    /// Reads the earliest node from the queue and returns it.
    virtual CTimerNode* GetFirst(void);
    
    /// Reschedule an "interval" <CTimerNode>.
    virtual void Reschedule( CTimerNode* );

    /// Factory method that allocates a new node.
    virtual CTimerNode* AllocNode(void);

    /// Factory method that frees a previously allocated node.
    virtual void FreeNode( CTimerNode* );
    
    void Insert( NodeType* apoNewNode );
        
    NodeType* Remove( apl_size_t auSlot );

private:
    CTimerQueue( CTimerQueue const& );
    CTimerQueue& operator = ( CTimerQueue const& );

private:
    NodeType** mppoNodeHeap;
    
    NodeType*  mpoPreallocatedNode;
    
    apl_size_t muCurSize;
    
    apl_size_t muFreeSize;
    
    apl_size_t muMaxSize;
        
    /// Adjusts for timer skew in various clocks.
    CTimeValue moTimerSkew;
    
    std::list<NodeType*> moFrees;
        
    CLock moLock;
};

ACL_NAMESPACE_END

#endif //ACL_TIMERQUEUE_H
