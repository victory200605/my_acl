
#include "acl/ThreadTask.h"

ACL_NAMESPACE_START

CThreadTask::CThreadTask(void)
    : mpoTaskList(NULL)
    , muBusyCount(0)
    , muTopBusyCount(0)
    , muTopWaitCount(0)
    , miScope(0)
    , miStackSize(0)
{
}

CThreadTask::~CThreadTask(void)
{
    this->Close();
}

apl_int_t CThreadTask::Initialize( 
        apl_size_t aiCapacity,
        apl_size_t aiThrCount,
        apl_int_t aiScope,
        apl_size_t aiStackSize )
{
    if (this->mpoTaskList != NULL)
    {
        apl_set_errno(APL_EINVAL);
        return -1;
    }
    
    ACL_NEW_ASSERT(this->mpoTaskList, TMsgQueue<IRunnable*>(aiCapacity) );
    
    if ( this->moThreadManager.SpawnN( aiThrCount, ThreadTaskEntry, this, aiScope, -1, aiStackSize ) != 0 )
    {
        this->Close();
        return -1;
    }
    
    this->miScope  = aiScope;
    this->miStackSize = aiStackSize;
    
    return 0;
}

void CThreadTask::Close(void)
{
    apl_size_t liThreadCount = this->moThreadManager.GetCount();
    
    if (this->mpoTaskList == NULL)
    {
        return;
    }
    
    for ( apl_size_t liN = 0; liN < liThreadCount; liN++ )
    {
        this->RunI( NULL, CTimeValue::MAXTIME );
    }
    
    this->moThreadManager.WaitAll();
    
    this->muBusyCount = 0;
    this->muTopBusyCount = 0;
    this->muTopWaitCount = 0;
    this->miScope  = 0;
    this->miStackSize = 0;
    
    for ( TMsgQueue<IRunnable*>::IteratorType loIter = this->mpoTaskList->Begin();
          loIter != this->mpoTaskList->End(); ++loIter )
    {
        IRunnable* lpoRunable = *loIter;
        ACL_DELETE(lpoRunable);
    }
    
    ACL_DELETE(this->mpoTaskList);
}

apl_int_t CThreadTask::GetScope(void)
{
    return this->miScope;
}

apl_size_t CThreadTask::GetStackSize(void)
{
    return this->miStackSize;
}

apl_size_t CThreadTask::GetThreadCount(void)
{
    return this->moThreadManager.GetCount();
}

apl_size_t CThreadTask::GetIdleCount(void)
{
    return this->GetThreadCount() - this->GetBusyCount();
}

apl_size_t CThreadTask::GetBusyCount(void)
{
    return this->muBusyCount;
}

apl_size_t CThreadTask::GetTopBusyCount(void)
{
    return this->muTopBusyCount;
}

CTimestamp CThreadTask::GetTopBusyTime(void)
{
    return this->moTopBusyTime;
}

apl_size_t CThreadTask::GetCapacity(void)
{
    return this->mpoTaskList->GetCapacity();
}
    
apl_size_t CThreadTask::GetWaitCount(void)
{
    return this->mpoTaskList->GetSize();
}
    
apl_size_t CThreadTask::GetTopWaitCount(void)
{
    return this->muTopWaitCount;
}
    
CTimestamp CThreadTask::GetTopWaitTime(void)
{
    return this->moTopWaitTime;
}

apl_int_t CThreadTask::RunI( IRunnable* apoRunnable, CTimeValue const& aoTimeout )
{
    if (this->mpoTaskList->Push(apoRunnable, aoTimeout) != 0)
    {
        return -1;
    }
    
    if (this->mpoTaskList->GetSize() > this->muTopWaitCount)
    {
        this->muTopWaitCount = this->mpoTaskList->GetSize();
        this->moTopWaitTime.Update();
    }
    
    return 0;
}

void CThreadTask::UpdateBusyTop(void)
{
    if (this->muBusyCount > this->muTopBusyCount)
    {
        this->muTopBusyCount = this->muBusyCount;
        this->moTopBusyTime.Update();
    }
}

void* CThreadTask::ThreadTaskEntry( void* apvParam )
{
    CThreadTask* lpoTask = reinterpret_cast<CThreadTask*>(apvParam);
    IRunnable*   lpoRunnable = NULL;

    while(true)
    {
        if ( lpoTask->mpoTaskList->Pop(lpoRunnable, CTimeValue::MAXTIME) != 0 )
        {
            ACL_ASSERT(false);
        }
        
        if (lpoRunnable == NULL)
        {
            break;
        }
        
        lpoTask->muBusyCount++;
        
        lpoTask->UpdateBusyTop();
        
        (*lpoRunnable)();
        
        lpoTask->muBusyCount--;
        
        ACL_DELETE(lpoRunnable);
    }
    
    return NULL;
}

ACL_NAMESPACE_END
