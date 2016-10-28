
#include "acl/SockConnector.h"
#include "apl/poll.h"

ACL_NAMESPACE_START

apl_int_t CSockConnector::Connect( 
    CSockStream& aoStream, 
    CSockAddr const& aoRemoteAddr,
    CTimeValue const& aoTimeout,
    CSockAddr const* apoLocalAddr,
    apl_int_t aiProtocol )
{
    apl_int_t  liRetCode   = 0;
    apl_int_t  liSockError = 0;
    apl_time_t li64Timeout = (aoTimeout == CTimeValue::MAXTIME ? -1 : aoTimeout.Nsec() );

    do
    {
        liRetCode = this->Connect(aoStream, aoRemoteAddr, apoLocalAddr, aiProtocol);
        if (liRetCode < 0)
        {
            if (apl_get_errno() == APL_EINPROGRESS)
            {
                ACL_RETRY_CHK(
                    (liRetCode = apl_poll_one(aoStream.GetHandle(), APL_POLLIN|APL_POLLOUT, NULL, li64Timeout) ), 2);
                if ( liRetCode > 0 
                    && aoStream.GetOption(CSockHandle::OPT_ERROR, &liSockError) == 0
                    && liSockError == 0 )
                {
                    break;
                }
                else if (liSockError != 0)
                {
                    apl_set_errno(liSockError);
                }
            }
            
            aoStream.Close();
            
            return -1;
        }
    }while(false);
    
    return 0;
}

apl_int_t CSockConnector::Connect( 
    CSockStream& aoStream, 
    CSockAddr const& aoRemoteAddr,
    CSockAddr const* apoLocalAddr,
    apl_int_t aiProtocol )
{
    apl_int_t  liRetCode   = 0;
    
    //Open socket handle, syscall <socket>
    if (aoStream.Open(aoRemoteAddr.GetFamily(), APL_SOCK_STREAM, aiProtocol) != 0)
    {
        return -1;
    }
    
    //Bind local addr, if required
    if ( apoLocalAddr != NULL &&
         apl_bind(aoStream.GetHandle(), apoLocalAddr->GetAddr(), apoLocalAddr->GetLength() ) != 0 )
    {
        return -1;
    }
    
    //Set socket handle non-block
    aoStream.Enable(CSockHandle::OPT_NONBLOCK);
    
    liRetCode = apl_connect(aoStream.GetHandle(), aoRemoteAddr.GetAddr(), aoRemoteAddr.GetLength() );
    if (liRetCode < 0 && apl_get_errno() != APL_EINPROGRESS)
    {
        aoStream.Close();
    }
    else
    {
        //Start out with non-blocking disabled on the <SockStream>
        aoStream.Disable(CSockHandle::OPT_NONBLOCK);
    }
    
    return liRetCode;
}
	    
ACL_NAMESPACE_END
