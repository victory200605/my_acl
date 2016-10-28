
#include "acl/Synch.h"

ACL_NAMESPACE_START

///////////////////////////////////////////////// CNullLock ///////////////////////////////////////////
apl_int_t CNullLock::Lock()
{
    return 0;
}
    
apl_int_t CNullLock::Unlock(void)
{
    return 0;
}

///////////////////////////////////////////////// CLock ///////////////////////////////////////////
CLock::CLock(void)
{
    apl_int_t liRetCode = apl_mutex_init(&this->moMutex);
    
    ACL_ASSERT(liRetCode == 0);
}

CLock::~CLock(void)
{
    apl_int_t liRetCode = apl_mutex_destroy(&this->moMutex);
    
    ACL_ASSERT(liRetCode == 0);
}

apl_int_t CLock::Lock()
{
    apl_int_t liRetCode = apl_mutex_lock(&this->moMutex);
    
    ACL_ASSERT(liRetCode == 0);

    return liRetCode;
}
    
apl_int_t CLock::Unlock(void)
{
    apl_int_t liRetCode = apl_mutex_unlock(&this->moMutex);
    
    ACL_ASSERT(liRetCode == 0);
    
    return liRetCode;
}

CLock::operator apl_mutex_t* (void)
{
    return &this->moMutex;
}

///////////////////////////////////////////////// CSpinLock ///////////////////////////////////////////
CSpinLock::CSpinLock(void)
{
    apl_int_t liRetCode = apl_spin_init(&this->moSpin);
    
    ACL_ASSERT(liRetCode == 0);
}

CSpinLock::~CSpinLock(void)
{
    apl_int_t liRetCode = apl_spin_destroy(&this->moSpin);
    
    ACL_ASSERT(liRetCode == 0);
}

apl_int_t CSpinLock::Lock(void)
{
    apl_int_t liRetCode = apl_spin_lock(&this->moSpin);
    
    ACL_ASSERT(liRetCode == 0);
    
    return liRetCode;
}
    
apl_int_t CSpinLock::Unlock(void)
{
    apl_int_t liRetCode = apl_spin_unlock(&this->moSpin);
    
    ACL_ASSERT(liRetCode == 0);
    
    return liRetCode;
}

///////////////////////////////////////////////// CFileLock ///////////////////////////////////////////
CFileLock::CFileLock( const char* apcName )
{
    this->miHandle = apl_open( apcName, APL_O_RDWR | APL_O_CREAT, 0666 );
    
    ACL_ASSERT( this->miHandle >= 0 );
}

CFileLock::~CFileLock(void)
{
    apl_close(this->miHandle);
}
    
apl_int_t CFileLock::Lock(void)
{
    apl_int_t liRetCode = 0;
    
    ACL_RETRY_CHK( 
        (liRetCode = apl_fcntl_wrlock(this->miHandle, APL_SEEK_SET, 0, 0, 1/*block*/) ) < 0, -1 );

    ACL_ASSERT(liRetCode == 0);
 
    return liRetCode;
}

apl_int_t CFileLock::TryLock(void)
{
    apl_int_t liRetCode = 0;
    
    liRetCode = apl_fcntl_wrlock(this->miHandle, APL_SEEK_SET, 0, 0, 0/*block*/);
    
    ACL_ASSERT(liRetCode == 0 || apl_get_errno() == APL_EACCES || apl_get_errno() == APL_EAGAIN );
    
    return liRetCode;
}
    
apl_int_t CFileLock::Unlock(void)
{
    apl_int_t liRetCode = 0;
    
    liRetCode = apl_fcntl_unlock(this->miHandle, APL_SEEK_SET, 0, 0);
    
    ACL_ASSERT(liRetCode == 0);
    
    return liRetCode;
}

///////////////////////////////////////////////// CNullRWLock ///////////////////////////////////////////
apl_int_t CNullRWLock::RLock( CTimeValue const& aoTimeout )
{
    return 0;
}
    
apl_int_t CNullRWLock::WLock( CTimeValue const& aoTimeout )
{
    return 0;
}

apl_int_t CNullRWLock::Unlock(void)
{
    return 0;
}

