/**
 * @file SharePtr.h
 */
#ifndef ACL_SMARTPTR_SHAREPTR_H
#define ACL_SMARTPTR_SHAREPTR_H

#include "acl/Utility.h"
#include "acl/Synch.h"
#include "acl/Number.h"
#include "acl/Operators.h"
#include "acl/stl/algorithm.h"

ACL_NAMESPACE_START

///Type cast tag
struct CStaticCast{};
struct CConstCast{};
struct CDynamicCast{};
struct CReinterpretCast{};

static const CStaticCast      STATIC_CAST_TAG      = CStaticCast();
static const CConstCast       CONST_CAST_TAG       = CConstCast();
static const CDynamicCast     DYNAMIC_CAST_TAG     = CDynamicCast();
static const CReinterpretCast REINTERPRET_CAST_TAG = CReinterpretCast();
///

struct CDefaultDestroyer
{
    template<typename T> static void Destroy( T* p )
    {
        ACL_DELETE(p);
    }
};

template<typename T, typename LockType = CNullLock, typename DestroyerType = CDefaultDestroyer>
class TSharePtr
    : public TLessThanAll2<TSharePtr<T, LockType>, T*>
    , public TLessThanAll1<TSharePtr<T, LockType> >
{
    typedef TSharePtr<T, LockType> Self;
    typedef TNumber<apl_int_t, LockType> CounterType;

public:
    TSharePtr(void)
        : mptPointer(NULL)
        , mpoCounter(NULL)
    {
    }
    
    template<typename Y>
    explicit TSharePtr( Y* aptPtr )
        : mptPointer(aptPtr)
        , mpoCounter(NULL)
    {
        this->InitReference();
    }
    
    TSharePtr( Self const& aoPtr )
        : mptPointer(aoPtr.mptPointer)
        , mpoCounter(aoPtr.mpoCounter)
    {
        this->InitReference();
    }
    
    template<typename Y>
    TSharePtr( TSharePtr<Y, LockType, DestroyerType> const& aoPtr )
        : mptPointer(aoPtr.mptPointer)
        , mpoCounter(aoPtr.mpoCounter)
    {
        this->InitReference();
    }
    
    template<typename Y>
    TSharePtr( TSharePtr<Y, LockType, DestroyerType> const& aoPtr, CStaticCast const& )
        : mptPointer(static_cast<T*>(aoPtr.mptPointer) )
        , mpoCounter(aoPtr.mpoCounter)
    {
        this->InitReference();
    }
    
    template<typename Y>
    TSharePtr( TSharePtr<Y, LockType, DestroyerType> const& aoPtr, CDynamicCast const& )
        : mptPointer(dynamic_cast<T*>(aoPtr.mptPointer) )
        , mpoCounter(aoPtr.mpoCounter)
    {
        this->InitReference();
    }
    
    template<typename Y>
    TSharePtr( TSharePtr<Y, LockType, DestroyerType> const& aoPtr, CConstCast const& )
        : mptPointer(const_cast<T*>(aoPtr.mptPointer) )
        , mpoCounter(aoPtr.mpoCounter)
    {
        this->InitReference();
    }
    
    template<typename Y>
    TSharePtr( TSharePtr<Y, LockType, DestroyerType> const& aoPtr, CReinterpretCast const& )
        : mptPointer(reinterpret_cast<T*>(aoPtr.mptPointer) )
        , mpoCounter(aoPtr.mpoCounter)
    {
        this->InitReference();
    }
    
    ~TSharePtr(void)
    {
        this->Release();
    }
    
    template<typename Y>
    Self& operator = ( Y* apoPtr )
    {
        Self loTemp(apoPtr);
        
        loTemp.Swap(*this);

        return *this;
    }
    
    Self& operator = ( Self const& aoPtr )
    {
        Self loTemp(aoPtr);
        
        loTemp.Swap(*this);

        return *this;
    }

    template<typename Y>
    Self& operator = ( TSharePtr<Y, LockType, DestroyerType> const& aoPtr )
    {
        Self loTemp(aoPtr);
        
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
        Self().Swap(*this);
    }
    
    template<typename Y>
    void Reset( Y* apoPtr )
    {
        Self(apoPtr).Swap(*this);
    }
    
    apl_size_t GetUseCount(void)
    {
        return *this->mpoCounter;
    }
    
    T* Get(void) const
    {
        return this->mptPointer;
    }
    
    bool operator ! (void)
    {
        return this->mptPointer != NULL;
    }

    T* operator -> (void) const
    {
        return this->mptPointer;
    }

    T& operator * (void) const
    {
        return *this->mptPointer;
    }
    
    inline friend bool operator < ( Self const& aoLeft, Self const& aoRight )
    {
        return aoLeft.mptPointer < aoRight.mptPointer;
    }
    
    inline friend bool operator < ( Self const& aoLeft, T const* apoRight )
    {
        return aoLeft.mptPointer < apoRight;
    }
    
    inline friend bool operator > ( Self const& aoLeft, T const* apoRight )
    {
        return aoLeft.mptPointer > apoRight;
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

public:
    
// Tasteless as this may seem, making all members public allows member templates
// to work in the absence of member template friends.
#ifndef ACL_NO_MEMBER_TEMPLATE_FRIENDS
private:
    template<typename Y, typename L, typename K> friend class TSharePtr;

#endif

    T* mptPointer;
    CounterType* mpoCounter;
};

ACL_NAMESPACE_END

#endif //ACL_SMARTPTR_H
