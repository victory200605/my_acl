/**
 * @file sockopt.h 
 * @author  $Author: hezk $
 * @date    $Date: 2009/08/18 09:31:11 $
 * @version $Revision: 1.9 $
 */ 
#ifndef APL_SOCKOPT_H
#define APL_SOCKOPT_H

#include "aibc_config.h"
#include "apl/sys.h"
#include "apl/inttypes.h"
#include "apl/types.h"
#include <sys/socket.h>


/* ------------------------------ */ APL_DECLARE_BEGIN /* ------------------------------ */

/**
 * Unspecified.
 */
#define APL_AF_UNSPEC   (AF_UNSPEC)

/**
 * Internet domain sockets for use with IPv4 addresses.
 */
#define APL_AF_INET     (AF_INET)

/**
 * UNIX domain sockets.
 */
#define APL_AF_UNIX     (AF_UNIX)

#if defined(AF_INET6)
#   define APL_AF_INET6    (AF_INET6)
#endif

/**
 * Byte-stream socket.
 */
#define APL_SOCK_STREAM     (SOCK_STREAM)

/**
 * Datagram socket
 */
#define APL_SOCK_DGRAM      (SOCK_DGRAM)

/**
 * Sequenced-packet socket.
 */
#define APL_SOCK_SEQPACKET  (SOCK_SEQPACKET)

/**
 * Internet protocol.
 */
#define APL_IPPROTO_IP      (IPPROTO_IP)

/**
 * Control message protocol.
 */
#define APL_IPPROTO_ICMP    (IPPROTO_ICMP)

/**
 * Transmission control protocol.
 */
#define APL_IPPROTO_TCP     (IPPROTO_TCP)

/**
 * User datagram protocol.
 */
#define APL_IPPROTO_UDP     (IPPROTO_UDP)

/**
 * Raw IP Packets Protocol.
 */
#define APL_IPPROTO_RAW     (IPPROTO_RAW)

#if defined(AF_INET6) && defined(IPPROTO_IPV6)
#   define APL_IPPROTO_IPV6    (IPPROTO_IPV6)
#endif

/**
 * Set whether transmission of broadcast messages is supported.
 *
 * @param[in]   ai_socket   the file descriptor associated with the opened socket.
 * @param[in]   ai_flag     the option value.
 *
 * @retval 0    success.
 * @retval -1   failure, and errno set to indicate the error
 */ 
apl_int_t apl_setsockopt_broadcast(
    apl_int_t         ai_socket,
    apl_int_t         ai_flag) ;


/**
 * Get whether transmission of broadcast messages is supported.
 *
 * @param[in]   ai_socket   the file descriptor associated with the opened 
 *                          socket
 * @param[out]  api_flag    the option value
 *
 * @retval 0    success.
 * @retval -1   failure, and errno set to indicate the error
 */ 
apl_int_t apl_getsockopt_broadcast(
    apl_int_t         ai_socket,
    apl_int_t*        api_flag) ;



/**
 * Set whether debugging information is being recorded.
 *
 * @param[in]   ai_socket   the file descriptor associated with the opened 
 *                          socket.
 * @param[in]   ai_flag     the option value
 *
 * @retval 0    successful
 * @retval -1   failure, and errno set to indicate the error
 */ 
apl_int_t apl_setsockopt_debug(
    apl_int_t         ai_socket,
    apl_int_t         ai_flag) ;


/**
 * Get whether debugging information is being recorded.
 *
 * @param[in]   ai_socket   the file descriptor associated with the opened 
 *                          socket
 * @param[out]  api_flag    the option value
 *
 * @retval 0    success.
 * @retval -1   failure, and errno set to indicate the error
 */ 
apl_int_t apl_getsockopt_debug(
    apl_int_t         ai_socket,
    apl_int_t*        api_flag) ;


