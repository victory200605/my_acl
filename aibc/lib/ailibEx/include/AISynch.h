/** @file AISynch.h
 */

#ifndef  __AILIBEX__AISYNCH_H__
#define  __AILIBEX__AISYNCH_H__

#include <pthread.h>
#include <semaphore.h>

#include "AILib.h"
#include "AITime.h"

///start namespace
AIBC_NAMESPACE_START

////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief This is class AINullMutexLock.
 */
class AINullMutexLock
{
public:
    void Lock(void) const   {};
    void Unlock(void) const {};
};

////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief This is class AIMutexLock.
 */
class AIMutexLock
{
public:
    AIMutexLock(bool abIsShared=false); ///<initialize the mutex
    ~AIMutexLock();                     ///<destroy the mutex

    void Lock(void);                    ///<lock the mutex
    void Unlock(void);                  ///<unlock the mutex 

private:
    pthread_mutex_t ctMutex;

    friend class AICondition;
};

////////////////////////////////////////////////////////////////////////////////////
class AISmartLock
{
public:
    AISmartLock(AIMutexLock& aoLock);    ///<initialize the smart lock
    ~AISmartLock();                      ///<destroy the smart lock

private:
    AIMutexLock& coCurLock;
};

////////////////////////////////////////////////////////////////////////////////////

class AICondition
{
public:
    AICondition(bool abIsShared=false);       ///<initialize condition variables
    ~AICondition();                           ///<destroy condition variables

    /**
     * @brief  Wait on a condition.
     *
     * @param aoMutex        IN - a mutex
     * 
     * @return 0:upon successful completion;-1:error
     */
    int Wait(AIMutexLock& aoMutex);

    /**
     * @brief Timedwait on a condition.
     *
     * @param aoMutex        IN - a mutex
     * @param aiTimeout      IN - timeout
     *
     * @return 0:upon successful completion;-1:timeout
     */
    int WaitFor(AIMutexLock& aoMutex, AITime_t aiTimeout);

    /**
     * @brief Timedwait on a condition
     *
     * @param aoMutex        IN - a mutex
     * @param aiTimeout      IN - timeout
     *
     * @return 0:upon successful completion;-1:timeout
     */
    int WaitUntil(AIMutexLock& aoMutex, AITime_t aiExpired);

    /**
     * @brief Signal a condition.
     *
     * @return Return 0 if successful;otherwise,an error number shall be returned to indicate the error.
     */
    int Signal(void);

    /**
     * @brief Broadcast a condition.
     *
     * @return Return 0 if successful;otherwise,an error number shall be returned to indicate the error.
     */
    int Broadcast(void);

private:
    pthread_cond_t  ctCond;
};

////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief This is class AIRWLock.
 */
class AIRWLock
{
public:
    AIRWLock();                ///<initialize a read-write lock object
    ~AIRWLock();               ///<destroy a read-write lock object

    /**
     * @brief Lock a read-write lock object for reading.
     */
    void RLock(void);          

    /**
     * @brief Lock a read-write lock object for writing.
     */
    void WLock(void);

    /**
     * @brief Unlock a read-write lock object.
     */
    void Unlock(void);

private:
    pthread_mutex_t ctMutex;
    pthread_cond_t  ctRCond;
    pthread_cond_t  ctWCond;
    int             ciRunning;
    int             ciReadWait;
    int             ciWriteWait;
};

////////////////////////////////////////////////////////////////////////////////////

class AISmartRLock
{
public:
    AISmartRLock(AIRWLock& aoLock);
    ~AISmartRLock();

private:
    AIRWLock  &coCurLock;
};

////////////////////////////////////////////////////////////////////////////////////

class AISmartWLock
{
public:
    AISmartWLock(AIRWLock& aoLock);
    ~AISmartWLock();

private:
    AIRWLock&   coCurLock;
};

////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief This is class AIFileLock.
 */
class AIFileLock
{
public:
    AIFileLock(char const* apcFileName);    ///<open an existed or create a new file for reading and writing
    ~AIFileLock();                          ///<close file descriptor

    /**
     * @brief Establish advisory record locking(establish write lock).
     *
     * @return Return value other than -1. 
     */
    int Lock();

    /**
     * @brief Delete the locking established before.
     *
     * @return 0:if success;-1:if unsuccess
     */
    int Unlock();

private:
    int ciFd;
};

////////////////////////////////////////////////////////////////////////////////////

class AISemaphoreImpl;
class AISemaphore
{
public:
    AISemaphore(bool abIsShared=false, unsigned aiInitValue=0);    ///<initialize an unnamed semaphore
    ~AISemaphore();                                                ///<destroy an unnamed semaphore 

    /**
     * @brief Lock a semaphore. 
     *
     * @return Return 0 if the calling process successfully.
     *         Return EAGAIN if the semaphore was already locked.
     */
    int TryWait();

    /**
     * @brief Lock a semaphore. 
     *
     * @return Return 0 if the calling process successfully.
     */
    int Wait();

    /**
     * @brief Lock a semaphore but this wait shall be terminated when the specified timeout expires.
     *
     * @param aiTimeout    IN - timeout
     *
     * @return Return 0 if the calling process successfully.
     *         Return ETIMEDOUT if the semaphore could not be locked before the specified timeout expired.
     */
    int WaitFor(AITime_t aiTimeout);

    /**
     * @brief Unlock a semaphore. 
     *
     * @return Return 0 if the calling process successfully.
     */
    int Post();

    /**
     * @brief Get the value of a semaphore. 
     *
     * @return Return 0 if the calling process successfully.
     */
    unsigned int GetValue();

private:
    AISemaphoreImpl*	cpoSema;
};

////////////////////////////////////////////////////////////////////////////////////

///end namespace
AIBC_NAMESPACE_END

#endif // __AILIBEX__AISYNCH_H__
 
