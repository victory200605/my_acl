
#include "acl/SockDgram.h"
#include "apl/poll.h"

ACL_NAMESPACE_START

apl_int_t CSockDgram::Open( apl_int_t aiFamily, apl_int_t aiProtocol )
{
    //Open acceptor socket handle
    if ( CSockHandle::Open(aiFamily, APL_SOCK_DGRAM, aiProtocol) != 0 )
    {
        return -1;
    }
    
    this->SetOption(CSockHandle::OPT_REUSEADDR, 1);
        
    return 0;
}
        
apl_int_t CSockDgram::Open( CSockAddr const& aoLocal, apl_int_t aiFamily, apl_int_t aiProtocol )
{
    //Open acceptor socket handle
    if ( CSockHandle::Open(aiFamily, APL_SOCK_DGRAM, aiProtocol) != 0 )
    {
        return -1;
    }
    
    this->SetOption(CSockHandle::OPT_REUSEADDR, 1);

    //Bind acceptor addr
    if ( apl_bind(this->GetHandle(), aoLocal.GetAddr(), aoLocal.GetLength() ) != 0 )
    {
        this->Close();
        return -1;
    }
    
    return 0;
}

apl_ssize_t CSockDgram::Recv( 
    void* apvBuffer, 
    apl_size_t aiN,
    CSockAddr* apoAddr,
    EOption aeFlag ) const
{
    apl_ssize_t  liResult   = 0;
    apl_size_t   liSockLen  = (apoAddr == NULL ? 0 : apoAddr->GetCapacity() );
    apl_sockaddr_t* lpoAddr = (apoAddr == NULL ? NULL : apoAddr->GetAddr() );

    //If liRetCode = EINTR or EAGAIN .. and retry it
    ACL_RETRY_CHK(
        (liResult = apl_recvfrom(this->GetHandle(), apvBuffer, aiN, aeFlag, lpoAddr, &liSockLen) ) < 0, 5 );
    if (liResult < 0)
    {
        return -1;
    }
    else if (liResult == 0)
    {
        //Connection closed by peer
        apl_set_errno(APL_ECONNRESET);
        return -1;
    }
    
    return liResult;
}

apl_ssize_t CSockDgram::Recv(
    void* apvBuffer, 
    apl_size_t aiN,
    CSockAddr* apoAddr ) const
{
    return this->Recv( apvBuffer, aiN, apoAddr, OPT_UNSPEC );
}
                
apl_ssize_t CSockDgram::Recv( 
    void* apvBuffer, 
    apl_size_t aiN,
    CSockAddr* apoAddr,
    EOption aeFlag,
    CTimeValue const& aoTimeout ) const
{
    apl_ssize_t liResult    = 0;
    apl_size_t  liSockLen   = (apoAddr == NULL ? 0 : apoAddr->GetCapacity() );
    apl_sockaddr_t* lpoAddr = (apoAddr == NULL ? NULL : apoAddr->GetAddr() );
    apl_time_t li64Timeout  = (aoTimeout == CTimeValue::MAXTIME ? -1 : aoTimeout.Nsec() );
    
    //If liRetCode = EINTR or EAGAIN .. and retry it
    ACL_RETRY_CHK(
        (liResult = apl_poll_one(this->GetHandle(), APL_POLLIN, NULL, li64Timeout) ) < 0, 5 );
    if (liResult < 0)
    {
        return -1;
    }
    else if (liResult == 0)
    {
        //Wait event timeout
        apl_set_errno(APL_EAGAIN);
        return 0;
    }
    
    //If liRetCode = EINTR or EAGAIN .. and retry it
    ACL_RETRY_CHK(
        (liResult = apl_recvfrom(this->GetHandle(), apvBuffer, aiN, aeFlag, lpoAddr, &liSockLen) ) < 0, 5 );
    if (liResult < 0)
    {
        return -1;
    }
    else if (liResult == 0)
    {
        //Connection closed by peer
        apl_set_errno(APL_ECONNRESET);
        return -1;
    }
    
    return liResult;
}

apl_ssize_t CSockDgram::Recv(
    void* apvBuffer, 
    apl_size_t aiN,
    CSockAddr* apoAddr,
    CTimeValue const& aoTimeout ) const
{
    return this->Recv( apvBuffer, aiN, apoAddr, OPT_UNSPEC, aoTimeout );
}