/**
 * Set whether outgoing messages bypass the standard routing facilities.
 *
 * @param[in]    ai_socket   -   the file descriptor associated with the opened socket
 * @param[in]    ai_flag     -   the option value
 *
 * @retval 0    success.
 * @retval -1   failure, and errno set to indicate the error
 */ 
apl_int_t apl_setsockopt_dontroute(
    apl_int_t         ai_socket,
    apl_int_t         ai_flag) ;


/**
 * Get whether outgoing messages bypass the standard routing facilities.
 *
 * @param[in]   ai_socket   the file descriptor associated with the opened 
 *                          socket
 * @param[out]  api_flag    the option value
 *
 * @retval 0    success.
 * @retval -1   failure, and errno set to indicate the error
 */ 
apl_int_t apl_getsockopt_dontroute(
    apl_int_t         ai_socket,
    apl_int_t*        api_flag) ;


/**
 * Set whether connections are kept active with periodic transmission of message.
 *
 * @param[in]    ai_socket   -   the file descriptor associated with the opened socket
 * @param[in]    ai_flag     -   the option value
 *
 * @retval 0    success.
 * @retval -1   failure, and errno set to indicate the error
 */ 
apl_int_t apl_setsockopt_keepalive(
    apl_int_t         ai_socket,
    apl_int_t         ai_flag) ;


/**
 * Get whether connections are kept active with periodic transmission of messages.
 *
 * @param[in]   ai_socket   the file descriptor associated with the opened 
 *                          socket
 * @param[out]  api_flag    the option value
 *
 * @retval 0    success.
 * @retval -1   failure, and errno set to indicate the error
 */ 
apl_int_t apl_getsockopt_keepalive(
    apl_int_t         ai_socket,
    apl_int_t*        api_flag) ;


/**
 * Set receive buffer size information.
 *
 * @param[in]   ai_socket   the file descriptor associated with the opened socket
 * @param[in]   ai_flag     the option value
 *
 * @retval 0    success.
 * @retval -1   failure, and errno set to indicate the error
 */ 
apl_int_t apl_setsockopt_rcvbuf(
    apl_int_t         ai_socket,
    apl_int_t         ai_flag) ;


/**
 * Get receive buffer size information.
 *
 * @param[in]   ai_socket   the file descriptor associated with the opened socket
 * @param[out]  api_flag    the option value
 *
 * @retval 0    success.
 * @retval -1   failure, and errno set to indicate the error
 */ 
apl_int_t apl_getsockopt_rcvbuf(
    apl_int_t         ai_socket,
    apl_int_t*        api_flag) ;


/**
 * Set send buffer size information.
 *
 * @param[in]   ai_socket   the file descriptor associated with the opened socket
 * @param[in]   ai_flag     the option value
 *
 * @retval 0    success.
 * @retval -1   failure, and errno set to indicate the error
 */ 
apl_int_t apl_setsockopt_sndbuf(
    apl_int_t         ai_socket,
    apl_int_t         ai_flag) ;


/**
 * Get send buffer size information.
 *
 * @param[in]   ai_socket   the file descriptor associated with the opened socket
 * @param[out]  api_flag    the option value
 *
 * @retval 0    success.
 * @retval -1   failure, and errno set to indicate the error
 */ 
apl_int_t apl_getsockopt_sndbuf(
    apl_int_t         ai_socket,
    apl_int_t*        api_flag) ;


/**
 * Set socket option value whether allow reusing local address.
 *
 * @param[in]   ai_socket   the file descriptor associated with the opened socket
 * @param[in]   ai_flag     the option value
 *
 * @retval 0    success.
 * @retval -1   failure, and errno set to indicate the error
 */ 
apl_int_t apl_setsockopt_reuseaddr(
    apl_int_t         ai_socket,
    apl_int_t         ai_flag) ;


/**
 * Get socket option value whether allow reusing local address.
 *
 * @param[in]   ai_socket   the file descriptor associated with the opened socket
 * @param[out]  api_flag    the option value
 *
 * @retval 0    success.
 * @retval -1   failure, and errno set to indicate the error
 */ 
