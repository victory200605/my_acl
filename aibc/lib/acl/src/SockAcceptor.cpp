
#include "acl/SockAcceptor.h"
#include "apl/poll.h"

ACL_NAMESPACE_START

CSockAcceptor::CSockAcceptor()
{
}

CSockAcceptor::~CSockAcceptor()
{
    this->Close();
}

apl_int_t CSockAcceptor::Open( 
    CSockAddr const& aoAddr, apl_int_t aiFamily, apl_int_t aiBackLog, apl_int_t aiProtocol )
{
    //Open acceptor socket handle
    if ( CSockHandle::Open(aiFamily, APL_SOCK_STREAM, aiProtocol) != 0 )
    {
        return -1;
    }
    
    this->SetOption(CSockHandle::OPT_REUSEADDR, 1);
    
    //Bind acceptor addr
    if ( apl_bind(this->GetHandle(), aoAddr.GetAddr(), aoAddr.GetLength() ) != 0 )
    {
        this->Close();
        return -1;
    }
    
    //Listen acceptor socket handle
    if ( apl_listen(this->GetHandle(), aiBackLog) != 0 )
    {
        this->Close();
        return -1;
    }

    return 0;
}
	
apl_int_t CSockAcceptor::Accept( 
    CSockStream& aoStream, CSockAddr* apoRemoteAddr, CTimeValue const& aoTimeout )
{
    apl_int_t       liRetCode   = 0;
    apl_time_t      li64Timeout = (aoTimeout == CTimeValue::MAXTIME ? -1 : aoTimeout.Nsec() );
    
    //If liRetCode = EINTR or EAGAIN .. and retry it
    ACL_RETRY_CHK(
        (liRetCode = apl_poll_one(this->GetHandle(), APL_POLLIN, NULL, li64Timeout) ) < 0, 5 );
    if (liRetCode < 0)
    {
        return -1;
    }
    else if ( liRetCode == 0 )
    {
        //Wait event timeout
        apl_set_errno(APL_ETIMEDOUT);
        return -1;
    }

    return this->Accept(aoStream, apoRemoteAddr);
}

apl_int_t CSockAcceptor::Accept( CSockStream& aoStream, CSockAddr* apoRemoteAddr )
{
    apl_int_t       liRetCode   = 0;
    apl_size_t      liAddrLen   = (apoRemoteAddr == NULL ? 0 : apoRemoteAddr->GetCapacity() );
    apl_sockaddr_t* lpoAddr     = (apoRemoteAddr == NULL ? NULL : apoRemoteAddr->GetAddr() );

    //If liRetCode = EINTR or EAGAIN .. and retry it
    ACL_RETRY_CHK(
        (liRetCode = apl_accept(this->GetHandle(), lpoAddr, &liAddrLen) ) < 0, 5 );
    if (liRetCode < 0)
    {
        return -1;
    }
    
    aoStream.SetHandle(liRetCode);
    
    return 0;
}

ACL_NAMESPACE_END
