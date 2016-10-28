
#include "acl/Thread.h"
#include "acl/ThreadManager.h"

ACL_NAMESPACE_START

//////////////////////////////////////////////////////////////////////////////////////
CThreadManager::CThreadAdapter::CThreadAdapter(
    apl_thr_func_ptr afFunc,
    void* apvArg,
    CThreadDescriptor* apoThreadDesc )
    : mfFunc(afFunc)
    , mpvArg(apvArg)
    , mpoThreadDesc(apoThreadDesc)
{
    this->mpoThreadDesc->Reference();
}

CThreadManager::CThreadAdapter::~CThreadAdapter(void)
{
    this->mpoThreadDesc->Release();
}
        
void CThreadManager::CThreadAdapter::Invoke(void)
{
    this->mpoThreadDesc->Lock();
    this->mpoThreadDesc->SetRunState(THR_RUNNING);
    this->mpoThreadDesc->Unlock();
    
    void* lpvReturn = (*this->mfFunc)(this->mpvArg);
    
    this->mpoThreadDesc->SetReturn(lpvReturn);
    
    this->mpoThreadDesc->SetRunState(THR_TERMINATED);
}

/////////////////////////////////////////////////////////////////////////////////////////////
CThreadManager::CThreadDescriptor::CThreadDescriptor( apl_int_t aiGrpID, apl_int_t aiScope )
    : miThreadID(0)
    , miScope(aiScope)
    , miGrpID(aiGrpID)
    , miRunState(THR_IDLE)
    , mpvReturn(NULL)
    , miReference(0)
{
    //printf("CThreadDescriptor constructor\n");
}

CThreadManager::CThreadDescriptor::~CThreadDescriptor(void)
{
    //printf("CThreadDescriptor disconstructor\n");
}

apl_thread_t CThreadManager::CThreadDescriptor::GetThreadID(void)
{
    return this->miThreadID;
}

apl_int_t CThreadManager::CThreadDescriptor::GetScope(void)
{
    return this->miScope;
}

apl_int_t CThreadManager::CThreadDescriptor::GetGrpID(void)
{
    return this->miGrpID;
}

apl_int_t CThreadManager::CThreadDescriptor::GetRunState(void)
{
    return this->miRunState;
}

void* CThreadManager::CThreadDescriptor::GetReturn(void)
{
    return this->mpvReturn;
}

void CThreadManager::CThreadDescriptor::SetThreadID( apl_thread_t aiThreadID )
{
    this->miThreadID = aiThreadID;
}

void CThreadManager::CThreadDescriptor::SetGrpID( apl_int_t aiGrpID )
{
    this->miGrpID = aiGrpID;
}

void CThreadManager::CThreadDescriptor::SetRunState( apl_int_t aiState )
{
    this->miRunState = aiState;
}

void CThreadManager::CThreadDescriptor::SetReturn( void* apvReturn )
{
    this->mpvReturn = apvReturn;
}

void CThreadManager::CThreadDescriptor::Reference(void)
{
    this->miReference++;
}
        
void CThreadManager::CThreadDescriptor::Release(void)
{
    this->moLock.Lock();
    
    apl_int_t liReference = --this->miReference;
    
    this->moLock.Unlock();
    
    if (liReference == 0)
    {
        CThreadDescriptor* lpoThis = this;
        ACL_DELETE(lpoThis);
    }
}

void CThreadManager::CThreadDescriptor::Lock(void)
{
    this->moLock.Lock();
}
        
void CThreadManager::CThreadDescriptor::Unlock(void)
{
    this->moLock.Unlock();
}

////////////////////////////////////////////////////////////////////////////////////////
CThreadManager::CThreadManager(void)
    : miGrpID(0)
    , muListSize(0)
{
}

CThreadManager::~CThreadManager(void)
{
    this->Clear();
}

