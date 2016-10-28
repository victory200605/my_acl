
#include "SockHandle.h"

ACL_NAMESPACE_START

CSockHandle::CSockHandle(void)
    : ciHandle(ACL_INVALID_HANDLE)
{
}

void CSockHandle::SetHandle( apl_handle_t aiHandle )
{
    this->ciHandle = aiHandle;
}

apl_handle_t CSockHandle::GetHandle(void) const
{
    return this->ciHandle;
}

apl_int_t CSockHandle::Open( apl_int_t aiFamily, apl_int_t aiType, apl_int_t aiProtocol )
{
    this->ciHandle = apl_socket(aiFamily, aiType, aiProtocol);
    
    return this->ciHandle == ACL_INVALID_HANDLE ? -1 : 0;
}

void CSockHandle::Close(void)
{
    apl_close(this->ciHandle);
    
    this->ciHandle = ACL_INVALID_HANDLE;
}

apl_int_t CSockHandle::Disable( ESWOption aeOpt ) const
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

    if ( get_control(this->ciHandle, &liFlag) != 0 )
    {
        return -1;
    }
    
    liFlag ^= liOptVal;
    
    if ( set_control(this->ciHandle, liFlag) != 0 )
    {
        return -1;
    }
    
    return 0;
}

apl_int_t CSockHandle::Enable( ESWOption aeOpt ) const
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

    if ( get_control(this->ciHandle, &liFlag) != 0 )
    {
        return -1;
    }
    
    liFlag |= liOptVal;
    
    if ( set_control(this->ciHandle, liFlag) != 0 )
    {
        return -1;
    }
    
    return 0;
}

apl_int_t CSockHandle::GetLocalAddr( CSockAddr& aoAddr ) const
{
    apl_size_t liLen = aoAddr.GetLength();
    
    return apl_getsockname(this->ciHandle, aoAddr.GetAddr(), &liLen );
}

apl_int_t CSockHandle::GetRemoteAddr( CSockAddr& aoAddr ) const
{
    apl_size_t liLen = aoAddr.GetLength();
    
    return apl_getpeername(this->ciHandle, aoAddr.GetAddr(), &liLen );
}

apl_int_t CSockHandle::GetOption( EIOption aeOpt, apl_int_t* apiOptVal ) const
{   
    apl_int_t liRetCode = 0;
    
    switch(aeOpt)
    {
        case OPT_BROADCAST:
            liRetCode = apl_getsockopt_broadcast(this->ciHandle, apiOptVal);
            break;
        case OPT_DEBUG:
            liRetCode = apl_getsockopt_debug(this->ciHandle, apiOptVal);
            break;
        case OPT_DONTROUTE:
            liRetCode = apl_getsockopt_dontroute(this->ciHandle, apiOptVal);
            break;
        case OPT_KEEPALIVE:
            liRetCode = apl_getsockopt_keepalive(this->ciHandle, apiOptVal);
            break;
        case OPT_OOBINLINE:
            liRetCode = apl_getsockopt_oobinline(this->ciHandle, apiOptVal);
            break;
        case OPT_RCVBUF:
            liRetCode = apl_getsockopt_rcvbuf(this->ciHandle, apiOptVal);
            break;
        case OPT_SNDBUF:
            liRetCode = apl_getsockopt_sndbuf(this->ciHandle, apiOptVal);
            break;
        case OPT_RCVLOWAT:
            liRetCode = apl_getsockopt_rcvlowat(this->ciHandle, apiOptVal);
            break;
        case OPT_SNDLOWAT:
            liRetCode = apl_getsockopt_sndlowat(this->ciHandle, apiOptVal);
            break;
        case OPT_REUSEADDR:
            liRetCode = apl_getsockopt_reuseaddr(this->ciHandle, apiOptVal);
            break;
        case OPT_TCPNODELAY:
            liRetCode = apl_getsockopt_tcpnodelay(this->ciHandle, apiOptVal);
            break;
        default:
            ACL_ASSERT_MSG(false, invalid_get_ioption_option);
    }

    return liRetCode;
}

