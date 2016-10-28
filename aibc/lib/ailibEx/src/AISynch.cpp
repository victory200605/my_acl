#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include "AISynch.h"
 
///start namespace
AIBC_NAMESPACE_START

////////////////////////////////////////////////////////////////////////////////////
AIMutexLock::AIMutexLock(bool abIsShared)
{
    int liRetCode;
    if (abIsShared)
    {
#if defined(_POSIX_THREAD_PROCESS_SHARED)
        pthread_mutexattr_t     ltMutexAttr;
 
        liRetCode = pthread_mutexattr_init(&ltMutexAttr);
        assert(0 == liRetCode);
 
        liRetCode = pthread_mutexattr_setpshared(&ltMutexAttr, PTHREAD_PROCESS_SHARED);
        assert(0 == liRetCode);
 
        liRetCode = pthread_mutex_init(&ctMutex, &ltMutexAttr);
        assert(0 == liRetCode);
 
        liRetCode = pthread_mutexattr_destroy(&ltMutexAttr);
        assert(0 == liRetCode);
#else
        assert(false); // NOT supported
#endif
    }
    else
    {
        liRetCode = pthread_mutex_init(&ctMutex, NULL);
        assert(0 == liRetCode);
    }
}
 
AIMutexLock::~AIMutexLock()
{
    int liRetCode = pthread_mutex_destroy(&ctMutex);
    assert(0 == liRetCode);
}
 
void AIMutexLock::Lock(void)
{
    int liRetCode = pthread_mutex_lock(&ctMutex);
    assert(0 == liRetCode);
}
 
void AIMutexLock::Unlock(void)
{
    int liRetCode = pthread_mutex_unlock(&ctMutex);
    assert(0 == liRetCode);
}
 
////////////////////////////////////////////////////////////////////////////////////
AISmartLock::AISmartLock(AIMutexLock &aoLock) : coCurLock(aoLock)
{
    coCurLock.Lock();
}
 
AISmartLock::~AISmartLock()
{
    coCurLock.Unlock();
}
 
////////////////////////////////////////////////////////////////////////////////////
 
AICondition::AICondition(bool abIsShared)
{
    int liRetCode;
    if (abIsShared)
    {
#if defined(PTHREAD_PROCESS_SHARED)
        pthread_condattr_t  ltCondAttr;
        liRetCode = pthread_condattr_init(&ltCondAttr);
        assert(0 == liRetCode);
 
        liRetCode = pthread_condattr_setpshared(&ltCondAttr, PTHREAD_PROCESS_SHARED);
        assert(0 == liRetCode);
 
        liRetCode = pthread_cond_init(&ctCond, &ltCondAttr);
        assert(0 == liRetCode);
 
        liRetCode = pthread_condattr_destroy(&ltCondAttr);
        assert(0 == liRetCode);
#else
        assert(false); // NOT supported
#endif
    }   
    else
    {
        liRetCode = pthread_cond_init(&ctCond, NULL);
        assert(0 == liRetCode);
    }
}
 
AICondition::~AICondition()
{
    int liRetCode = pthread_cond_destroy(&ctCond); 
    assert(0 == liRetCode);
}
 
int AICondition::Wait(AIMutexLock& aoMutex)
{
    int liRetCode;
 
    liRetCode = pthread_cond_wait(&ctCond, &(aoMutex.ctMutex));
 
    return (0 == liRetCode ? 0 : -1);
}
 
int AICondition::WaitFor(AIMutexLock& aoMutex, AITime_t aiTimeout)
{
    return WaitUntil(aoMutex, AICurTime() + aiTimeout);
}
 
int AICondition::WaitUntil(AIMutexLock& aoMutex, AITime_t aiExpired)
{
    int         liRetCode;
 
    timespec loTS;
    AI_TIME_TO_TS(loTS, aiExpired);
    
    liRetCode = pthread_cond_timedwait(&ctCond, &(aoMutex.ctMutex), &loTS);
 
    assert(EINVAL != liRetCode);
 
    return (ETIMEDOUT == liRetCode ? -1 : 0);
}
 
int AICondition::Signal()
{
    int liRetCode = pthread_cond_signal(&ctCond);
    assert(0 == liRetCode);
 
    return liRetCode;
}
 
int AICondition::Broadcast()
{
    int liRetCode = pthread_cond_broadcast(&ctCond);
    assert(0 == liRetCode);
 
    return liRetCode;
}
 