apl_int_t CThreadManager::Spawn(
    apl_thread_t* aptThreadID,
    apl_thr_func_ptr afFunc,
    void* apvArg,
    apl_int_t aiScope,
    apl_int_t aiGrpID,
    apl_size_t aiStackSize )
{
    apl_thread_t       ltThrd;
    CThreadAdapter*    lpoAdapter = NULL;
    CThreadDescriptor* lpoDesc = NULL;
    
    if (aiGrpID <= 0)
    {
        //Default group id
        aiGrpID = this->miGrpID++;
    }
    
    ACL_NEW_ASSERT( lpoDesc, CThreadDescriptor(aiGrpID, aiScope) );
    ACL_NEW_ASSERT( lpoAdapter, CThreadAdapter(afFunc, apvArg, lpoDesc) );
    
    lpoDesc->Lock();
    
    if ( CThread::Spawn(&ltThrd, ThreadEntry, lpoAdapter, aiScope, aiStackSize) != 0 )
    {
        lpoDesc->Unlock();
        
        ACL_DELETE(lpoAdapter);
        //ACL_DELETE(lpoDesc); will be release in CThreadAdapter
        
        return -1;
    }
    
    lpoDesc->SetThreadID(ltThrd);
    lpoDesc->SetRunState(THR_SPAWNED);
    
    this->Append(lpoDesc);

    if (aptThreadID != NULL)
    {
        *aptThreadID = ltThrd;
    }
    
    lpoDesc->Unlock();
    
    return 0;
}

apl_int_t CThreadManager::Spawn(
    apl_thr_func_ptr afFunc,
    void* apvArg,
    apl_int_t aiScope,
    apl_int_t aiGrpID,
    apl_size_t aiStackSize )
{
    apl_thread_t ltThrd;
    
    return this->Spawn(&ltThrd, afFunc, apvArg, aiScope, aiGrpID, aiStackSize);
}
    
apl_int_t CThreadManager::SpawnN(
    apl_thread_t aptThreadID[],
    apl_size_t aiN,
    apl_thr_func_ptr afFunc,
    void* apvArg,
    apl_int_t aiScope,
    apl_int_t aiGrpID,
    apl_size_t aiStackSize )
{
    for ( apl_size_t liN = 0; liN < aiN; liN++ )
    {
        if ( this->Spawn(&aptThreadID[liN], afFunc, apvArg, aiScope, aiGrpID, aiStackSize) != 0 )
        {
            return -1;
        }
    }
    
    return 0;
}

apl_int_t CThreadManager::SpawnN(
    apl_size_t aiN,
    apl_thr_func_ptr afFunc,
    void* apvArg,
    apl_int_t aiScope,
    apl_int_t aiGrpID,
    apl_size_t aiStackSize )
{
    for ( apl_size_t liN = 0; liN < aiN; liN++ )
    {
        if ( this->Spawn(afFunc, apvArg, aiScope, aiGrpID, aiStackSize) != 0 )
        {
            return -1;
        }
    }
    
    return 0;
}

apl_int_t CThreadManager::Wait( apl_thread_t atThreadID, void** appvStatus, CTimeValue const& aoTimeout )
{
    apl_int_t  liRetCode = 0;
    CTimeValue loTimeout(aoTimeout);
    DescIterType  loIter = this->Find(atThreadID);
    
    if ( loIter == this->End() )
    {
        return 0;
    }
    
    liRetCode = this->Wait(loIter, appvStatus, aoTimeout);
    if (liRetCode == 0)
    {
        this->Erase(loIter);
    }
    
    return liRetCode;
}
    
apl_int_t CThreadManager::WaitGrp( apl_int_t aiGrpID, CTimeValue const& aoTimeout )
{
    CTimestamp loStartTime;
    CTimestamp loEndTime;
    CTimeValue loTimeout(aoTimeout);
    DescIterType  loNext = this->Begin();
    DescIterType  loCurr;

    while ( loNext != this->End() )
    {
        loCurr = loNext;
        loNext++;
        
        if ( (*loCurr)->GetGrpID() != aiGrpID )
        {
            continue;
        }
        
        loStartTime.Update();
        
        if ( this->Wait(loCurr, NULL, loTimeout) != 0 )
        {
            return -1;
        }
        
        this->Erase(loCurr);
        
        loEndTime.Update();
        
        loTimeout -= (loEndTime - loStartTime);
    }
    
    return 0;
}
    
apl_int_t CThreadManager::WaitAll( CTimeValue const& aoTimeout )
{
    CTimestamp loStartTime;
    CTimestamp loEndTime;
    CTimeValue loTimeout(aoTimeout);
    DescIterType  loNext = this->Begin();

    while ( loNext != this->End() )
    {        
        loStartTime.Update();
        
        if ( this->Wait(loNext, NULL, loTimeout) != 0 )
        {
            return -1;
        }
        
        this->Erase(loNext++);
        
        loEndTime.Update();
        
        loTimeout -= (loEndTime - loStartTime);
    }
    
    return 0;
}