apl_int_t CSockHandle::SetOption( EIOption aeOpt, apl_int_t aeOptVal ) const
{
    apl_int_t liRetCode = 0;
    
    switch(aeOpt)
    {
        case OPT_BROADCAST:
            liRetCode = apl_setsockopt_broadcast(this->ciHandle, aeOptVal);
            break;
        case OPT_DEBUG:
            liRetCode = apl_setsockopt_debug(this->ciHandle, aeOptVal);
            break;
        case OPT_DONTROUTE:
            liRetCode = apl_setsockopt_dontroute(this->ciHandle, aeOptVal);
            break;
        case OPT_KEEPALIVE:
            liRetCode = apl_setsockopt_keepalive(this->ciHandle, aeOptVal);
            break;
        case OPT_OOBINLINE:
            liRetCode = apl_setsockopt_oobinline(this->ciHandle, aeOptVal);
            break;
        case OPT_RCVBUF:
            liRetCode = apl_setsockopt_rcvbuf(this->ciHandle, aeOptVal);
            break;
        case OPT_SNDBUF:
            liRetCode = apl_setsockopt_sndbuf(this->ciHandle, aeOptVal);
            break;
        case OPT_RCVLOWAT:
            liRetCode = apl_setsockopt_rcvlowat(this->ciHandle, aeOptVal);
            break;
        case OPT_SNDLOWAT:
            liRetCode = apl_setsockopt_sndlowat(this->ciHandle, aeOptVal);
            break;
        case OPT_REUSEADDR:
            liRetCode = apl_setsockopt_reuseaddr(this->ciHandle, aeOptVal);
            break;
        case OPT_TCPNODELAY:
            liRetCode = apl_setsockopt_tcpnodelay(this->ciHandle, aeOptVal);
            break;
        default:
            ACL_ASSERT_MSG(false, invalid_set_ioption_option);
    }

    return liRetCode;
}

apl_int_t CSockHandle::GetOption( EIGOOption aeOpt, apl_int_t* apiOptVal ) const
{
    apl_int_t liRetCode = 0;
    
    switch(aeOpt)
    {
        case OPT_ERROR:
            liRetCode = apl_getsockopt_error(this->ciHandle, apiOptVal);
            break;
        case OPT_TYPE:
            liRetCode = apl_getsockopt_type(this->ciHandle, apiOptVal);
            break;
        case OPT_ACCEPTCONN:
            liRetCode = apl_getsockopt_acceptconn(this->ciHandle, apiOptVal);
            break;
        default:
            ACL_ASSERT_MSG(false, invalid_get_igooption_option);
    }

    return liRetCode;
}

apl_int_t CSockHandle::GetOption( EI64Option aeOpt, apl_int64_t* apiOptVal ) const
{
    apl_int_t liRetCode = 0;
    
    switch(aeOpt)
    {
        case OPT_RCVTIMEO:
            liRetCode = apl_getsockopt_rcvtimeo(this->ciHandle, apiOptVal);
            break;
        case OPT_SNDTIMEO:
            liRetCode = apl_getsockopt_sndtimeo(this->ciHandle, apiOptVal);
            break;
        default:
            ACL_ASSERT_MSG(false, invalid_get_i64option_option);
    }

    return liRetCode;
}

apl_int_t CSockHandle::SetOption( EI64Option aeOpt, apl_int64_t aeOptVal ) const
{
    apl_int_t liRetCode = 0;
    
    switch(aeOpt)
    {
        case OPT_RCVTIMEO:
            liRetCode = apl_setsockopt_rcvtimeo(this->ciHandle, aeOptVal);
            break;
        case OPT_SNDTIMEO:
            liRetCode = apl_setsockopt_sndtimeo(this->ciHandle, aeOptVal);
            break;
        default:
            ACL_ASSERT_MSG(false, invalid_set_i64option_option);
    }

    return liRetCode;
}

apl_int_t CSockHandle::GetOption( EOptionLinger aeOpt, apl_int_t* apiOnoff, apl_int_t* apiLonger ) const
{
    apl_int_t liRetCode = 0;
    
    switch(aeOpt)
    {
        case OPT_LINGER:
            liRetCode = apl_getsockopt_linger(this->ciHandle, apiOnoff, apiLonger);
            break;
        default:
            ACL_ASSERT_MSG(false, invalid_get_lingeroption_option);
    }

    return liRetCode;
}

apl_int_t CSockHandle::SetOption( EOptionLinger aeOpt, apl_int_t apiOnoff, apl_int_t apiLonger ) const
{
    apl_int_t liRetCode = 0;
    
    switch(aeOpt)
    {
        case OPT_LINGER:
            liRetCode = apl_setsockopt_linger(this->ciHandle, apiOnoff, apiLonger);
            break;
        default:
            ACL_ASSERT_MSG(false, invalid_set_lingeroption_option);
    }

    return liRetCode;
}

ACL_NAMESPACE_END
