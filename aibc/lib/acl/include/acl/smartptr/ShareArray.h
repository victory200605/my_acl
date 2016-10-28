/**
 * @file ShareArray.h
 */
#ifndef ACL_SMARTPTR_SHAREARRAY_H
#define ACL_SMARTPTR_SHAREARRAY_H

#include "acl/Utility.h"
#include "acl/Synch.h"
#include "acl/Number.h"
#include "acl/Operators.h"
#include "acl/stl/algorithm.h"

ACL_NAMESPACE_START

struct CDefaultArrayDestoryer
{
    template<typename T> static void Destroy( T* p )
    {
        ACL_DELETE_N(p);
    }
};

template<typename T, typename LockType = CNullLock, typename DestroyerType = CDefaultArrayDestoryer>
class TShareArray
    : public TEquality2<TShareArray<T, LockType>, T*>
    , public TEquality1<TShareArray<T, LockType> >
{
    typedef TShareArray<T, LockType> Self;
    typedef TNumber<apl_int_t, LockType> CounterType;

public:
    TShareArray(void)
        : mptPointer(NULL)
        , mpoCounter(NULL)
    {
    }
    
    explicit TShareArray( T* aptPtr )
        : mptPointer(aptPtr)
        , mpoCounter(NULL)
    {
        this->InitReference();
    }
    
    TShareArray( Self const& aoPtr )
        : mptPointer(aoPtr.mptPointer)
        , mpoCounter(aoPtr.mpoCounter)
    {
        this->InitReference();
    }

    ~TShareArray(void)
    {
        this->Release();
    }

    Self& operator = ( T* apoPtr )
    {
        TShareArray loTemp(apoPtr);
        
        loTemp.Swap(*this);

        return *this;
    }
    
    Self& operator = ( Self const& aoPtr )
    {
        TShareArray loTemp(aoPtr);
        
        loTemp.Swap(*this);

        return *this;
    }

    void Swap( Self& aoPtr )
    {
        std::swap(this->mptPointer, aoPtr.mptPointer);
        std::swap(this->mpoCounter, aoPtr.mpoCounter);
    }
    
    void Reset(void)
    {
        TShareArray().Swap(*this);
    }
    
    void Reset( T* apoPtr )
    {
        TShareArray(apoPtr).Swap(*this);
    }
    
    apl_size_t GetUseCount(void)
    {
        return *this->mpoCounter;
    }
    
    T* Get(void) const
    {
        return this->mptPointer;
    }
    
    T& operator [] ( apl_size_t auN ) const
    {
        ACL_ASSERT(this->mptPointer != NULL);
        ACL_ASSERT(auN >= 0);
        
        return this->mptPointer[auN];
    }
    
    bool operator ! (void) const
    {
        return this->mptPointer != NULL;
    }

    inline friend bool operator == ( Self const& aoLeft, Self const& aoRight )
    {
        return aoLeft.mptPointer == aoRight.mptPointer;
    }
    
    inline friend bool operator == ( Self const& aoLeft, T const* apoRight )
    {
        return aoLeft.mptPointer == apoRight;
    }
    
protected:
    void InitReference(void)
    {
        if (this->mptPointer != NULL)
        {
            if (this->mpoCounter != NULL)
            {
                ++*this->mpoCounter;
            }
            else
            {
                ACL_NEW_ASSERT(this->mpoCounter, CounterType(1) );
            }
        }
        else
        {
            this->mpoCounter = NULL;
        }
    }
    
    void Release(void)
    {
        if (this->mpoCounter != NULL)
        {
            if (!--(*this->mpoCounter) )
            {
                DestroyerType::Destroy(this->mptPointer);
                ACL_DELETE(this->mpoCounter);
            }
            
            this->mptPointer = NULL;
            this->mpoCounter = NULL;
        }
    }

private:
    T* mptPointer;
    CounterType* mpoCounter;
};

ACL_NAMESPACE_END

#endif //ACL_SMARTPTR_H