apl_int_t CThreadManager::Wait( DescIterType aoIter, void** appvStatus, CTimeValue const& aoTimeout )
{
    CTimeValue loTimeout(aoTimeout);
    
    while(loTimeout.Msec() >= 0)
    {
        if ( (*aoIter)->GetRunState() != THR_TERMINATED )
        {
            apl_sleep(APL_TIME_MSEC * 10);
            loTimeout.Msec(loTimeout.Msec() - 10);
        }
        else
        {
            if (appvStatus != NULL)
            {
                *appvStatus = (*aoIter)->GetReturn();
            }
            
            break;
        }
    }
    
    if (loTimeout.Sec() < 0)
    {
        apl_set_errno(APL_ETIMEDOUT);
        return -1;
    }

    return 0;
}
/*
apl_int_t CThreadManager::Cancel(apl_thread_t atThreadID)
{
    DescIterType loIter = this->Find(atThreadID);
    if ( loIter == this->End() )
    {
        return 0;
    }
    
    (*loIter)->SetRunState(THR_CANCELED);
    
    return 0;
}

apl_int_t CThreadManager::CancelGrp(apl_int_t aiGrpID)
{
    for ( DescIterType loFirst = this->Begin(); loFirst != this->End(); loFirst++ )
    {
        if ( (*loFirst)->GetGrpID() != aiGrpID )
        {
            continue;
        }
    
        if ( this->Cancel( (*loFirst)->GetThreadID() ) != 0 )
        {
            return -1;
        }
    }
    
    return 0;
}

apl_int_t CThreadManager::CancelAll(void)
{
    for ( DescIterType loFirst = this->Begin(); loFirst != this->End(); loFirst++ )
    {
        if ( this->Cancel( (*loFirst)->GetThreadID() ) != 0 )
        {
            return -1;
        }
    }
    
    return 0;
}
*/

apl_int_t CThreadManager::GetRunState( apl_thread_t atThreadID )
{
    DescIterType loIter = this->Find(atThreadID);
    if ( loIter == this->End() )
    {
        return -1;
    }
    
    return (*loIter)->GetRunState();
}

apl_int_t CThreadManager::GetScope( apl_thread_t atThreadID )
{
    DescIterType loIter = this->Find(atThreadID);
    if ( loIter == this->End() )
    {
        return -1;
    }
    
    return (*loIter)->GetScope();
}

apl_size_t CThreadManager::GetCount()
{
    return this->muListSize;
}

CThreadManager::DescIterType CThreadManager::Begin(void)
{
    return this->moDescList.begin();
}
    
CThreadManager::DescIterType CThreadManager::End(void)
{
    return this->moDescList.end();
}
    
CThreadManager::DescIterType CThreadManager::Find( apl_thread_t atThreadID )
{
    for ( DescIterType loIter = this->moDescList.begin(); loIter != this->moDescList.end(); loIter++ )
    {
        if ( (*loIter)->GetThreadID() == atThreadID )
        {
            return loIter;
        }
    }
    
    return this->moDescList.end();
}

void CThreadManager::Clear(void)
{
    for ( DescIterType loIter = this->moDescList.begin(); loIter != this->moDescList.end(); )
    {
        this->Erase(loIter++);
    }
}
    
void CThreadManager::Erase( DescIterType aoIter )
{
    CThreadDescriptor* lpoDesc = (*aoIter);
    
    this->moDescList.erase(aoIter);
    
    this->muListSize--;
    
    lpoDesc->Release();
}
    
void CThreadManager::Append( CThreadDescriptor* apoDesc )
{
    this->moDescList.push_back(apoDesc);
    
    this->muListSize++;
    
    apoDesc->Reference();
}
    
void* CThreadManager::ThreadEntry( void* apvParam )
{
    CThreadAdapter* lpoAdapter = reinterpret_cast<CThreadAdapter*>(apvParam);
    
    lpoAdapter->Invoke();
    
    ACL_DELETE(lpoAdapter);
    
    CThread::Exit(NULL);
    
    return NULL;
}

ACL_NAMESPACE_END
