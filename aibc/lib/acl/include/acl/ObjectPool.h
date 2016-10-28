/**
 * @file ObjectPool.h
 */
#ifndef ACL_OBJECTPOOL_H
#define ACL_OBJECTPOOL_H

#include "acl/Utility.h"
#include "acl/Ring.h"
#include "acl/TimeValue.h"
#include "acl/Timestamp.h"
#include "acl/MsgQueue.h"

ACL_NAMESPACE_START

/**
 * struct CDefaultPoolConstructor
 */
struct CDefaultPoolConstructor
{
    template<typename T> apl_int_t operator () ( T& atObj )
    {
        return 0;
    }
};

/**
 * struct CDefaultPoolConstructor
 */
struct CDefaultPoolDestructor
{
    template<typename T> void operator () ( T& atObj )
    {
    }
};

template<typename T>
struct TObjectPoolHolder
{
    TObjectPoolHolder( T* apoVal );
    
    void Reference(void);
    
    bool Release(void);
    
    void Visited(void);
    
    ACL_RING_ENTRY(TObjectPoolHolder) moLink;
    
    CTimestamp moCreateTime;
    
    CTimestamp moLastVisitTime;
    
    apl_int_t miVisitCount;
    
    apl_int_t miReferenceCount;

    bool mbIsDetach;
    
    bool mbIsDestroy;
    
    T* mptVal;
    
    CLock moLock;
};

/**
 * Class CPointer: smartptr encaplusate the pointer points to the object of  objectpool
 */
template<typename T, typename PoolType>
class TObjectPoolPointer
{
public:
    typedef TObjectPoolHolder<T> HolderType;
    
public:
    /**
     * Default Constructor
     */
    TObjectPoolPointer(void);
    
    /**
     * Constructor
     *
     * @param [in] apoPool    the pointer to ObjectPool
     * @param [in] apoHolder  the pointer to Holder
     */ 
    TObjectPoolPointer( PoolType* apoPool, HolderType* apoHolder );
    
    /**
     * Copy constructor
     */
    TObjectPoolPointer( TObjectPoolPointer const& aoRhs );
    
    /**
     * Destructor 
     */
    ~TObjectPoolPointer(void);
    
    /**
     * Overloaded operator =
     */
    void operator = ( TObjectPoolPointer const& aoRhs );
    
    /**
     * Overloaded operator->
     *
     * @retval T* pointer points to object
     */
    T* operator -> (void);
    
    /**
     * Overloaded operator->
     *
     * @retval T* pointer points to object
     */
    T& operator * (void);
    
    /**
     * If the pointer which was encapsulated by smartptr is empty 
     *
     * @retval true the pointer is empty
     * @retval false the pointer isn't empty
     */
    bool IsEmpty(void);
    
    /**
     * Destroy object from pool
     */
    void Destroy(void);
    
    /**
     * Release the pointer which was encapsulated by smartptr, Push back to object pool
     */
    void Release(void);
    
    /**
     * Get the createtime of object.
     *
     * @return CTimestamp
     */
    CTimestamp GetCreateTime(void);
    
    /**
     * Get the lastVisitTime of object.
     *
     * @return CTimestamp
     */ 
    CTimestamp GetLastVisitTime(void);
    
    /**
     * Get the the VisitCount of object.
     *
     * @retval >=0
     */
    apl_size_t GetVisitCount(void);
    
    /**
     * Get the referenceCount of object.
     *
     * @retval >=0
     */
    apl_int_t  GetReferenceCount(void);

private:
    PoolType* mpoPool;
    HolderType* mpoHolder;
};

/**
 * Class TObjectPool
 */
template
<
    typename T,
    typename ConstructorType = CDefaultPoolConstructor,
    typename DestructorType  = CDefaultPoolDestructor
>
class TObjectPool
{
public:
    friend class TObjectPoolPointer<T, TObjectPool<T, ConstructorType, DestructorType> >;
    
