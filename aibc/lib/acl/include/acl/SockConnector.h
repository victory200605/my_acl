/**
 * @file SockConnector.h
 */

#ifndef ACL_SOCKCONNECTOR_H
#define ACL_SOCKCONNECTOR_H

#include "acl/Utility.h"
#include "acl/TimeValue.h"
#include "acl/SockAddr.h"
#include "acl/SockStream.h"

ACL_NAMESPACE_START

/**
 * class CSockConnector
 */
class CSockConnector
{
public:
    /**
     * @brief Attempt to make a connection on a socket.
     *
     * @param [out] aoStream       the socket stream object
     * @param [in]  aoRemoteAddr   the remote address
     * @param [in]  aoTimeout      timeout
     * - =CTimeValue::MAXTIME : never timeout
     * - =CTimeValue::ZERO : non-blocking
     * - >0 : blocking
     * @param [in] apoLocalAddr    the local address
     * @param [in] aiProtocol      the particular protocol to be used with the socket
     *
     * @retval 0 Upon successful completion.
     * @retval -1 If an error is detected.
     */
	apl_int_t Connect( 
	    CSockStream& aoStream, 
	    CSockAddr const& aoRemoteAddr,
	    CTimeValue const& aoTimeout,
	    CSockAddr const* apoLocalAddr = NULL,
	    apl_int_t aiProtocol = 0 );
	
	/**
     * @brief Attempt to make a connection on a socket non-block.
     *
     * @param [out] aoStream       the socket stream object
     * @param [in]  aoRemoteAddr   the remote address
     * @param [in]  apoLocalAddr    the local address
     * @param [in]  aiProtocol      the particular protocol to be used with the socket
     *
     * @retval 0 Upon successful completion.
     * @retval -1 If an error is detected.
     */
	apl_int_t Connect( 
	    CSockStream& aoStream, 
	    CSockAddr const& aoRemoteAddr,
	    CSockAddr const* apoLocalAddr = NULL,
	    apl_int_t aiProtocol = 0 );
};

ACL_NAMESPACE_END

#endif//ACL_SOCKCONNECTOR_H
