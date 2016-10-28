/**
 * @file SockAcceptor.h
 */

#ifndef ACL_SOCKACCEPTOR_H
#define ACL_SOCKACCEPTOR_H

#include "acl/SockHandle.h"
#include "acl/SockStream.h"

ACL_NAMESPACE_START

/**
 * class CSockAcceptor
 */
class CSockAcceptor : public CSockHandle
{
public:
    /**
     * @brief A constructor.
     */
    CSockAcceptor();
    
    /**
     * @brief A destructor.
     */
    ~CSockAcceptor();
    
    /**
     * @brief Open the socket and bind the address,then listen the socket handle.
     *
     * @param [in] aoAddr      the address to be bound to the socket handle
     * @param [in] aiFamily    the specified communications domain in which a socket is to be created 
     * @param [in] aiBackLog   the length of the listen queue 
     * @param [in] aiProtocol  the particular protocol to be used with the socket
     *
     * @retval 0 Upon successful completion.
     * @retval -1 If an error is detected.
     */
	apl_int_t Open( 
	    CSockAddr const& aoAddr,
	    apl_int_t aiFamily = APL_AF_INET,
	    apl_int_t aiBackLog = 10,
	    apl_int_t aiProtocol = 0 );
	
    /**
     * @brief Accept a new connection on a socket.
     *
     * @param [out] aoStream         the socket stream object created
     * @param [out] apoRemoteAddr    the remote address, If apoRemoteAddr is a null pointe, 
     *                               where the address of the connecting socket shall not be returned
     * @param [in]  aoTimeout        timeout      
     * - =CTimeValue::MAXTIME : never timeout
     * - =CTimeValue::ZERO : non-blocking
     * - >0 : blocking
     *
     * @retval 0 Upon successful completion.
     * @retval -1 If an error is detected.
     */
	apl_int_t Accept( CSockStream& aoStream, CSockAddr* apoRemoteAddr, CTimeValue const& aoTimeout );
	
	/**
     * @brief Accept a new connection on a socket non-block.
     *
     * @param [out] aoStream         the socket stream object created
     * @param [out] apoRemoteAddr    the remote address
     *
     * @retval 0 Upon successful completion.
     * @retval -1 If an error is detected.
     */
	apl_int_t Accept( CSockStream& aoStream, CSockAddr* apoRemoteAddr = NULL);

private:
    CSockAcceptor( CSockAcceptor const& );
    
    CSockAcceptor& operator = ( CSockAcceptor const& );
};

ACL_NAMESPACE_END

#endif//ACL_SOCKACCEPTOR_H