    typedef TObjectPoolHolder<T> HolderType;
    //ATTENTION: this is thread-unsafe
    typedef TObjectPoolPointer<T, TObjectPool<T, ConstructorType, DestructorType> > PointerType;
    
public:
    /**
     * Constructor
     */
    TObjectPool(void);
    
    /**
     * Destructor
     */
    ~TObjectPool(void);
    
    /**
     * Initialize the objectpool.
     *
     * @param [in] auCapacity         the Capacity of objectpool
     * @param [in] abIsDynamic        if the object create dynamic
     *     -true.  the object create when object first use 
     *     -false. the object create when objectpool initialize 
     * @param [in] aoConstructor     the constructor object of objectpool  
     * @param [in] aoDestructor      the destructor object of objectpool  
     * @retval true If sucess
     * @retval false If fail
     */
    apl_int_t Initialize(
        apl_size_t auCapacity,
        bool abIsDynamic = false,
        ConstructorType const& aoConstructor = ConstructorType(),
        DestructorType const&  aoDestructor = DestructorType() );
 
    /**
     * Give the pointer to objectpool.
     *
     * @param [in] apoGived    the pointer points to object
     * @retval 0 If success
     * @retval -1 If fail
     */    
    apl_int_t Give( T* apoGived );
       
    /**
     * Close the objectPool.
     *
     * @param [in] aoTimeout    set the timeout value
     *     -CTimeValue::MAXTIME never timeout
     *     -CTimeValue::ZERO    none-block
     *     ->0                  set the timeout
     *  @retval 0 If success
     *  @retval -1 If fail  
     */ 
    void Close( CTimeValue const& aoTimeout = CTimeValue::MAXTIME );
    
    /**
     * Get the idel object.
     *
     * @param [in] aoTimeout    set the timeout value
     *     -CTimeValue::MAXTIME never timeout
     *     -CTimeValue::ZERO    none-block
     *     ->0                  set the timeout
     * @return PointerType. smartptr of objectpool
     */
    PointerType GetIdle( CTimeValue const& aoTimeout = CTimeValue::ZERO );
 
    /**
     * Resize the size of objectpool.
     *
     * @param [in] auCapacity    reset the Capacity of objectpool
     * @retval 0 If success
     * @retval -1 If fail  
     */
    apl_int_t Resize( apl_size_t auCapacity );
    
    /**
     * Get the number of object in the objectpool.
     *
     * @retval >=0.
     */
    apl_size_t GetSize(void);
    
    /**
     * Get the number of idle object in the objectpool.
     *
     * @retval >=0.
     */
    apl_size_t GetIdleSize(void);
    
    /**
     * Get the number of busy object in the objectpool.
     *
     * @retval >=0.
     */
    apl_size_t GetBusySize(void);
    
    /**
     * Get the Capacity of object in the objectpool.
     *
     * @retval >=0. 
     */
    apl_size_t GetCapacity(void);

protected:
    HolderType* CreateHolder(void);
    
    void DeleteHolder( HolderType* apoHolder );
    
    void Recycle( HolderType* apoHolder );
    
    void PushToBusy( HolderType* apoHolder );
    
    void EraseFromBusy( HolderType* apoHolder );
    
    void PushToIdle( HolderType* apoHolder );
    
    HolderType* PopFromIdle(void);
    
private:
    apl_size_t muCapacity;
    
    apl_size_t muIdleSize;
    
    apl_size_t muBusySize;
    
    bool mbIsClosed;
    
    bool mbIsDynamic;

    ACL_RING_HEAD(IdleHeader, HolderType) moIdle;
    
    ACL_RING_HEAD(BusyHeader, HolderType) moBusy;
    
    CLock moLock;
    
    CCondition moCond;
    
    ConstructorType* mpoConstructor;
    
    DestructorType* mpoDestructor;
    
    static CLock coRecycleLock;
};