apl_int_t apl_getsockopt_reuseaddr(
    apl_int_t         ai_socket,
    apl_int_t*        api_flag) ;


/**
 * Set socket option value whether a socket should remain open after socket 
 * closed.
 *
 * @param[in]   ai_socket   the file descriptor associated with the opened 
 *                          socket.
 * @param[in]   ai_onoff    whether a socket should remain open for a 
 *                          specified amount of time after socket closed and 
 *                          enable queued data to be sent.
 * @param[in]   ai_seconds  the linger time in seconds.
 *
 * @retval 0    success.
 * @retval -1   failure, and errno set to indicate the error
 */ 
apl_int_t apl_setsockopt_linger(
    apl_int_t         ai_socket,
    apl_int_t         ai_onoff,
    apl_int_t         ai_seconds) ;


/**
 * Get socket option value whether a socket should remain open after socket 
 * closed.
 *
 * @param[in]   ai_socket   the file descriptor associated with the opened 
 *                          socket.
 * @param[out]  api_onoff   get on/off infomation of whether a socket should 
 *                          remain open after socket closed
 * @param[out]  api_seconds get the linger time in seconds
 *
 * @retval 0    success.
 * @retval -1   failure, and errno set to indicate the error
 */ 
apl_int_t apl_getsockopt_linger(
    apl_int_t         ai_socket,
    apl_int_t*        api_onoff,
    apl_int_t*        api_seconds) ;


/**
 * Get error status and clears it.
 *
 * @param[in]   ai_socket   the file descriptor associated with the opened 
 *                          socket.
 * @param[out]  api_flag    the option value
 *
 * @retval 0    success.
 * @retval -1   failure, and errno set to indicate the error
 */ 
apl_int_t apl_getsockopt_error(
    apl_int_t         ai_socket,
    apl_int_t*        api_flag) ;


/**
 * Get the socket type.
 *
 * @param[in]   ai_socket   the file descriptor associated with the opened 
 *                          socket.
 * @param[out]  api_flag    the option value.
 *
 * @retval 0    success.
 * @retval -1   failure, and errno set to indicate the error
 */ 
apl_int_t apl_getsockopt_type(
    apl_int_t         ai_socket,
    apl_int_t*        api_flag) ;


/**
 * Get whether socket listening is enabled or disable.
 *
 * @param[in]   ai_socket   the file descriptor associated with the opened 
 *                          socket.
 * @param[out]  api_flag    the option value.
 *
 * @retval 0    success.
 * @retval -1   failure, and errno set to indicate the error
 */ 
apl_int_t apl_getsockopt_acceptconn(
    apl_int_t         ai_socket,
    apl_int_t*        api_flag) ;


/**
 * Set the Nagle algorithm for TCP sockets enable or disable.
 *
 * @param[in]   ai_socket   the file descriptor associated with the opened 
 *                          socket.
 * @param[in]   ai_flag     the option value
 *
 * @retval 0    success.
 * @retval -1   failure, and errno set to indicate the error
 */ 
apl_int_t apl_setsockopt_tcpnodelay(
    apl_int_t         ai_socket,
    apl_int_t         ai_flag) ;


/**
 * Get whether the Nagle algorithm for TCP sockets is enable or disable. 
 *
 * @param[in]   ai_socket   the file descriptor associated with the opened 
 *                          socket.
 * @param[out]  api_flag    the option value.
 *
 * @retval 0    success.
 * @retval -1   failure, and errno set to indicate the error
 */ 
apl_int_t apl_getsockopt_tcpnodelay(
    apl_int_t         ai_socket,
    apl_int_t*        api_flag) ;


/* ------------------------------ */ APL_DECLARE_END /* ------------------------------ */

#endif /* APL_SOCKOPT_H */

