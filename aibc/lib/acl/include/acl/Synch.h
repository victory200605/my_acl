/**
 * @file Synch.h
 */

#ifndef ACL_SYNCH_H
#define ACL_SYNCH_H

#include "acl/Utility.h"
#include "acl/TimeValue.h"
#include "acl/Timestamp.h"

ACL_NAMESPACE_START

///////////////////////////////////////////////// CNullLock ///////////////////////////////////////////

/**
 * class CNullLock
 */
class CNullLock
{
public:

    /**
     * @brief A constructor.
     */
    CNullLock(void) {};
    
    /**
     * @brief Lock.
     *
     * @retval 0 Upon successful completion.
     */
    apl_int_t Lock();
    
    /**
     * @brief Unlock.
     *
     * @retval 0 Upon successful completion.
     */
    apl_int_t Unlock(void);

private:
    CNullLock( CNullLock const& );
    CNullLock operator = ( CNullLock const& );
};

///////////////////////////////////////////////// CLock ///////////////////////////////////////////
/**
 * class CLock
 */
class CLock
{
    friend class CCondition;
    
public:
    /**
     * @brief A constructor. Initialize a mutex.
     */
    CLock(void);
    
    /**
     * @brief A destructor. Destroy a mutex.
     */
    ~CLock(void);
    
    /**
     * @brief Lock a mutex.
     *
     * @retval 0 Upon successful completion.
     * @retval -1 If an error is detected.
     */
    apl_int_t Lock();
    
    /**
     * @brief Unlock a mutex.
     *
     * @retval 0 Upon successful completion.
     */
    apl_int_t Unlock(void);

protected:
    operator apl_mutex_t* (void);

private:
    CLock( CLock const& );
    CLock& operator = ( CLock const& );

private:
    apl_mutex_t moMutex;
};

///////////////////////////////////////////////// CSpinLock ///////////////////////////////////////////
/**
 * class CSpinLock
 */
class CSpinLock
{
public:
    /**
     * @brief A constructor.Initialize a spin lock object.
     */
    CSpinLock(void);
    
    /**
     * @brief A destructor.Destroy a spin lock object.
     */
    ~CSpinLock(void);
    
    /**
     * @brief Lock a spin lock object.
     *
     * @retval 0 Upon successful completion.
     */
    apl_int_t Lock(void);
    
    /**
     * @brief Unlock a spin lock object.
     *
     * @retval 0 Upon successful completion.
     */
    apl_int_t Unlock(void);

private:
    CSpinLock( CSpinLock const& );
    CSpinLock& operator = ( CSpinLock const& );

private:
    apl_spin_t moSpin;
};

///////////////////////////////////////////////// CFileLock ///////////////////////////////////////////
/**
 * class CFileLock
 */
class CFileLock
{
public:
    /**
     * @brief A construtor.Open a file.
     */
    CFileLock( const char* apcName );
    
    /**
     * @brief A destructor.Close a file.
     */
    ~CFileLock(void);
    
    /**
     * @brief Exclusive lock a file (blocking).
     * 
     * @retval 0 Upon successful completion.
     */
    apl_int_t Lock(void);
    
    /**
     * @brief Exclusive lock a file (non-blocking)
     *
     * @retval 0 Upon successful completion.
     * @retval -1 If get errno APL_EACCES and APL_EAGAIN.
     */
    apl_int_t TryLock(void);
    
    /**
     * @brief Unlock a file.
     *
     * @retval 0 Upon successful completion.
     */
    apl_int_t Unlock(void);

private:
    CFileLock( CFileLock const& );
    CFileLock operator = ( CFileLock const& );
    
private:
    apl_handle_t miHandle;
};

///////////////////////////////////////////////// CNullRWLock ///////////////////////////////////////////
/**
 * class CNullRWLock
 */
class CNullRWLock
{
public:
    /**
     * @brief A construtor.
     */
    CNullRWLock(void) {};
    
    /**
     * @brief Shared read lock.
     *
     * @param [in] aoTimeout    timeout
     * - =CTimeValue :: MAXTIME : never timeout
     * - =CTimeValue :: ZERO : non-blocking
     * - >0 : blocking
     *
     * @retval 0 On any condition.
     */
    apl_int_t RLock( CTimeValue const& aoTimeout = CTimeValue::MAXTIME );
    
    /**
     * @brief Exclusive write lock.
     *
     * @param [in] aoTimeout    timeout
     * - =CTimeValue :: MAXTIME : never timeout
     * - =CTimeValue :: ZERO : non-blocking
     * - >0 : blocking
     *
     * @retval 0 On any condition.
     */
    apl_int_t WLock( CTimeValue const& aoTimeout = CTimeValue::MAXTIME );
    
    /** 
     * @brief Unlock.
     *
     * @retval 0 On any condition.
     */
    apl_int_t Unlock(void);

private:
    CNullRWLock( CNullRWLock const& );
    CNullRWLock operator = ( CNullRWLock const& );
};

///////////////////////////////////////////////// CRWLock ///////////////////////////////////////////
/**
 * class CRWLock
 */