///////////////////////////////////////////// Holder Impletement ///////////////////////////////////
template<typename T>
inline TObjectPoolHolder<T>::TObjectPoolHolder( T* aptVal )
    : miVisitCount(0)
    , miReferenceCount(0)
    , mbIsDetach(false)
    , mbIsDestroy(false)
    , mptVal(aptVal)
{
    this->moCreateTime.Update();
    this->moLastVisitTime.Update();
    
    ACL_RING_ELEM_INIT(this, moLink);
}

template<typename T>
inline void TObjectPoolHolder<T>::Reference()
{
    //TSmartLock<CLock> loLock(this->moLock);

    this->miReferenceCount++;
}

template<typename T>
inline bool TObjectPoolHolder<T>::Release()
{
    //TSmartLock<CLock> loLock(this->moLock);
    
    this->miReferenceCount--;
    
    if (this->miReferenceCount == 0)
    {
        return true;
    }
    
    return false;
}

template<typename T>
inline void TObjectPoolHolder<T>::Visited(void)
{
    this->miVisitCount++;
    this->moLastVisitTime.Update();
}

///////////////////////////////////////////// Pointer Impletement ///////////////////////////////////
template<typename T, typename PoolType>
inline TObjectPoolPointer<T, PoolType>::TObjectPoolPointer(void)
    : mpoPool(NULL)
    , mpoHolder(NULL)
{
}

template<typename T, typename PoolType>
inline TObjectPoolPointer<T, PoolType>::TObjectPoolPointer( PoolType* apoPool, HolderType* apoHolder )
    : mpoPool(apoPool)
    , mpoHolder(apoHolder)
{
    if (this->mpoHolder != NULL)
    {
        this->mpoHolder->Reference();
    }
}

template<typename T, typename PoolType>
inline TObjectPoolPointer<T, PoolType>::TObjectPoolPointer( const TObjectPoolPointer& aoRhs )
    : mpoPool(aoRhs.mpoPool)
    , mpoHolder(aoRhs.mpoHolder)
{
    if (this->mpoHolder != NULL)
    {
        this->mpoHolder->Reference();
    }
}

template<typename T, typename PoolType>
inline TObjectPoolPointer<T, PoolType>::~TObjectPoolPointer(void)
{
    this->Release();
}

template<typename T, typename PoolType>
inline void TObjectPoolPointer<T, PoolType>::operator = ( TObjectPoolPointer const& aoRhs )
{
    this->Release();
    
    this->mpoHolder = aoRhs.mpoHolder;
    this->mpoPool = aoRhs.mpoPool;
    
    if (this->mpoHolder != NULL)
    {
        this->mpoHolder->Reference();
    }
}

template<typename T, typename PoolType>
inline T* TObjectPoolPointer<T, PoolType>::operator -> (void)
{
    return this->mpoHolder->mptVal;
}

template<typename T, typename PoolType>
inline T& TObjectPoolPointer<T, PoolType>::operator * (void)
{
    return *this->mpoHolder->mptVal;
}

template<typename T, typename PoolType>
inline bool TObjectPoolPointer<T, PoolType>::IsEmpty(void)
{
    return this->mpoHolder == NULL;
}

template<typename T, typename PoolType>
inline void TObjectPoolPointer<T, PoolType>::Release(void)
{
    if ( this->mpoHolder != NULL && this->mpoHolder->Release() )
    {
        this->mpoPool->Recycle(this->mpoHolder);
    }
    
    this->mpoHolder = NULL;
}

template<typename T, typename PoolType>
inline void TObjectPoolPointer<T, PoolType>::Destroy(void)
{
    this->mpoHolder->mbIsDestroy = true;
}

template<typename T, typename PoolType>
inline CTimestamp TObjectPoolPointer<T, PoolType>::GetCreateTime(void)
{
    return this->mpoHolder->moCreateTime;
}

template<typename T, typename PoolType>
inline CTimestamp TObjectPoolPointer<T, PoolType>::GetLastVisitTime(void)
{
    return this->mpoHolder->moLastVisitTime;
}

template<typename T, typename PoolType>
inline apl_size_t TObjectPoolPointer<T, PoolType>::GetVisitCount(void)
{
    return this->mpoHolder->miVisitCount;
}