////////////////////////////////////////////////////////////////////////////////////
AIRWLock::AIRWLock()
    :ciRunning(0),ciReadWait(0),ciWriteWait(0)
{
    int liRetCode;
    
    liRetCode = pthread_mutex_init(&ctMutex, NULL);
    assert(0 == liRetCode);
 
    liRetCode = pthread_cond_init(&ctRCond, NULL);
    assert(0 == liRetCode);
 
    liRetCode = pthread_cond_init(&ctWCond, NULL);
    assert(0 == liRetCode);
}
 
AIRWLock::~AIRWLock()
{
    int liRetCode;
 
    liRetCode = pthread_mutex_destroy(&ctMutex);
    assert(0 == liRetCode);
 
    liRetCode = pthread_cond_destroy(&ctRCond);
    assert(0 == liRetCode);
 
    liRetCode = pthread_cond_destroy(&ctWCond);
    assert(0 == liRetCode);
}
 
void AIRWLock::RLock()
{
    int liRetCode = pthread_mutex_lock(&ctMutex);
    assert(0 == liRetCode);
 
    ++ciReadWait;
    
    while (ciRunning < 0 || ciWriteWait > 0)
    {
        pthread_cond_wait(&ctRCond, &ctMutex);
    }
    
    --ciReadWait;
    ++ciRunning;
    
    liRetCode = pthread_mutex_unlock(&ctMutex);
    assert(0 == liRetCode);
}
 
void AIRWLock::WLock()
{
    int liRetCode;
 
    liRetCode = pthread_mutex_lock(&ctMutex);
    assert(0 == liRetCode);
 
    ++ciWriteWait;
    
    while (ciRunning != 0)
    {
        pthread_cond_wait(&ctWCond, &ctMutex);
    }
    
    --ciWriteWait;
    ciRunning = -1;
    
    liRetCode = pthread_mutex_unlock(&ctMutex);
    assert(0 == liRetCode);
}
 
void AIRWLock::Unlock()
{
    int liRetCode;
 
    liRetCode = pthread_mutex_lock(&ctMutex);
    assert(0 == liRetCode);
 
    if (-1 == ciRunning)
    { // wlock
        ciRunning= 0;
    }
    else
    { // rlock
        --ciRunning;
    }
    
    if (0 == ciRunning)
    {
        if (ciWriteWait > 0)
        {
            liRetCode = pthread_cond_signal(&ctWCond);
            assert(0 == liRetCode);
        }
        else if (ciReadWait > 0)
        {
            liRetCode = pthread_cond_broadcast(&ctRCond);
            assert(0 == liRetCode);
        }
    }
    
    liRetCode = pthread_mutex_unlock(&ctMutex);
    assert(0 == liRetCode);
}
 
////////////////////////////////////////////////////////////////////////////////////
 
AISmartRLock::AISmartRLock(AIRWLock& aoRWLock)
    :coCurLock(aoRWLock)
{
    coCurLock.RLock();
}
 
AISmartRLock::~AISmartRLock()
{
    coCurLock.Unlock();
}
 
////////////////////////////////////////////////////////////////////////////////////
 
AISmartWLock::AISmartWLock(AIRWLock& aoRWLock)
    :coCurLock(aoRWLock)
{
    coCurLock.WLock();
}
 
AISmartWLock::~AISmartWLock()
{
    coCurLock.Unlock();
}
 
////////////////////////////////////////////////////////////////////////////////////
 
AIFileLock::AIFileLock(char const* apcFileName)
{
    ciFd = open(apcFileName, O_RDWR | O_CREAT, 0666);
    assert( ciFd >= 0 );
}
 
AIFileLock::~AIFileLock()
{
    close( ciFd );
}
 
int AIFileLock::Lock()
{
    int             liRetCode;
    struct flock    loMutexLock;
 
    memset(&loMutexLock, 0, sizeof(struct flock));
 
    loMutexLock.l_type   = F_WRLCK;
    loMutexLock.l_whence = SEEK_SET;
 
    do
    {
        liRetCode = fcntl(ciFd, F_SETLKW, &loMutexLock);
    }while(liRetCode < 0 && errno == EINTR);
 
    return  (liRetCode);
}
 
int AIFileLock::Unlock()
{
    struct flock    loMutexLock;
    
    memset(&loMutexLock, 0, sizeof(struct flock));
 
    loMutexLock.l_type   = F_UNLCK;
    loMutexLock.l_whence = SEEK_SET;
 
    if (fcntl(ciFd, F_SETLKW, &loMutexLock) < 0)
    {
        return  (-1);
    }
 
    return  (0);
}
 
