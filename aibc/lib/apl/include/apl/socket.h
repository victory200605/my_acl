/**
 * @file socket.h 
 * @author  $Author: hezk $
 * @date    $Date: 2009/08/18 09:31:11 $
 * @version $Revision: 1.10 $
 */ 
#ifndef APL_SOCKET_H
#define APL_SOCKET_H

#include "aibc_config.h"
#include "apl/sys.h"
#include "apl/inttypes.h"
#include "apl/types.h"
#include "apl/sockaddr.h"
#include <sys/socket.h>


/* ------------------------------ */ APL_DECLARE_BEGIN /* ------------------------------ */


/**
 * Control data truncated.
 */
#define APL_MSG_CTRUNC      (MSG_CTRUNC)

/**
 * Send without using routing tables.
 */
#define APL_MSG_DONTROUTE   (MSG_DONTROUTE)

/**
 * Terminates a record (if supported by the protocol).
 */
#define APL_MSG_EOR         (MSG_EOR)

/**
 * Out-of-band data.
 */
#define APL_MSG_OOB         (MSG_OOB)

/**
 * Leave received data in queue.
 */
#define APL_MSG_PEEK        (MSG_PEEK)

/**
 * Normal data truncated.
 */
#define APL_MSG_TRUNC       (MSG_TRUNC)

/**
 * Attempt to fill the read buffer.
 */
#define APL_MSG_WAITALL     (MSG_WAITALL)

/**
 * Disables further receive operations.
 */
#define APL_SHUT_RD         (SHUT_RD)

/**
 * Disables further send operations.
 */
#define APL_SHUT_WR         (SHUT_WR)

/**
 * Disables further send and receive operations.
 */
#define APL_SHUT_RDWR       (SHUT_RDWR)

/**
 * Create an endpoint for communication.
 *
 * @param[in]   ai_family   the specified communications domain in which a 
 *                          socket is to be created
 * @param[in]   ai_type     the specified type of socket to be created
 * @param[in]   ai_protocol the specified protocol to be used with the socket
 *
 * @retval >=0  success, the socket file descriptor
 * @retval  -1  failure, and errno set to indicate the error
 */ 
apl_handle_t apl_socket(
    apl_int_t       ai_family, 
    apl_int_t       ai_type,
    apl_int_t       ai_protocol);

/**
 * Bind a name to a socket.
 * 
 * @param[in]   ai_socket   the file descriptor of the socket to be bound
 * @param[in]   aps_sa      the points to a sockaddr structure containing the 
 *                          address to be bound to the socket
 * @param[in]   au_salen    the length of the sockaddr structure pointed to 
 *                          by the address argument
 *
 * @retval 0    success.
 * @retval -1   failure, and errno set to indicate the error
 */ 

apl_int_t apl_bind(
    apl_handle_t                    ai_socket,
    struct apl_sockaddr_t const*    aps_sa,
    apl_size_t                      au_salen);

/**
 * Listen for socket connections and limit the queue of incoming connections.
 *
 * @param[in]   ai_socket   the socket to be listened
 * @param[in]   ai_backlog  the length of the listen queue
 *
 * @retval 0    success.
 * @retval -1   failure, and errno set to indicate the error
 */ 
apl_int_t apl_listen(
    apl_handle_t        ai_socket,
    apl_int_t           ai_backlog);


/**
 * Accept a new connection on a socket.
 *
 * @param[in]   ai_socket   the file descriptor associated with the socket.
 * @param[out]  aps_sa      either a null pointer, or a pointer to a sockaddr 
 *                          structure where the address of the connecting 
 *                          socket shall be returned.
 * @param[out]  apu_salen   the length of the supplied sockaddr structure.
 *
 * @retval >=0  success, file descriptor of the accepted socket.
 * @retval -1   failure, and errno set to indicate the error.
 */ 
apl_int_t apl_accept(
    apl_handle_t            ai_socket,
    struct apl_sockaddr_t*  aps_sa,
    apl_size_t*             apu_salen);

/**
 * Connect a socket.
 *
 * @param[in]       ai_socket   the file descriptor associated with the socket.
 * @param[in]       aps_sa      the pointer to a sockaddr structure containing
 *                              the peer address.
 * @param[in,out]   au_salen    the length of the sockaddr structure.
 *
 * @retval 0    success.
 * @retval -1   failure, and errno set to indicate the error
 */  
apl_int_t apl_connect(
    apl_handle_t                    ai_socket,
    struct apl_sockaddr_t const*    aps_sa,
    apl_size_t                      au_salen);


/**
 * Shut down socket send and receive operations.
 *
 * @param[in]   ai_socket   the file descriptor associated with the socket
 * @param[in]   ai_how      the specified type of shutdown
 *
 * @retval 0    success.
 * @retval -1   failure, and errno set to indicate the error
 */ 
apl_int_t apl_shutdown(
    apl_handle_t         ai_socket,
    apl_int_t            ai_how);