template<typename T, typename PoolType>
inline apl_int_t TObjectPoolPointer<T, PoolType>::GetReferenceCount(void)
{
    return this->mpoHolder->miReferenceCount;
}

//////////////////////////////////////////// ObjectPool Impletement ///////////////////////////////////////
template
<
    typename T,
    typename ConstructorType,
    typename DestructorType
>
CLock TObjectPool<T, ConstructorType, DestructorType>::coRecycleLock;

template
<
    typename T,
    typename ConstructorType,
    typename DestructorType
>
TObjectPool<T, ConstructorType, DestructorType>::TObjectPool(void)
    : muCapacity(0)
    , muIdleSize(0)
    , muBusySize(0)
    , mbIsClosed(false)
    , mbIsDynamic(true)
    , mpoConstructor(NULL)
    , mpoDestructor(NULL)
{
    ACL_RING_INIT(&this->moBusy, HolderType, moLink);
    ACL_RING_INIT(&this->moIdle, HolderType, moLink);
}

template
<
    typename T,
    typename ConstructorType,
    typename DestructorType
>
TObjectPool<T, ConstructorType, DestructorType>::~TObjectPool(void)
{
    this->Close(0);
    
    ACL_DELETE(this->mpoConstructor);
    ACL_DELETE(this->mpoDestructor);
}

template
<
    typename T,
    typename ConstructorType,
    typename DestructorType
>
apl_int_t TObjectPool<T, ConstructorType, DestructorType>::Initialize(
    apl_size_t auCapacity,
    bool abIsDynamic,
    ConstructorType const& aoConstructor,
    DestructorType const&  aoDestructor )
{
    HolderType* lpoHolder = NULL;
    
    ACL_DELETE(this->mpoConstructor);
    ACL_NEW_ASSERT(this->mpoConstructor, ConstructorType(aoConstructor) );
    ACL_DELETE(this->mpoDestructor);
    ACL_NEW_ASSERT(this->mpoDestructor, DestructorType(aoDestructor) );
    
    this->muCapacity = auCapacity;
    this->mbIsClosed = false;
    this->mbIsDynamic = abIsDynamic;
    
    if (!abIsDynamic)
    {
        for ( apl_size_t liN = 0; liN < auCapacity; liN++ )
        {
            if ( (lpoHolder = this->CreateHolder() ) == NULL)
            {
                return -1;
            }

            this->PushToIdle(lpoHolder);
        }
    }
    
    return 0;
}

template
<
    typename T,
    typename ConstructorType,
    typename DestructorType
>
apl_int_t TObjectPool<T, ConstructorType, DestructorType>::Give( T* apoGived )
{
    HolderType* lpoHolder = NULL;
    
    TSmartLock<CLock> loGuard(this->moLock);
    
    if (this->muBusySize + this->muIdleSize >= this->muCapacity)
    {
        return -1;
    }

    ACL_NEW_ASSERT(lpoHolder, HolderType(apoGived) );
    this->PushToIdle(lpoHolder);

    return 0;
}

template
<
    typename T,
    typename ConstructorType,
    typename DestructorType
>
void TObjectPool<T, ConstructorType, DestructorType>::Close( CTimeValue const& aoTimeout )
{
    CTimeValue  loTimeout(aoTimeout);
    HolderType* lpoCurr = NULL;
    
    if (!this->mbIsClosed)
    {
        this->mbIsClosed = true;
        
        this->Resize(0);
        
        while ( loTimeout.Sec() > 0 && this->muBusySize > 0 )
        {
            apl_sleep(APL_TIME_SEC);
            
            this->Resize(0);
            
            --loTimeout;
        }
        
        //Set all busy object by detach state
        TSmartLock<CLock> loGuard(coRecycleLock);//mutex recycle option
        
        {
            TSmartLock<CLock> loLock(this->moLock);
            
            ACL_RING_FOREACH(lpoCurr, &this->moBusy, HolderType, moLink)
            {
                lpoCurr->mbIsDetach = true;
            }
            
            ACL_RING_INIT(&this->moBusy, HolderType, moLink);
            
            this->muIdleSize = 0;
        }
    }
}