class CRWLock
{
public:
    /**
     * @brief A constructor.Initialize a read-write lock object.
     */
    CRWLock(void);
    
    /**
     * @brief A destructor.Destroy a read-write lock object.  
     */
    ~CRWLock(void);
    
    /**
     * @brief Shared read lock.
     *
     * @param [in] aoTimeout    timeout
     * - =CTimeValue :: MAXTIME : never timeout
     * - =CTimeValue :: ZERO : non-blocking
     * - >0 : blocking
     *
     * @retval 0 Upon successful completion.
     * @retval -1 Otherwise.
     */
    apl_int_t RLock( CTimeValue const& aoTimeout = CTimeValue::MAXTIME );
    
    /**
     * @brief Exclusive write lock.
     *
     * @param [in] aoTimeout    timeout
     * - =CTimeValue :: MAXTIME : never timeout
     * - =CTimeValue :: ZERO : non-blocking
     * - >0 : blocking
     *
     * @retval 0 Upon successful completion.
     * @retval -1 Otherwise.
     */
    apl_int_t WLock( CTimeValue const& aoTimeout = CTimeValue::MAXTIME );
    
    /**
     * @brief Unlock a read-write lock object.
     *
     * @retval 0 Upon successful completion.
     */
    apl_int_t Unlock(void);

private:
    CRWLock( CRWLock const& );
    CRWLock& operator = ( CRWLock const& );
    
private:
    apl_rwlock_t moRWLock;
};

///////////////////////////////////////////////// CFileRWLock ///////////////////////////////////////////
/**
 * class CFileRWLock
 */
class CFileRWLock
{
public:
    /**
     * @brief A constructor.Open a file.
     */
    CFileRWLock( const char* apcName );
    
    /**
     * @brief A destructor.Close a file.
     */
    ~CFileRWLock(void);
    
    /**
     * @brief Shared read lock (blocking).
     *
     * @retval 0 Upon successful completion.
     */
    apl_int_t RLock(void);
    
    /**
     * @brief Shared read lock (non-blocking).
     *
     * @retval 0 Upon successful completion.
     * @retval -1 If get errno APL_EACCES and APL_EAGAIN.
     */
    apl_int_t RTryLock(void);
    
    /**
     * @brief Exclusive write lock (blocking).
     *
     * @retval 0 Upon successful completion.
     */
    apl_int_t WLock();
    
    /**
     * @brief Exclusive write lock (non-blocking).
     *
     * @retval 0 Upon successful completion.
     * @retval -1 If get errno APL_EACCES and APL_EAGAIN.
     */
    apl_int_t WTryLock(void);
    
    /**
     * @brief Unlock.
     *
     * @retval 0 Upon successful completion.
     */
    apl_int_t Unlock(void);

private:
    CFileRWLock( CFileRWLock const& );
    CFileRWLock operator = ( CFileRWLock const& );
    
private:
    apl_handle_t miHandle;
};

///////////////////////////////////////////////// TSmartLock ///////////////////////////////////////////
/**
 * class TSmartLock
 */
template<typename LockType> class TSmartLock
{
public:
    /**
     * @brief A constructor.Smart lock.
     *
     * @tparam [in] aoLock    lock type
     */
    TSmartLock( LockType& aoLock )
        : moLock(aoLock)
        , mbIsOwner(false)
    {
        this->Lock();
    }
    
    /**
     * @brief acquire smart lock.
     */
    apl_int_t Lock(void)
    {
        if (this->moLock.Lock() == 0)
        {
            this->mbIsOwner = true;
            return 0;
        }
        else
        {
            this->mbIsOwner = false;
            return -1;
        }
    }
    
    /**
     * @brief try acquire smart lock.
     */
    apl_int_t TryLock(void)
    {
        if (this->moLock.TryLock() == 0)
        {
            this->mbIsOwner = true;
            return 0;
        }
        else
        {
            this->mbIsOwner = false;
            return -1;
        }
    }
    
    /**
     * @brief release smart lock.
     */
    apl_int_t Unlock(void)
    {
        if (this->mbIsOwner)
        {
            this->mbIsOwner = false;
            return this->moLock.Unlock();
        }
        else
        {
            return -1;
        }
    }
    
    /**
     * @brief A destructor.Destroy smart lock.
     */
    ~TSmartLock(void)
    {
        this->Unlock();
    }

private:
    LockType& moLock;
    
    bool mbIsOwner;
};

///////////////////////////////////////////////// TSmartLock ///////////////////////////////////////////
/**
 * class TSmartRLock
 */
template<typename TRWLock> class TSmartRLock
{
public:
    /**
     * @brief A constructor.Smart shared read lock.
     *
     * @param [in] aoLock    a read write lock
     */
    TSmartRLock( TRWLock& aoLock )
        : moLock(aoLock)
        , mbIsOwner(false)
    {
        this->Lock();
    }
    
    /**
     * @brief acquire smart lock.
     */
    apl_int_t Lock(void)
    {
        if (this->moLock.RLock() == 0)
        {
            this->mbIsOwner = true;
            return 0;
        }
        else
        {
            this->mbIsOwner = false;
            return -1;
        }
    }

    /**
     * @brief release smart lock.
     */
    apl_int_t Unlock(void)
    {
        if (this->mbIsOwner)
        {
            this->mbIsOwner = false;
            return this->moLock.Unlock();
        }
        else
        {
            return -1;
        }
    }
    
    /**
     * @brief A destructor.Destroy shared read lock.
     */
    ~TSmartRLock(void)
    {
        this->Unlock();
    }

private:
    TRWLock& moLock;
    
    bool mbIsOwner;
};

