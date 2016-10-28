/**
 * @file ScopeArray.h
 */
#ifndef ACL_SCOPEARRAY_H
#define ACL_SCOPEARRAY_H

#include "acl/Utility.h"
#include "acl/Operators.h"
#include "acl/stl/algorithm.h"

ACL_NAMESPACE_START

template<typename T>
class TScopeArray
    : public TEquality2<TScopeArray<T>, T*>
    , public TEquality1<TScopeArray<T> >
{
    typedef TScopeArray<T> Self;
    
public:
    TScopeArray(void)
        : mptPointer(NULL)
    {
    }
    
    explicit TScopeArray( T* aptPtr )
        : mptPointer(aptPtr)
    {
    }
    
    ~TScopeArray(void)
    {
        ACL_DELETE_N(this->mptPointer);
    }
    
    void Reset(void)
    {
        Self().Swap(*this);
    }
    
    void Reset( T* aptPtr )
    {
        Self(aptPtr).Swap(*this);
    }

    T& operator [] ( apl_size_t auN ) const
    {
        ACL_ASSERT(this->mptPointer != NULL);
        ACL_ASSERT(auN >= 0);
        
        return this->mptPointer[auN];
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
    
    inline friend bool operator == ( Self const& aoLeft, Self const& aoRight )
    {
        return aoLeft.mptPointer == aoRight.mptPointer;
    }
    
    inline friend bool operator == ( Self const& aoLeft, T const* apoRight )
    {
        return aoLeft.mptPointer == apoRight;
    }

private:
    TScopeArray( Self const& );
    TScopeArray& operator=( Self const& );
    
private:
    T* mptPointer;
};

ACL_NAMESPACE_END

#endif//ACL_SCOPEARRAY_H