///////////////////////////////////////////////// CRWLock ///////////////////////////////////////////
CRWLock::CRWLock(void)
{
    apl_int_t liRetCode = apl_rwlock_init(&this->moRWLock);
    
    ACL_ASSERT(liRetCode == 0);
}

CRWLock::~CRWLock(void)
{
    apl_int_t liRetCode = apl_rwlock_destroy(&this->moRWLock);
    
    ACL_ASSERT(liRetCode == 0);
}

apl_int_t CRWLock::RLock( CTimeValue const& aoTimeout )
{
    apl_time_t liTimeout = (aoTimeout == CTimeValue::MAXTIME ? -1 : aoTimeout.Nsec() );
    
    apl_int_t liRetCode = apl_rwlock_rdlock(&this->moRWLock, liTimeout);
    
    if (liTimeout == 0)
    {
        return liRetCode == 0 ? 0 : -1;
    }
    else
    {
        ACL_ASSERT(liRetCode == 0 || apl_get_errno() == APL_ETIMEDOUT);

        return liRetCode;
    }
}
    
apl_int_t CRWLock::WLock( CTimeValue const& aoTimeout )
{
    apl_time_t liTimeout = (aoTimeout == CTimeValue::MAXTIME ? -1 : aoTimeout.Nsec() );
    
    apl_int_t liRetCode = apl_rwlock_wrlock(&this->moRWLock, liTimeout);
    
    if (liTimeout == 0)
    {
        return liRetCode == 0 ? 0 : -1;
    }
    else
    {
        ACL_ASSERT(liRetCode == 0 || apl_get_errno() == APL_ETIMEDOUT);

        return liRetCode;
    }
}
    
apl_int_t CRWLock::Unlock(void)
{
    apl_int_t liRetCode = apl_rwlock_unlock(&this->moRWLock);
    
    ACL_ASSERT(liRetCode == 0);
    
    return liRetCode;
}

///////////////////////////////////////////////// CFileRWLock ///////////////////////////////////////////
CFileRWLock::CFileRWLock( const char* apcName )
{
    this->miHandle = apl_open( apcName, APL_O_RDWR | APL_O_CREAT, 0666 );
    
    ACL_ASSERT( this->miHandle >= 0 );
}
    
CFileRWLock::~CFileRWLock(void)
{
    apl_close(this->miHandle);
}

apl_int_t CFileRWLock::RLock(void)
{
    apl_int_t liRetCode = 0;
    
    ACL_RETRY_CHK( 
        (liRetCode = apl_fcntl_rdlock(this->miHandle, APL_SEEK_SET, 0, 0, 1/*block*/) ) < 0, -1 );

    ACL_ASSERT(liRetCode == 0);
 
    return liRetCode;
}
    
apl_int_t CFileRWLock::RTryLock(void)
{
    apl_int_t liRetCode = 0;
    
    liRetCode = apl_fcntl_rdlock(this->miHandle, APL_SEEK_SET, 0, 0, 0/*block*/);
    
    ACL_ASSERT(liRetCode == 0 || apl_get_errno() == APL_EACCES || apl_get_errno() == APL_EAGAIN );
    
    return liRetCode;
}
    
apl_int_t CFileRWLock::WLock(void)
{
    apl_int_t liRetCode = 0;
    
    ACL_RETRY_CHK( 
        (liRetCode = apl_fcntl_wrlock(this->miHandle, APL_SEEK_SET, 0, 0, 1/*block*/) ) < 0, -1 );

    ACL_ASSERT(liRetCode == 0);
 
    return liRetCode;
}
    
apl_int_t CFileRWLock::WTryLock(void)
{
    apl_int_t liRetCode = 0;
    
    liRetCode = apl_fcntl_wrlock(this->miHandle, APL_SEEK_SET, 0, 0, 0/*block*/);
    
    ACL_ASSERT(liRetCode == 0 || apl_get_errno() == APL_EACCES || apl_get_errno() == APL_EAGAIN );
 
    return liRetCode;
}
    
apl_int_t CFileRWLock::Unlock(void)
{
    apl_int_t liRetCode = 0;
    
    liRetCode = apl_fcntl_unlock(this->miHandle, APL_SEEK_SET, 0, 0);
    
    ACL_ASSERT(liRetCode == 0);
    
    return liRetCode;
}