///////////////////////////////////////////////// TSmartWLock ///////////////////////////////////////////
/**
 * class TSmartWLock
 */
template<typename TRWLock> class TSmartWLock
{
public:
    /**
     * @brief A constructor.Smart exclusive write lock.
     *
     * @param [in] aoLock    a read write lock
     */
    TSmartWLock( TRWLock& aoLock )
        : moLock(aoLock)
        , mbIsOwner(false)
    {
        this->Lock();
    }
    
    /**
     * @brief acquire smart lock.
     */
    apl_int_t Lock(void)
    {
        if (this->moLock.WLock() == 0)
        {
            this->mbIsOwner = true;
            return 0;
        }
        else
        {
            this->mbIsOwner = false;
            return -1;
        }
    }

    /**
     * @brief release smart lock.
     */
    apl_int_t Unlock(void)
    {
        if (this->mbIsOwner)
        {
            this->mbIsOwner = false;
            return this->moLock.Unlock();
        }
        else
        {
            return -1;
        }
    }
    
    /**
     * @brief A destructor.Destroy exclusive write lock.
     */
    ~TSmartWLock(void)
    {
        this->Unlock();
    }

private:
    TRWLock& moLock;
    
    bool mbIsOwner;
};


///////////////////////////////////////////////// CConndition ///////////////////////////////////////////
/**
 * class CCondition
 */
class CCondition
{
public:
    /**
     * @brief A constructor.Initialize condition variables.
     */
    CCondition(void);

    /**
     * @brief A destructor.Destroy condition variables.
     */
    ~CCondition(void);
    
    /**
     * @brief Wait on a condition.
     *
     * @param [in] aoLock      a mutex lock
     * @param [in] aoTimeout   timeout
     * - =CTimeValue :: MAXTIME : never timeout
     * - =CTimeValue :: ZERO : non-blocking
     * - >0 : blocking
     *
     * @retval 0 Upon successful completion.
     * @retval -1 Otherwise.
     */
    apl_int_t Wait( CLock& aoLock, CTimeValue const& aoTimeout = CTimeValue::MAXTIME );
    
    /**
     * @brief Wait a future time on a condition.
     *
     * @param [in] aoLock    a mutex lock
     * @param [in] aoExpired a future time
     *
     * @retval 0 Upon successful completion.
     * @retval -1 Otherwise.
     */
    apl_int_t Wait( CLock& aoLock, CTimestamp const& aoExpired );

    /**
     * @brief Signal a condition and unblock at least one of the threads that are blocked on the specified condition variable.
     *
     * @retval 0 Upon successful completion.
     */
    apl_int_t Signal(void);
    
    /**
     * @brief Broadcaset a condition and unblock all threads currently blocked on the specified condition variable. 
     *
     * @retval 0 Upon successful completion.
     */
    apl_int_t Broadcast(void);

private:
    CCondition( CCondition const& );
    CCondition operator = ( CCondition const& );

private:
    apl_cond_t moCond;
};

///////////////////////////////////////////////// CSemaphore ///////////////////////////////////////////
/**
 * class CSemaphore
 */
class CSemaphore
{
public:
    /**
     * @brief A constructor.Initialize an unnamed semaphore.
     *
     * @param [in] aiInitValue    the value of the initialized semaphore
     */
    CSemaphore( apl_int_t aiInitValue = 0 );
    
    /**
     * @brief A destructor.Destroy an unnamed semaphore.
     */
    ~CSemaphore(void);
    
    /**
     * @brief Wait on a semaphore.
     *
     * @param [in] aoTimeout    timeout
     * - =CTimeValue :: MAXTIME : never timeout
     * - =CTimeValue :: ZERO : non-blocking
     * - >0 : blocking
     *
     * @retval 0 Upon successful completion.
     * @retval -1 Otherwise.
     */
    apl_int_t Wait( CTimeValue const& aoTimeout = CTimeValue::MAXTIME );
        
    /**
     * @brief Wait a future time on a semaphore.
     *
     * @param [in] aoExpired a future time
     *
     * @retval 0 Upon successful completion.
     * @retval -1 Otherwise.
     */
    apl_int_t Wait( CTimestamp const& aoExpired );
        
    /**
     * @brief Unlock a semaphore.
     *
     * @retval 0 Upon successful completion.
     */
    apl_int_t Post(void);
    
    /**
     * @brief Get the value of a semaphore.
     *
     * @retval 0 Upon successful completion.
     */
    apl_int_t GetValue(void);

private:
    CSemaphore( CSemaphore const& );
    CSemaphore operator = ( CSemaphore const& );
    
private:
    apl_sema_t moSema;
};

ACL_NAMESPACE_END

#endif//ACL_SYNCH_H
