
#include "acl/TimeValue.h"
#include "acl/Synch.h"
#include "anf/Utility.h"
#include "anf/IoFuture.h"

ANF_NAMESPACE_START

CIoFuture::CIoFuture(void)
    : mbIsDone(false)
    , mbIsCancelled(false)
    , miState(0)
{
}

CIoFuture::~CIoFuture(void)
{
}

apl_int_t CIoFuture::Await(acl::CTimeValue const& aoTimeout)
{
    acl::CTimeValue loTimeout(aoTimeout);
    acl::CTimeValue loUnit(10, APL_TIME_SEC);
    
    this->moLock.Lock();
    
    while( !this->mbIsDone 
           && !this->mbIsCancelled 
           && loTimeout.Sec() > 0 )
    {
        this->moCond.Wait(this->moLock, loUnit);
        loTimeout -= loUnit;
    }
    
    this->moLock.Unlock();

    return this->mbIsDone || this->mbIsCancelled ? 0 : -1;
}

bool CIoFuture::IsDone(void)
{
    return this->mbIsDone;
}

bool CIoFuture::IsCancelled(void)
{
    return this->mbIsCancelled;
}

bool CIoFuture::IsException(void)
{
    return this->mbIsDone && this->miState != 0;
}

void CIoFuture::Set( bool abIsDone, bool abIsCancelled, apl_int_t aiState, apl_int_t aiErrno )
{
    this->mbIsCancelled = abIsCancelled;
    this->miState = aiState;
    this->miErrno = aiErrno;
    this->mbIsDone = abIsDone;
}

void CIoFuture::Done(void)
{
    this->moLock.Lock();
    this->Set(true, false, 0, 0);
    this->moCond.Signal();
    this->moLock.Unlock();
}

void CIoFuture::Cancel(void)
{
    this->moLock.Lock();
    this->Set(true, true, 0, 0);
    this->moCond.Signal();
    this->moLock.Unlock();
}

void CIoFuture::Exception( apl_int_t aiState, apl_int_t aiErrno )
{
    this->moLock.Lock();
    this->Set(true, false, aiState, aiErrno);
    this->moCond.Signal();
    this->moLock.Unlock();
}

apl_int_t CIoFuture::GetState(void)
{
    return this->miState;
}

apl_int_t CIoFuture::GetErrno(void)
{
    return this->miErrno;
}

ANF_NAMESPACE_END