///////////////////////////////////////////////// CCondition ///////////////////////////////////////////
CCondition::CCondition(void)
{
    apl_int_t liRetCode = apl_cond_init(&this->moCond);
    
    ACL_ASSERT(liRetCode == 0);
}

CCondition::~CCondition(void)
{
    apl_int_t liRetCode = apl_cond_destroy(&this->moCond);
    
    ACL_ASSERT(liRetCode == 0);
}

apl_int_t CCondition::Wait( CLock& aoLock, CTimeValue const& aoTimeout )
{
    apl_time_t liTimeout = (aoTimeout == CTimeValue::MAXTIME ? -1 : aoTimeout.Nsec() );
        
    apl_int_t liRetCode = apl_cond_wait(&this->moCond, (apl_mutex_t*)aoLock, liTimeout);
    
    if (liTimeout == 0)
    {
        return liRetCode == 0 ? 0 : -1;
    }
    else
    {
        ACL_ASSERT(liRetCode == 0 || apl_get_errno() == APL_ETIMEDOUT);

        return liRetCode;
    }
}

apl_int_t CCondition::Wait( CLock& aoLock, CTimestamp const& aoExpired )
{
    apl_int_t liRetCode = apl_cond_wait_abs(&this->moCond, (apl_mutex_t*)aoLock, aoExpired.Nsec() );
    
    ACL_ASSERT(liRetCode == 0 || apl_get_errno() == APL_ETIMEDOUT);

    return liRetCode;
}

apl_int_t CCondition::Signal(void)
{
    apl_int_t liRetCode = apl_cond_signal(&this->moCond);
    
    ACL_ASSERT(liRetCode == 0);
    
    return liRetCode;
}

apl_int_t CCondition::Broadcast(void)
{
    apl_int_t liRetCode = apl_cond_broadcast(&this->moCond);
    
    ACL_ASSERT(liRetCode == 0);
    
    return liRetCode;
}

///////////////////////////////////////////////// CSemaphore ///////////////////////////////////////////
CSemaphore::CSemaphore( apl_int_t aiInitValue )
{
    apl_int_t liRetCode = apl_sema_init(&this->moSema, aiInitValue);
    
    ACL_ASSERT(liRetCode == 0);
}
	
CSemaphore::~CSemaphore(void)
{
    apl_int_t liRetCode = apl_sema_destroy(&this->moSema);
    
    ACL_ASSERT(liRetCode == 0);
}
	
apl_int_t CSemaphore::Wait( CTimeValue const& aoTimeout )
{
    apl_int_t liRetCode = 0;
    apl_time_t liTimeout = (aoTimeout == CTimeValue::MAXTIME ? -1 : aoTimeout.Nsec() );
        
    ACL_RETRY_CHK( 
        (liRetCode = apl_sema_wait(&this->moSema, liTimeout) ) < 0, -1 );
    
    if (liTimeout == 0)
    {
        return liRetCode;
    }
    else
    {
        ACL_ASSERT(liRetCode == 0 || apl_get_errno() == APL_ETIMEDOUT);

        return liRetCode;
    }
}

apl_int_t CSemaphore::Wait( CTimestamp const& aoExpired )
{
    apl_int_t liRetCode = 0;
        
    ACL_RETRY_CHK( 
        (liRetCode = apl_sema_wait_abs(&this->moSema, aoExpired.Nsec() ) ) < 0, -1 );
    
    ACL_ASSERT(liRetCode == 0 || apl_get_errno() == APL_ETIMEDOUT);

    return liRetCode;
}

apl_int_t CSemaphore::Post(void)
{
    apl_int_t liRetCode = apl_sema_post(&this->moSema);
    
    ACL_ASSERT(liRetCode == 0);
    
    return liRetCode;
}

apl_int_t CSemaphore::GetValue(void)
{
    apl_int_t liValue = 0;
    apl_int_t liRetCode = apl_sema_getvalue(&this->moSema, &liValue);
    
    ACL_ASSERT(liRetCode == 0);
    
    return liValue;
}

ACL_NAMESPACE_END
