
#include "acl/SockHandle.h"

ACL_NAMESPACE_START

apl_int_t CSockHandle::Open( apl_int_t aiFamily, apl_int_t aiType, apl_int_t aiProtocol )
{
    apl_handle_t liHandle = apl_socket(aiFamily, aiType, aiProtocol);
    
    this->SetHandle(liHandle);
    
    return liHandle == ACL_INVALID_HANDLE ? -1 : 0;
}

apl_int_t CSockHandle::GetLocalAddr( CSockAddr& aoAddr ) const
{
    apl_size_t liLen = aoAddr.GetCapacity();
    
    return apl_getsockname(this->GetHandle(), aoAddr.GetAddr(), &liLen );
}

apl_int_t CSockHandle::GetRemoteAddr( CSockAddr& aoAddr ) const
{
    apl_size_t liLen = aoAddr.GetCapacity();
    
    return apl_getpeername(this->GetHandle(), aoAddr.GetAddr(), &liLen );
}

apl_int_t CSockHandle::GetOption( EIOption aeOpt, apl_int_t* apiOptVal ) const
{   
    apl_int_t liRetCode = 0;
    
    switch(aeOpt)
    {
        case OPT_BROADCAST:
            liRetCode = apl_getsockopt_broadcast(this->GetHandle(), apiOptVal);
            break;
        case OPT_DEBUG:
            liRetCode = apl_getsockopt_debug(this->GetHandle(), apiOptVal);
            break;
        case OPT_DONTROUTE:
            liRetCode = apl_getsockopt_dontroute(this->GetHandle(), apiOptVal);
            break;
        case OPT_KEEPALIVE:
            liRetCode = apl_getsockopt_keepalive(this->GetHandle(), apiOptVal);
            break;
        case OPT_RCVBUF:
            liRetCode = apl_getsockopt_rcvbuf(this->GetHandle(), apiOptVal);
            break;
        case OPT_SNDBUF:
            liRetCode = apl_getsockopt_sndbuf(this->GetHandle(), apiOptVal);
            break;
        case OPT_REUSEADDR:
            liRetCode = apl_getsockopt_reuseaddr(this->GetHandle(), apiOptVal);
            break;
        case OPT_TCPNODELAY:
            liRetCode = apl_getsockopt_tcpnodelay(this->GetHandle(), apiOptVal);
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
            liRetCode = apl_setsockopt_broadcast(this->GetHandle(), aeOptVal);
            break;
        case OPT_DEBUG:
            liRetCode = apl_setsockopt_debug(this->GetHandle(), aeOptVal);
            break;
        case OPT_DONTROUTE:
            liRetCode = apl_setsockopt_dontroute(this->GetHandle(), aeOptVal);
            break;
        case OPT_KEEPALIVE:
            liRetCode = apl_setsockopt_keepalive(this->GetHandle(), aeOptVal);
            break;
        case OPT_RCVBUF:
            liRetCode = apl_setsockopt_rcvbuf(this->GetHandle(), aeOptVal);
            break;
        case OPT_SNDBUF:
            liRetCode = apl_setsockopt_sndbuf(this->GetHandle(), aeOptVal);
            break;
        case OPT_REUSEADDR:
            liRetCode = apl_setsockopt_reuseaddr(this->GetHandle(), aeOptVal);
            break;
        case OPT_TCPNODELAY:
            liRetCode = apl_setsockopt_tcpnodelay(this->GetHandle(), aeOptVal);
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
            liRetCode = apl_getsockopt_error(this->GetHandle(), apiOptVal);
            break;
        case OPT_TYPE:
            liRetCode = apl_getsockopt_type(this->GetHandle(), apiOptVal);
            break;
        case OPT_ACCEPTCONN:
            liRetCode = apl_getsockopt_acceptconn(this->GetHandle(), apiOptVal);
            break;
        default:
            ACL_ASSERT_MSG(false, invalid_get_igooption_option);
    }

    return liRetCode;
}

apl_int_t CSockHandle::GetOption( EOptionLinger aeOpt, apl_int_t* apiOnoff, apl_int_t* apiLonger ) const
{
    apl_int_t liRetCode = 0;
    
    switch(aeOpt)
    {
        case OPT_LINGER:
            liRetCode = apl_getsockopt_linger(this->GetHandle(), apiOnoff, apiLonger);
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
            liRetCode = apl_setsockopt_linger(this->GetHandle(), apiOnoff, apiLonger);
            break;
        default:
            ACL_ASSERT_MSG(false, invalid_set_lingeroption_option);
    }

    return liRetCode;
}

//////////////////////////////////////////// SockHandlePair ///////////////////////////////////
apl_int_t CSockHandlePair::Open( apl_int_t aiFamily, apl_int_t aiType, apl_int_t aiProtocol )
{
    apl_handle_t liFirst = ACL_INVALID_HANDLE;
    apl_handle_t liSecond = ACL_INVALID_HANDLE;
    
    if ( apl_socketpair( aiFamily, aiType, aiProtocol, &liFirst, &liSecond ) != 0 )
    {
        return -1;
    }
    
    this->maoHandle[0].SetHandle(liFirst);
    this->maoHandle[1].SetHandle(liSecond);
    
    return 0;
}
    
CSockHandle& CSockHandlePair::GetFirst(void)
{
    return this->maoHandle[0];
}
    
CSockHandle& CSockHandlePair::GetSecond(void)
{
    return this->maoHandle[1];
}

ACL_NAMESPACE_END
