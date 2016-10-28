
#include "acl/ThreadPool.h"

ACL_NAMESPACE_START

CThreadPool::CThreadPool(void)
    : moIdle(0)
    , muPoolSize(0)
    , muTopBusyCount(0)
    , miScope(0)
    , miStackSize(0)
{
}

CThreadPool::~CThreadPool(void)
{
    this->Close();
}

apl_int_t CThreadPool::Initialize( apl_size_t aiPoolSize, apl_int_t aiScope, apl_size_t aiStackSize )
{
    if ( this->moThreadManager.SpawnN( aiPoolSize, ThreadPoolEntry, this, aiScope, -1, aiStackSize ) != 0 )
    {
        this->Close();
        return -1;
    }
    
    this->muPoolSize  = aiPoolSize;
    this->miScope  = aiScope;
    this->miStackSize = aiStackSize;
        
    return 0;
}

void CThreadPool::Close(void)
{
    CThreadNode* lpoNode = NULL;
    
    for ( apl_size_t liN = 0; liN < this->muPoolSize; )
    {
        if ( this->moIdle.Pop(lpoNode) != 0 )
        {
            apl_sleep(APL_TIME_SEC);
            continue;
        }
        
        lpoNode->mpoRunnable = NULL;
        lpoNode->moSema.Post();
        liN++;
    }
    
    this->moThreadManager.WaitAll();
    
    this->muPoolSize  = 0;
    this->muTopBusyCount = 0;
    this->miScope  = 0;
    this->miStackSize = 0;
}

apl_int_t CThreadPool::GetScope(void)
{
    return this->miScope;
}

apl_size_t CThreadPool::GetStackSize(void)
{
    return this->miStackSize;
}

apl_size_t CThreadPool::GetPoolSize(void)
{
    return this->muPoolSize;
}

apl_size_t CThreadPool::GetIdleCount(void)
{
    return this->moIdle.GetSize();
}

apl_size_t CThreadPool::GetBusyCount(void)
{
    return this->muPoolSize - this->moIdle.GetSize();
}

apl_size_t CThreadPool::GetTopBusyCount(void)
{
    return this->muTopBusyCount;
}

CTimestamp CThreadPool::GetTopBusyTime(void)
{
    return this->moTopBusyTime;
}

apl_int_t CThreadPool::RunI( IRunnable* apoRunnable, CTimeValue const& aoTimeout )
{
    CThreadNode* lpoNode = NULL;
    apl_size_t   liCurrBusy = 0;
    
    if (this->moIdle.Pop(lpoNode, aoTimeout) != 0)
    {
        return -1;
    }
    
    liCurrBusy = this->GetBusyCount();
    
    if (this->muTopBusyCount < liCurrBusy)
    {
        this->muTopBusyCount = liCurrBusy;
        this->moTopBusyTime.Update();
    }
    
    lpoNode->mpoRunnable = apoRunnable;
    lpoNode->moSema.Post();
    
    return 0;
}

void* CThreadPool::ThreadPoolEntry( void* apvParam )
{
    CThreadPool* lpoPool = reinterpret_cast<CThreadPool*>(apvParam);
    CThreadNode* lpoNode = NULL;
    
    ACL_NEW_ASSERT( lpoNode, CThreadNode );
    
    lpoNode->mpoRunnable = NULL;

    while(true)
    {
        lpoPool->moIdle.Push(lpoNode);
        
        lpoNode->moSema.Wait();
        
        if (lpoNode->mpoRunnable == NULL)
        {
            break;
        }
        
        (*lpoNode->mpoRunnable)();
        
        ACL_DELETE(lpoNode->mpoRunnable);
        
        lpoNode->mpoRunnable = NULL;
    }
    
    ACL_DELETE(lpoNode);
    
    return NULL;
}

ACL_NAMESPACE_END