/**
 * Create a pair of connected sockets.
 *
 * @param[in]   ai_family   the specified communications domain in which the 
 *                          sockets are to be created.
 * @param[in]   ai_type     the type of sockets to be created.
 * @param[in]   ai_protocol a particular protocol to be used with the sockets
 * @param[out]  api_socket1 the first socket to store file descriptors of the 
 *                          created socket pair.
 * @param[out]  api_socket2 the other socket to store file descriptors of the 
 *                          created socket pair. 
 *
 * @retval 0    success.
 * @retval -1   failure, and errno set to indicate the error
 */ 
apl_int_t apl_socketpair(
    apl_int_t       ai_family,
    apl_int_t       ai_type,
    apl_int_t       ai_protocol,
    apl_handle_t*   api_socket1,
    apl_handle_t*   api_socket2);


/**
 * Get the socket name.
 *
 * @param[in]   ai_socket   the file descriptor associated with the socket.
 * @param[out]  aps_sa      the apl_sockaddr_t structure to store the address 
 *                          of the retrieve the locally-bound name of the 
 *                          specified socket.
 * @param[out]  apu_salen   the length of aps_sa.
 *
 * @retval 0    success.
 * @retval -1   failure, and errno set to indicate the error
 */ 
apl_int_t apl_getsockname(
    apl_handle_t            ai_socket,
    struct apl_sockaddr_t*  aps_sa,
    apl_size_t*             apu_salen);


/**
 * Get the name of the peer socket.
 *
 * @param[in]       ai_socket   the file descriptor associated with the socket
 * @param[out]      aps_sa      the apl_sockaddr_t address to store retrieve 
 *                              the peer address of the specified socket.
 * @param[in,out]  apu_salen    the length of aps_sa.
 *
 * @retval 0    success.
 * @retval -1   failure, and errno set to indicate the error
 */ 
apl_int_t apl_getpeername(
    apl_handle_t            ai_socket,
    struct apl_sockaddr_t*  aps_sa,
    apl_size_t*             apu_salen);


/**
 * Send a message on a socket.
 *
 * @param[in]   ai_socket   the file descriptor associated with the socket.
 * @param[in]   apv_buf     the pointer to the buffer containing the message 
 *                          to send.
 * @param[in]   au_buflen   the length of the message in bytes.
 * @param[in]   ai_flags    the type of message transmission.
 *
 * @retval >= 0 successful, return the number of bytes sent.
 * @retval -1   failure, and errno set to indicate the error.
 */ 
apl_int_t apl_send(
    apl_handle_t        ai_socket,
    void const*         apv_buf,
    apl_size_t          au_buflen,
    apl_int_t           ai_flags);


/**
 * Receive a message from a connected socket.
 *
 * @param[in]    ai_socket   -   the file descriptor associated with the socket
 * @param[out]   apv_buf     -   the pointer to a buffer where the message should be stored
 * @param[in]    au_buflen   -   the length in bytes of the buffer
 * @param[in]    ai_flags    -   the type of message reception
 *
 * @return
 *     - successful: return the length of the message in bytes or 0
 *     - failed: -1 shall be returned and errno set to indicate the error
 */ 
apl_int_t apl_recv(
    apl_handle_t        ai_socket,
    void*               apv_buf,
    apl_size_t          au_buflen,
    apl_int_t           ai_flags);


/**
 * Send a message on a socket.
 *
 * @param[in]   ai_socket   the file descriptor associated with the socket.
 * @param[in]   apv_buf     the pointer to a buffer containing the message 
 *                          to be sent.
 * @param[in]   au_buflen   the size of the message in bytes.
 * @param[in]   ai_flags    the type of message transmission.
 * @param[in]   aps_sa      the pointer to a sockaddr structure containing 
 *                          the destination address.
 * @param[in]   au_salen    the length of the sockaddr structure.
 *
 * @retval >=0  success, return the number of bytes sent
 * @retval -1   failure, and errno set to indicate the error
 */  
apl_int_t apl_sendto(
    apl_handle_t                    ai_socket,
    void const*                     apv_buf,
    apl_size_t                      au_buflen,
    apl_int_t                       ai_flags,
    struct apl_sockaddr_t const*    aps_sa,
    apl_size_t                      au_salen);


/**
 * Receive a message from a socket.
 * 
 * @param[in]       ai_socket   the file descriptor associated with the socket
 * @param[out]      apv_buf     the pointer to the buffer where the message 
 *                              should be stored
 * @param[in]       au_buflen   the length in bytes of the buffer
 * @param[in]       ai_flags    the type of message reception
 * @param[out]      aps_sa      the apl_sockaddr_t structure to store the 
 *                              sending address
 * @param[in,out]   apu_salen   the length of the apl_sockaddr_t structure
 * 
 * @retval >=0  successful, return the length of the message in bytes. If no 
 *              messages are available to be received and the peer has 
 *              performed an orderly shutdown, apl_recvfrom() shall return 0.
 * @retval -1   failure, and set errno to indicate the error.
 */ 
apl_int_t apl_recvfrom(
    apl_handle_t                    ai_socket,
    void*                           apv_buf,
    apl_size_t                      au_buflen,
    apl_int_t                       ai_flags,
    struct apl_sockaddr_t*          aps_sa,
    apl_size_t*                     apu_salen);


/* ------------------------------ */ APL_DECLARE_END /* ------------------------------ */

#endif /* APL_SOCKET_H */