template
<
    typename T,
    typename ConstructorType,
    typename DestructorType
>
typename TObjectPool<T, ConstructorType, DestructorType>::PointerType
TObjectPool<T, ConstructorType, DestructorType>::GetIdle( CTimeValue const& aoTimeout )
{
    HolderType* lpoHolder = NULL;
    
    this->moLock.Lock();
    
    if (this->muIdleSize > 0)
    {
        lpoHolder = this->PopFromIdle();
        ACL_ASSERT(lpoHolder != NULL);
    }
    else if (this->muBusySize >= this->muCapacity)
    {  
        while(true)
        {
            apl_int_t liRetCode = this->moCond.Wait(this->moLock, aoTimeout);
            
            if (this->muIdleSize > 0)
            {
                lpoHolder = this->PopFromIdle();
                ACL_ASSERT(lpoHolder != NULL);
                break;
            }
            else if (liRetCode != 0)
            {
                break;
            }
        }
    }
    else
    {
        this->muBusySize++;
        
        this->moLock.Unlock();
        //Create another, and release lock
        lpoHolder = this->CreateHolder();
        this->moLock.Lock();
        
        this->muBusySize--;
    }
    
    if (lpoHolder != NULL)
    {
        this->PushToBusy(lpoHolder);
    }
    
    this->moLock.Unlock();
    
    if (lpoHolder != NULL)
    {
        lpoHolder->Visited();
    }
    
    return PointerType(this, lpoHolder);
}

template
<
    typename T,
    typename ConstructorType,
    typename DestructorType
>
apl_int_t TObjectPool<T, ConstructorType, DestructorType>::Resize( apl_size_t auCapacity )
{
    HolderType* lpoHolder = NULL;
    
    this->moLock.Lock();
    
    if (this->muBusySize + this->muIdleSize <= auCapacity)
    {
        this->muCapacity = auCapacity;
        
        if (!this->mbIsDynamic)
        {
            while(this->muBusySize + this->muIdleSize < this->muCapacity)
            {
                this->moLock.Unlock();
                //Create another, and release lock
                if ( (lpoHolder = this->CreateHolder() ) == NULL)
                {
                    return -1;
                }
                this->moLock.Lock();
                
                this->PushToIdle(lpoHolder);
            }
        }
    }
    else
    {
        this->muCapacity = auCapacity;
        
        while(this->muBusySize + this->muIdleSize > this->muCapacity)
        {
            if ( (lpoHolder = this->PopFromIdle() ) == NULL)
            {
                break;
            }
            
            this->moLock.Unlock();//Release lock, because destructor may be block
            this->DeleteHolder(lpoHolder);
            this->moLock.Lock();
        }
    }
    
    this->moLock.Unlock();
    
    return 0;
}

template
<
    typename T,
    typename ConstructorType,
    typename DestructorType
>
apl_size_t TObjectPool<T, ConstructorType, DestructorType>::GetSize(void)
{
    return this->muIdleSize + this->muBusySize;
}

template
<
    typename T,
    typename ConstructorType,
    typename DestructorType
>
apl_size_t TObjectPool<T, ConstructorType, DestructorType>::GetIdleSize(void)
{
    return this->muIdleSize;
}

template
<
    typename T,
    typename ConstructorType,
    typename DestructorType
>
apl_size_t TObjectPool<T, ConstructorType, DestructorType>::GetBusySize(void)
{
    return this->muBusySize;
}

template
<
    typename T,
    typename ConstructorType,
    typename DestructorType
>
apl_size_t TObjectPool<T, ConstructorType, DestructorType>::GetCapacity(void)
{
    return this->muCapacity;
}

template
<
    typename T,
    typename ConstructorType,
    typename DestructorType