apl_ssize_t CSockDgram::RecvUntil(
    void* apvBuffer, 
    apl_size_t aiN,
    CSockAddr* apoAddr,
    char const* apcTag,
    EOption aeFlag,
    CTimeValue const& aoTimeout ) const
{
    apl_ssize_t liResult      = 0;
    char*       lpcUntil      = NULL;
    apl_size_t  liNeedSize    = aiN;
    char*       lpcCurrBuffer = (char*)apvBuffer;
    apl_size_t  liTagLen      = apl_strlen(apcTag);
    apl_sockaddr_t* lpoAddr  = (apoAddr == NULL ? NULL : apoAddr->GetAddr() );
    apl_size_t  liSockLen     = (apoAddr == NULL ? 0 : apoAddr->GetCapacity() );
    apl_time_t  li64Timeout   = aoTimeout == CTimeValue::MAXTIME ? -1 : aoTimeout.Nsec();
    
    //If liRetCode = EINTR or EAGAIN .. and retry it
    ACL_RETRY_CHK(
        (liResult = apl_poll_one(this->GetHandle(), APL_POLLIN, NULL, li64Timeout) ) < 0, 5 );
    if (liResult < 0)
    {
        return -1;
    }
    else if (liResult == 0)
    {
        //Wait event timeout
        apl_set_errno(APL_EAGAIN);
        return 0;
    }
    
    //If liRetCode = EINTR or EAGAIN .. and retry it
    ACL_RETRY_CHK(
        (liResult = apl_recvfrom(this->GetHandle(), apvBuffer, liNeedSize, OPT_PEEK, lpoAddr, &liSockLen) ) < 0, 5 );
    if (liResult < 0)
    {
        return -1;
    }
    else if (liResult == 0)
    {
        //Connection closed by peer
        apl_set_errno(APL_ECONNRESET);
        return -1;
    }
    
    //Set end character for strstr
    lpcCurrBuffer[ (apl_size_t)liResult < liNeedSize ? liResult : liNeedSize - 1 ] = '\0';
    
    if ( aeFlag == OPT_PEEK )
    {
        //Search tag token in curr buffer
        if ( (lpcUntil = apl_strstr( lpcCurrBuffer, apcTag ) ) != NULL )
        {
            //Hit, complete
            return lpcUntil - lpcCurrBuffer + liTagLen;
        }
        
        return liResult;
    }
    else
    {
        //Search tag token in curr buffer
        if ( (lpcUntil = apl_strstr(
            lpcCurrBuffer - liTagLen >= (char*)apvBuffer? lpcCurrBuffer - liTagLen : (char*)apvBuffer, apcTag ) ) != NULL )
        {
            //Hit, offset liNeedSize for recv
            liNeedSize = lpcUntil - lpcCurrBuffer + liTagLen;
        }
        
        //If liRetCode = EINTR or EAGAIN .. and retry it
        ACL_RETRY_CHK(
            (liResult = apl_recvfrom(this->GetHandle(), apvBuffer, liNeedSize, aeFlag, lpoAddr, &liSockLen) ) < 0, 5);
        if (liResult < 0)
        {
            return -1;
        }
        
        return liResult;
    }
}

apl_ssize_t CSockDgram::RecvUntil(
    void* apvBuffer, 
    apl_size_t aiN,
    CSockAddr* apoAddr,
    char const* apcTag, 
    CTimeValue const& aoTimeout ) const
{
    return this->RecvUntil( apvBuffer, aiN, apoAddr, apcTag, OPT_UNSPEC, aoTimeout );
}

apl_ssize_t CSockDgram::Send(
    void const* apvBuffer, 
    apl_size_t aiN,
    CSockAddr const& aoAddr,
    EOption aeFlag ) const
{
    apl_ssize_t liResult = 0;

    //If liRetCode = EINTR or EAGAIN .. and retry it
    ACL_RETRY_CHK(
        (liResult = apl_sendto(
            this->GetHandle(), apvBuffer, aiN, aeFlag, aoAddr.GetAddr(), aoAddr.GetLength() ) ) < 0, 5 );
    if (liResult < 0)
    {
        return -1;
    }
    
    return liResult;
}

apl_ssize_t CSockDgram::Send(
    void const* apvBuffer, 
    apl_size_t aiN,
    CSockAddr const& aoAddr ) const
{
    return this->Send( apvBuffer, aiN, aoAddr, OPT_UNSPEC );
}

apl_ssize_t CSockDgram::Send(
    void const* apvBuffer, 
    apl_size_t aiN,
    CSockAddr const& aoAddr,
    EOption aeFlag,
    CTimeValue const& aoTimeout ) const
{
    apl_ssize_t liResult    = 0;
    apl_time_t  li64Timeout = (aoTimeout == CTimeValue::MAXTIME ? -1 : aoTimeout.Nsec() );

    //If liRetCode = EINTR or EAGAIN .. and retry it
    ACL_RETRY_CHK(
        (liResult = apl_poll_one(this->GetHandle(), APL_POLLOUT, NULL, li64Timeout) ) < 0, 5 );
    if (liResult < 0)
    {
        return -1;
    }
    else if (liResult == 0)
    {
        //Wait event timeout
        apl_set_errno(APL_EAGAIN);
        return -1;
    }
    
    //If liRetCode = EINTR or EAGAIN .. and retry it
    ACL_RETRY_CHK(
        (liResult = apl_sendto(
            this->GetHandle(), apvBuffer, aiN, aeFlag, aoAddr.GetAddr(), aoAddr.GetLength() ) ) < 0, 5 );
    if (liResult < 0)
    {
        return -1;
    }
    
    return liResult;
}

apl_ssize_t CSockDgram::Send(
    void const* apvBuffer, 
    apl_size_t aiN,
    CSockAddr const& aoAddr,
    CTimeValue const& aoTimeout ) const
{
    return this->Send( apvBuffer, aiN, aoAddr, OPT_UNSPEC, aoTimeout );
}

ACL_NAMESPACE_END