////////////////////////////////////////////////////////////////////////////////////
#if defined(_POSIX_TIMEOUTS) && (_POSIX_TIMEOUTS >= 199506L) 
class AISemaphoreImpl
{
public:
    AISemaphoreImpl(bool abIsShared, unsigned aiInitValue)
    {
        int liRetCode = sem_init(&coSem, (int)abIsShared, aiInitValue);
        assert(0 == liRetCode);
    }
 
    ~AISemaphoreImpl()
    {
        int liRetCode = sem_destroy(&coSem);
        assert(0 == liRetCode);
    }
 
 
    int TryWait()
    {
        int liRetCode;
 
        do
        {
            liRetCode = sem_trywait(&coSem);
        }while (liRetCode != 0 && EINTR == errno);
 
        assert(0 == liRetCode || EAGAIN == errno);
 
        return liRetCode;
    }
 
    int Wait()
    {
        int liRetCode;
 
        do
        {
            liRetCode = sem_wait(&coSem);
        }while (liRetCode != 0 && EINTR == errno);
 
        assert(0 == liRetCode);
 
        return 0;
    }
 
    int WaitFor(AITime_t aiTimeout)
    {
        AITime_t    liTime = AICurTime() + aiTimeout;
        struct timespec loTS;
        int liRetCode;
 
        AI_TIME_TO_TS(loTS, liTime);
 
        do
        {
            liRetCode = sem_timedwait(&coSem, &loTS);
        }while (liRetCode != 0 && EINTR == errno); 
       
        assert(0 == liRetCode || ETIMEDOUT == errno);
 
        return liRetCode; 
    }
 
    int Post()
    {
        int liRetCode;
        
        liRetCode = sem_post(&coSem);
 
        assert(0 == liRetCode);
 
        return 0;
    }
 
    int GetValue()
    {
        int liVal;
        int liRetCode = sem_getvalue(&coSem, &liVal);
 
        assert(0 == liRetCode);
 
        return liVal;
    }
private:
    sem_t   coSem;
};
#else // !defined(_POSIX_TIMEOUTS)
class AISemaphoreImpl
{
public:
    AISemaphoreImpl(bool abIsShared, unsigned aiInitValue)
        :coMutex(abIsShared), coCond(abIsShared), ciValue(aiInitValue)
    {
    }
 
    int TryWait()
    {
        AISmartLock loLock(coMutex);
        
        if (ciValue <= 0)
        {
            return -1;
        }
        
        --ciValue;
        
        return 0;
    }
 
    int Wait()
    {
        AISmartLock loLock(coMutex);
 
        while (ciValue <= 0)
        {
            coCond.Wait(coMutex);
        }
 
        --ciValue;
        
        return 0;
    }
 
    int WaitFor(AITime_t aiTimeout)
    {
        AISmartLock lolock(coMutex);
        
        while(ciValue <= 0)
        {
            if(coCond.WaitFor(coMutex, aiTimeout) != 0)
            {
                return -1;
            }
        }
        
        --ciValue;
        
        return 0; 
    }
 
    int Post()
    {
        AISmartLock loLock(coMutex);
 
        ++ciValue;
        if (1 == ciValue)
        {
            coCond.Signal();
        }
        
        return 0;
    }
 
    int GetValue()
    {
        return ciValue;
    }
 
 
private:
    AIMutexLock     coMutex;
    AICondition     coCond;
    unsigned int    ciValue;
};
#endif
 
 
AISemaphore::AISemaphore(bool abIsShared, unsigned int aiInitValue)
{
    AI_NEW_INIT_ASSERT(cpoSema, AISemaphoreImpl, abIsShared, aiInitValue);
};
 
 
AISemaphore::~AISemaphore()
{
    AI_DELETE(cpoSema);
}
 
 
int AISemaphore::Wait()
{
    return cpoSema->Wait();
} 
 
 
int AISemaphore::TryWait()
{
    return cpoSema->TryWait();
}
 
int AISemaphore::WaitFor(AITime_t aiTimeout)
{
    return cpoSema->WaitFor(aiTimeout);
}
 
int AISemaphore::Post()
{
    return cpoSema->Post();
}
 
 
unsigned int AISemaphore::GetValue()
{
    return cpoSema->GetValue();
}

///end namespace
AIBC_NAMESPACE_END
