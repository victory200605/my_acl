
#include "acl/TimerQueueAdapter.h"

ACL_NAMESPACE_START

CTimerQueueAdapter::CTimerQueueAdapter(void)
    : mpoTimerQueue(NULL)
    , mbIsDeleteTimerQueue(true)
{
    ACL_NEW_ASSERT(this->mpoTimerQueue, CTimerQueue(1024) );
}

CTimerQueueAdapter::CTimerQueueAdapter( CTimerQueue* apoTimerQueue, bool abIsDoDelete )
    : mpoTimerQueue(apoTimerQueue)
    , mbIsDeleteTimerQueue(abIsDoDelete)
{
}

CTimerQueueAdapter::CTimerQueueAdapter( apl_size_t auSize )
    : mpoTimerQueue(NULL)
    , mbIsDeleteTimerQueue(true)
{
    ACL_NEW_ASSERT(this->mpoTimerQueue, CTimerQueue(auSize) );
}

CTimerQueueAdapter::~CTimerQueueAdapter(void)
{
    this->Deactivate();
    this->moThreadManager.WaitAll();
    
    if (this->mbIsDeleteTimerQueue)
    {
        ACL_DELETE(this->mpoTimerQueue);
    }
}
    
apl_int_t CTimerQueueAdapter::Activate(void)
{
    this->miActive = 1;

    if (this->moThreadManager.Spawn(CTimerQueueAdapter::Svc, this) != 0 )
    {
        this->miActive = 0;
        return -1;
    }
    
    return 0;
}

void CTimerQueueAdapter::Deactivate(void)
{
    this->miActive = 0;
    
    this->moCondition.Signal();
}

apl_int_t CTimerQueueAdapter::Schedule(
    IEventHandler* apoEventHandler,
    void const* apvAct,
    CTimestamp const& aoFutureTime,
    CTimeValue const& aoInterval )
{
    if (this->mpoTimerQueue == NULL)
    {
        return -1;
    }
    
    this->moLock.Lock();

    apl_int_t liResult = this->mpoTimerQueue->Schedule(apoEventHandler, apvAct, aoFutureTime, aoInterval);
    
    this->moCondition.Signal();
    
    this->moLock.Unlock();
    
    return liResult;
}

apl_int_t CTimerQueueAdapter::Schedule(
    IEventHandler* apoEventHandler,
    void const* apvAct,
    CTimeValue const& aoAfter,
    CTimeValue const& aoInterval )
{
    CTimestamp loFutureTime;
    
    loFutureTime.Update(CTimestamp::PRC_USEC);
    loFutureTime += aoAfter;
    
    return this->Schedule( apoEventHandler, apvAct, loFutureTime, aoInterval );
}

apl_int_t CTimerQueueAdapter::Cancel( apl_int_t aiTimerID, void const** appvAct )
{
    if (this->mpoTimerQueue == NULL)
    {
        return -1;
    }
    
    this->moLock.Lock();
    
    apl_int_t liResult = this->mpoTimerQueue->Cancel(aiTimerID, appvAct, false);
    
    this->moCondition.Signal();
    
    this->moLock.Unlock();
    
    return liResult;
}

apl_int_t CTimerQueueAdapter::Wait( CTimeValue const& aoTimeout )
{
    return this->moThreadManager.WaitAll(aoTimeout);
}

void* CTimerQueueAdapter::Svc( void* apvParam )
{
    CTimerQueueAdapter* lpoAdapter = reinterpret_cast<CTimerQueueAdapter*>(apvParam);
    CTimeValue loTimeout(1);
    
    while ( lpoAdapter->miActive )
    {
        lpoAdapter->moLock.Lock();
        
        // If the queue is empty, sleep until there is a change on it.
        if (lpoAdapter->mpoTimerQueue->IsEmpty() )
        {
            if ( lpoAdapter->moCondition.Wait(lpoAdapter->moLock, loTimeout) != 0 )
            {
                lpoAdapter->moLock.Unlock();
                continue;
            }
        }
        else
        {
            // Compute the remaining time, being careful not to sleep
            // for "negative" amounts of time.
            CTimestamp loCurTime;
            CTimestamp loEarlTime;
            loCurTime.Update(CTimestamp::PRC_USEC);
            
            loEarlTime = lpoAdapter->mpoTimerQueue->EarliestTime();
 
            if ( loEarlTime > loCurTime )
            {
              // The earliest time on the Timer_Queue is in future, so
              // use ACE_OS::gettimeofday() to convert the tv to the
              // absolute time.
              CTimeValue loTimeout = loEarlTime - loCurTime;  
              lpoAdapter->moCondition.Wait(lpoAdapter->moLock, loTimeout);
            }
        }
        
        lpoAdapter->moLock.Unlock();

        // Expire timers anyway, at worst this is a no-op.
        lpoAdapter->mpoTimerQueue->Expire();
    }
    
    return NULL;
}

ACL_NAMESPACE_END