>
typename TObjectPool<T, ConstructorType, DestructorType>::HolderType* 
TObjectPool<T, ConstructorType, DestructorType>::CreateHolder(void)
{
    T* lptVal = NULL;
    HolderType* lpoHolder = NULL;
    
    if (lptVal == NULL)
    {
        ACL_NEW_ASSERT(lptVal, T);
    }
        
    if ( (*this->mpoConstructor)(*lptVal) != 0)
    {
        ACL_DELETE(lptVal);
        
        return NULL;
    }
    
    ACL_NEW_ASSERT(lpoHolder, HolderType(lptVal) );
    
    return lpoHolder;
}

template
<
    typename T,
    typename ConstructorType,
    typename DestructorType
>
void TObjectPool<T, ConstructorType, DestructorType>::DeleteHolder( HolderType* apoHolder )
{
    (*this->mpoDestructor)(*apoHolder->mptVal);

    ACL_DELETE(apoHolder->mptVal);
    ACL_DELETE(apoHolder);
}

template
<
    typename T,
    typename ConstructorType,
    typename DestructorType
>
void TObjectPool<T, ConstructorType, DestructorType>::Recycle( HolderType* apoHolder )
{
    HolderType* lpoHolder = NULL;
    
    do
    {
        TSmartLock<CLock> loGuard(coRecycleLock);//mutex recycle option
        
        if (!apoHolder->mbIsDetach)
        {
            TSmartLock<CLock> loGuard(this->moLock);

            this->EraseFromBusy(apoHolder);
            
            if (this->muIdleSize + this->muBusySize >= this->muCapacity)
            {
                break;
            }
            
            if (apoHolder->mbIsDestroy)
            {
                if (!this->mbIsDynamic)
                {
                    while(this->muBusySize + this->muIdleSize < this->muCapacity)
                    {
                        this->muBusySize++;
                        this->moLock.Unlock();
                        //Create another, and release lock
                        lpoHolder = this->CreateHolder();
                        this->moLock.Lock();
                        this->muBusySize--;
                        
                        if (lpoHolder == NULL)
                        {
                            break;
                        }
                        
                        this->PushToIdle(lpoHolder);
                    }
                }
                
                break;
            }
            
            this->PushToIdle(apoHolder);
            
            return;
        }
    }while(false);
    
    this->DeleteHolder(apoHolder);
}

template
<
    typename T,
    typename ConstructorType,
    typename DestructorType
>
void TObjectPool<T, ConstructorType, DestructorType>::PushToBusy( HolderType* apoHolder )
{      
    ACL_RING_INSERT_TAIL(&this->moBusy, apoHolder, HolderType, moLink);
    this->muBusySize++;
}
    
template
<
    typename T,
    typename ConstructorType,
    typename DestructorType
>
void TObjectPool<T, ConstructorType, DestructorType>::EraseFromBusy( HolderType* apoHolder )
{    
    ACL_RING_REMOVE(apoHolder, moLink);
    this->muBusySize--;
}

template
<
    typename T,
    typename ConstructorType,
    typename DestructorType
>
void TObjectPool<T, ConstructorType, DestructorType>::PushToIdle( HolderType* apoHolder )
{        
    ACL_RING_INSERT_TAIL(&this->moIdle, apoHolder, HolderType, moLink);

    this->muIdleSize++;
    
    this->moCond.Signal();
}
    
template
<
    typename T,
    typename ConstructorType,
    typename DestructorType
>
typename TObjectPool<T, ConstructorType, DestructorType>::HolderType* 
TObjectPool<T, ConstructorType, DestructorType>::PopFromIdle(void)
{
    HolderType* lpoHolder = NULL;

    if (ACL_RING_IS_EMPTY(&this->moIdle, HolderType, moLink) )
    {
        return NULL;
    }
    
    lpoHolder = ACL_RING_FIRST(&this->moIdle);
         
    ACL_RING_REMOVE(lpoHolder, moLink);

    this->muIdleSize--;
    
    return lpoHolder;
}

ACL_NAMESPACE_END

#endif //ACL_OBJECTPOOL_H
