
#ifndef ACL_POLL_EVENT_H
#define ACL_POLL_EVENT_H

#include "acl/Utility.h"
#include "acl/TimeValue.h"

ACL_NAMESPACE_START

class CPollee;

/////////////////////////////////////// poll iterator ////////////////////////////////////
class CPolleeIterator
{
public:
    CPolleeIterator(void);
    
    CPolleeIterator( CPollee* apoPoll, apl_size_t aiPos );
    
    CPolleeIterator& operator ++ (void);
    
    CPolleeIterator operator ++ (int);
    
    bool operator == ( CPolleeIterator const& aoRhs );
    
    bool operator != ( CPolleeIterator const& aoRhs );
    
    apl_handle_t Handle(void);
    
    apl_int_t& Event(void);
    
private:
    CPollee* mpoPollee;
    
    apl_size_t muPos;
    
    apl_int_t  miRevents;
};

/////////////////////////////////////////////// poll ////////////////////////////////////////
class CPollee
{
public:
    typedef CPolleeIterator IteratorType;
    
public:
    CPollee(void);
    
    ~CPollee(void);
    
    apl_int_t Initialize( apl_size_t auSize );
    
    apl_int_t Close(void);

    apl_int_t AddHandle( apl_handle_t aiHandle, apl_int_t aiEvent );
    
    apl_int_t DelHandle( apl_handle_t aiHandle );
    
    apl_ssize_t Poll( CTimeValue const& aoTimeout = CTimeValue::MAXTIME );
    
    apl_handle_t GetHandle( apl_size_t aiN );
    
    apl_int_t GetEvent( apl_size_t aiN );
    
    IteratorType Begin(void);
    
    IteratorType End(void);
    
    apl_size_t GetCapacity(void);
    
    apl_size_t GetResultSize(void);
    
    static apl_ssize_t Poll(
        apl_handle_t aiHandle,
        apl_int_t aiEvent,
        apl_int_t* apiREvent,
        CTimeValue const& aoTimeout = CTimeValue::MAXTIME );

private:
    apl_pollset_t moPollSet;
    
    apl_pollfd_t* mpoPollFds;
    
    apl_size_t muCapacity;

    apl_size_t muResultSize;
    
    bool mbIsInited;
};

//Change class name CPollee to CPoll
typedef CPollee CPoll;

//////////////////////////////////////// inline implement /////////////////////////////////////////////
inline CPolleeIterator::CPolleeIterator(void)
    : mpoPollee(NULL)
    , muPos(0)
    , miRevents(0)
{
}

inline CPolleeIterator::CPolleeIterator( CPollee* apoPoll, apl_size_t aiPos )
    : mpoPollee(apoPoll)
    , muPos(aiPos)
    , miRevents(0)
{
    if ( this->mpoPollee != NULL )
    {
        this->miRevents = this->mpoPollee->GetEvent(this->muPos);
    }
}

inline CPolleeIterator& CPolleeIterator::operator ++ (void)
{
    this->muPos++;
    
    if ( this->mpoPollee != NULL )
    {
        this->miRevents = this->mpoPollee->GetEvent(this->muPos);
    }
    
    return *this;
}

inline CPolleeIterator CPolleeIterator::operator ++ (int)
{
    CPolleeIterator loTmp(*this);
    
    ++(*this);
    
    return loTmp;
}

inline bool CPolleeIterator::operator == ( CPolleeIterator const& aoRhs )
{
    return this->mpoPollee == aoRhs.mpoPollee && this->muPos == aoRhs.muPos;
}

inline bool CPolleeIterator::operator != ( CPolleeIterator const& aoRhs )
{
    return !(*this == aoRhs);
}

inline apl_handle_t CPolleeIterator::Handle(void)
{
    return this->mpoPollee != NULL ? this->mpoPollee->GetHandle(this->muPos) : -1;
}

inline apl_int_t& CPolleeIterator::Event(void)
{
    return this->miRevents;
}
    
ACL_NAMESPACE_END

#endif//ACL_POLL_EVENT_H
