/**
 * @file ScopePtr.h
 */
#ifndef ACL_SCOPEPTR_H
#define ACL_SCOPEPTR_H

#include "acl/Utility.h"
#include "acl/Operators.h"
#include "acl/stl/algorithm.h"

ACL_NAMESPACE_START

template<typename T>
class TScopePtr
    : public TLessThanAll2<TScopePtr<T>, T*>
    , public TLessThanAll1<TScopePtr<T> >
{
    typedef TScopePtr<T> Self;
    
public:
    TScopePtr(void)
        : mptPointer(NULL)
    {
    }
    
    explicit TScopePtr( T* aptPtr )
        : mptPointer(aptPtr)
    {
    }
    
    ~TScopePtr(void)
    {
        ACL_DELETE(this->mptPointer);
    }
    
    void Reset(void)
    {
        Self().Swap(*this);
    }
    
    void Reset( T* aptPtr )
    {
        Self(aptPtr).Swap(*this);
    }

    T& operator * () const
    {
        return *this->mptPointer;
    }

    T* operator -> () const
    {
        return this->mptPointer;
    }

    T* Get() const
    {
        return this->mptPointer;
    }

    bool operator ! (void) const
    {
        return this->mptPointer != NULL;
    }

    void Swap( Self& aoPtr )
    {
        std::swap(aoPtr.mptPointer, this->mptPointer);
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

private:
    TScopePtr( Self const& );
    TScopePtr& operator=( Self const& );
    
private:
    T* mptPointer;
};

ACL_NAMESPACE_END

#endif//ACL_SCOPEPTR_H
