/**
 * @file ThreadTask.h
 */
#ifndef ACL_THREADTASK_H
#define ACL_THREADTASK_H

#include "acl/Utility.h"
#include "acl/TimeValue.h"
#include "acl/Timestamp.h"
#include "acl/Synch.h"
#include "acl/ThreadManager.h"
#include "acl/MsgQueue.h"

ACL_NAMESPACE_START

/**
 * Class CThreadTask
 */
class CThreadTask
{
private:
    class IRunnable
    {
    public:
        virtual ~IRunnable(void) {};
        
        virtual void operator () (void) = 0;
    };
        
    template<typename T>
    class TReferenceHolder : public IRunnable
    {
    public:
        TReferenceHolder( T const& aoRunnable )
            : moRunnable(aoRunnable)
        {
        }
        
        virtual void operator () (void)
        {
            (this->moRunnable)();
        }
        
    private:
        T moRunnable;
    };
    
    template<typename T>
    class TPointerHolder : public IRunnable
    {
    public:
        TPointerHolder( T* apoRunnable )
            : mpoRunnable(apoRunnable)
        {
        }
        
        virtual ~TPointerHolder(void)
        {
            ACL_DELETE(this->mpoRunnable);
        }
                       
        virtual void operator () (void)
        {
            (*this->mpoRunnable)();
        }
        
        virtual void Release(void)
        {
            this->mpoRunnable = NULL;
        }
        
    private:
        T* mpoRunnable;
    };

public:
   
    /**
     * Constructor
     */
    CThreadTask(void);
    
    /**
     * destructor 
     */
    ~CThreadTask(void);
    
    /**
     * Initialize the thread task
     *
     * @param [in] aiCapacity the queuesize of threadTask
     * @param [in] aiThrCount the size of threadTask
     * @param [in] aiScope    the scope of thread
     * @param [in] aiStackSize the size of stack
     * @retval 0    If success
     * @retval -1   If fail
     */
    apl_int_t Initialize(
        apl_size_t aiCapacity,
        apl_size_t aiThrCount,
        apl_int_t aiScope = APL_THREAD_USER,
        apl_size_t aiStackSize = 0 );
    
    /**
     * Close the threadTask
     */
    void Close(void);
    
    /**
     * get the scope the thread which belongs threadTask
     *
     * @return   APL_THREAD_KERNEL,APL_THREAD_USER.
     */
    apl_int_t GetScope(void);

    /**
     * get the stacksize of thread which belongs threadTask
     *
     * @return     >=0.
     */    
    apl_size_t GetStackSize(void);
    
    /**
     * get the threadsize of threadTask
     *
     * @return    >=0
     */
    apl_size_t GetThreadCount( void );
    
    /**
     * get the idleCount of thread in the threadTask
     *
     * @return    >=0
     */
    apl_size_t GetIdleCount(void);
    
    /**
     * get the busyCount of thread in the threadTask
     *
     * @return    >=0
     */
    apl_size_t GetBusyCount(void);
    
    /**
     * get the TopBusyCount of thread until now in the threadTask
     *
     * @return    >=0
     */
    apl_size_t GetTopBusyCount(void);

    /**
     * get the TopBusyTime until now in the threadTask
     *
     * @return    CTimestamp type
     */    
    CTimestamp GetTopBusyTime(void);
    
    /**
     * get the queuesize of threadTask
     *
     * @return    >=0
     */
    apl_size_t GetCapacity(void);
    
    /**
     * get the waiting queuesize of threadTask 
     *
     * @return    >=0
     */
    apl_size_t GetWaitCount(void);
    
    /**
     * get the topwait queuesize of threadTask until now
     *
     * @return    >=0
     */
    apl_size_t GetTopWaitCount(void);
 
    /**
     * get the TopWaitTime until now in the threadTask
     *
     * @return    CTimestamp type
     */   
    CTimestamp GetTopWaitTime(void);
    
    /**
     * get a idle thread to run from the threadPool
     *
     * @param [in] aoRunnable     runnable object
     * @param [in] aoTimeout      set the timeout value
     *     -CTimeValue::MAXTIME never timeout
     *     -CTimeValue::ZERO    none-block
     *     ->0                  set the timeout
     * @retval 0    If success
     * @retval -1   If fail or timeout
     */
    template<typename RunnableType>
    apl_int_t Run( RunnableType const& aoRunnable, CTimeValue const& aoTimeout = CTimeValue::ZERO );

    /**
     * get a idle thread to run from the threadPool
     *
     * @param [in] apoRunnable    a pointer points to runnable object
     * @param [in] aoTimeout      set the timeout value
     *     -CTimeValue::MAXTIME never timeout
     *     -CTimeValue::ZERO    none-block
     *     ->0                  set the timeout
     * @retval 0    If success
     * @retval -1   If fail or timeout
     */
    template<typename RunnableType>
    apl_int_t Run( RunnableType* apoRunnable, CTimeValue const& aoTimeout = CTimeValue::ZERO );

protected:
    apl_int_t RunI( IRunnable* apoRunnable, CTimeValue const& aoTimeout );
    
    void UpdateBusyTop(void);
    
    static void* ThreadTaskEntry( void* apvParam );

private:
    CThreadManager moThreadManager;
    
    TMsgQueue<IRunnable*>* mpoTaskList;
    
    apl_size_t muBusyCount;
    
    apl_size_t muTopBusyCount;
    
    CTimestamp moTopBusyTime;
    
    apl_size_t muTopWaitCount;
    
    CTimestamp moTopWaitTime;
    
    apl_int_t miScope;
    
    apl_int_t miStackSize;
};

template<typename RunnableType>
apl_int_t CThreadTask::Run( RunnableType const& aoRunnable, CTimeValue const& aoTimeout )
{
    apl_int_t liRetCode = 0;
    TReferenceHolder<RunnableType>* lpoHolder = NULL;
    
    ACL_NEW_ASSERT(lpoHolder, TReferenceHolder<RunnableType>(aoRunnable) );
    
    liRetCode = this->RunI(lpoHolder, aoTimeout);
    if (liRetCode != 0)
    {
        ACL_DELETE(lpoHolder);
    }
    
    return liRetCode;
}

template<typename RunnableType>
apl_int_t CThreadTask::Run( RunnableType* apoRunnable, CTimeValue const& aoTimeout )
{
    apl_int_t liRetCode = 0;
    TPointerHolder<RunnableType>* lpoHolder = NULL;
    
    if (apoRunnable == NULL)
    {
        apl_set_errno(APL_EINVAL);
        return -1;
    }
    
    ACL_NEW_ASSERT(lpoHolder, TPointerHolder<RunnableType>(apoRunnable) );
    
    liRetCode = this->RunI(lpoHolder, aoTimeout);
    if (liRetCode != 0)
    {
        lpoHolder->Release();
        ACL_DELETE(lpoHolder);
    }
    
    return liRetCode;
}

ACL_NAMESPACE_END

#endif //ACL_THREADTASK_H
