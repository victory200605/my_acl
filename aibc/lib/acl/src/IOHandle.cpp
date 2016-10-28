
#include "acl/IOHandle.h"

ACL_NAMESPACE_START

CIOHandle::CIOHandle(void)
    : miHandle(ACL_INVALID_HANDLE)
{
}

void CIOHandle::SetHandle( apl_handle_t aiHandle )
{
    this->miHandle = aiHandle;
    this->Enable(OPT_CLOEXEC);
}

apl_handle_t CIOHandle::GetHandle(void) const
{
    return this->miHandle;
}

apl_int_t CIOHandle::Disable( ESWOption aeOpt ) const
{
    apl_int_t (*set_control)( apl_int_t, apl_int_t);
    apl_int_t (*get_control)( apl_int_t, apl_int_t*);
    apl_int_t liFlag   = 0;
    apl_int_t liOptVal = 0;
    
    switch(aeOpt)
    {
        case OPT_NONBLOCK:
        {
            set_control = apl_fcntl_setfl;
            get_control = apl_fcntl_getfl;
            liOptVal    = APL_O_NONBLOCK;
            break;
        }
        case OPT_CLOEXEC:
        {
            set_control = apl_fcntl_setfd;
            get_control = apl_fcntl_getfd;
            liOptVal    = APL_FD_CLOEXEC;
            break;
        }
        default:
        {
            ACL_ASSERT_MSG(false, invalid_disable_option);
        }
    }

    if ( get_control(this->miHandle, &liFlag) != 0 )
    {
        return -1;
    }
    
    if (liFlag & liOptVal)
    {
        liFlag ^= liOptVal;
        if ( set_control(this->miHandle, liFlag) != 0 )
        {
            return -1;
        }
    }
    
    return 0;
}

apl_int_t CIOHandle::Enable( ESWOption aeOpt ) const
{
    apl_int_t (*set_control)( apl_int_t, apl_int_t);
    apl_int_t (*get_control)( apl_int_t, apl_int_t*);
    apl_int_t liFlag   = 0;
    apl_int_t liOptVal = 0;
    
    switch(aeOpt)
    {
        case OPT_NONBLOCK:
        {
            set_control = apl_fcntl_setfl;
            get_control = apl_fcntl_getfl;
            liOptVal    = APL_O_NONBLOCK;
            break;
        }
        case OPT_CLOEXEC:
        {
            set_control = apl_fcntl_setfd;
            get_control = apl_fcntl_getfd;
            liOptVal    = APL_FD_CLOEXEC;
            break;
        }
        default:
        {
            ACL_ASSERT_MSG(false, invalid_enable_option);
        }
    }

    if ( get_control(this->miHandle, &liFlag) != 0 )
    {
        return -1;
    }
    
    liFlag |= liOptVal;
    
    if ( set_control(this->miHandle, liFlag) != 0 )
    {
        return -1;
    }
    
    return 0;
}

void CIOHandle::Close(void)
{
    if (this->miHandle != ACL_INVALID_HANDLE)
    {
        apl_close(this->miHandle);
        this->miHandle = ACL_INVALID_HANDLE;
    }
}

ACL_NAMESPACE_END
