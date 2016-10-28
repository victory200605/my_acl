
#include "acl/Poll.h"

ACL_NAMESPACE_START

CPollee::CPollee(void)
    : mpoPollFds(NULL)
    , muCapacity(0)
    , muResultSize(0)
    , mbIsInited(false)
{
}
    
CPollee::~CPollee(void)
{
    this->Close();
}
    
apl_int_t CPollee::Initialize( apl_size_t auSize )
{
    if (this->mpoPollFds != NULL)
    {
        apl_set_errno(APL_EBUSY);
        return -1;
    }
    
    if ( apl_pollset_init(&this->moPollSet, auSize) != 0 )
    {
        return -1;
    }
    
    ACL_NEW_N_ASSERT(this->mpoPollFds, apl_pollfd_t, auSize);
    apl_memset(this->mpoPollFds, 0, sizeof(apl_pollfd_t) * auSize );
    
    this->muCapacity = auSize;
    
    this->mbIsInited = true;
    
    return 0;
}

apl_int_t CPollee::Close(void)
{
    apl_int_t liRetCode = 0;
    
    if (mbIsInited)
    {
        ACL_DELETE_N(this->mpoPollFds);
    
        this->muCapacity = 0;
        
        this->mbIsInited = false;
    
        liRetCode = apl_pollset_destroy(&this->moPollSet);
    }
    
    return liRetCode;
}

apl_int_t CPollee::AddHandle( apl_handle_t aiHandle, apl_int_t aiEvent )
{
    apl_int_t liRetCode = 0;
    
    if (!this->mbIsInited)
    {
        apl_set_errno(APL_EBADF);
        return -1;
    }
    
    ACL_RETRY_CHK(
        (liRetCode = apl_pollset_add_handle(&this->moPollSet, aiHandle, aiEvent) ) == 0, 5 );
    
    return liRetCode;
}

apl_int_t CPollee::DelHandle( apl_handle_t aiHandle )
{
    apl_int_t liRetCode = 0;
    
    if (!this->mbIsInited)
    {
        apl_set_errno(APL_EBADF);
        return -1;
    }
    
    ACL_RETRY_CHK(
        (liRetCode = apl_pollset_del_handle(&this->moPollSet, aiHandle) ) == 0, 5 );
    
    return liRetCode;
}

apl_ssize_t CPollee::Poll( CTimeValue const& aoTimeout )
{
    apl_ssize_t liResult = 0;
    apl_time_t  li64Timeout = (aoTimeout == CTimeValue::MAXTIME ? -1 : aoTimeout.Nsec() );
    
    this->muResultSize = 0;
    
    if (!this->mbIsInited)
    {
        apl_set_errno(APL_EBADF);
        return -1;
    }
    
    ACL_RETRY_CHK(
        (liResult = apl_poll(&this->moPollSet, this->mpoPollFds, this->muCapacity, li64Timeout) ) < 0, -1);
    
    this->muResultSize = (liResult >= 0 ? liResult : 0);
    
    return liResult;
}

apl_handle_t CPollee::GetHandle( apl_size_t aiN )
{
    return (aiN >= this->muCapacity ? -1 : APL_POLLFD_GET_HANDLE(&(this->mpoPollFds[aiN])));
}

apl_int_t CPollee::GetEvent( apl_size_t aiN )
{
    return (aiN >= this->muCapacity ? 0 : APL_POLLFD_GET_EVENTS(&(this->mpoPollFds[aiN])));
}

CPollee::IteratorType CPollee::Begin(void)
{
    return IteratorType(this, 0);
}

CPollee::IteratorType CPollee::End(void)
{
    return IteratorType(this, this->muResultSize);
}

apl_size_t CPollee::GetCapacity(void)
{
    return this->muCapacity;
}

apl_size_t CPollee::GetResultSize(void)
{
    return this->muResultSize;
}

apl_ssize_t CPollee::Poll(
    apl_handle_t aiHandle,
    apl_int_t aiEvent,
    apl_int_t* apiREvent,
    CTimeValue const& aoTimeout )
{
    apl_ssize_t liResult = 0;
    apl_time_t  li64Timeout = (aoTimeout == CTimeValue::MAXTIME ? -1 : aoTimeout.Nsec() );
    
    ACL_RETRY_CHK(
        (liResult = apl_poll_one(aiHandle, aiEvent, apiREvent, li64Timeout) ) < 0, -1);

    return liResult;
}

ACL_NAMESPACE_END
