/**
 *
 */
#ifndef APL__SOCKET_H
#define APL__SOCKET_H

#include "apl/common.h"
#include "apl/types.h"

APL_BEGIN_C_DECLARE

/**
 *
 */
int apl_socket_open(
    apl_socket_t*   p_sock,
    int             family,
    int             type,
    int             proto
); 

/**
 *
 */
int apl_socket_close(
    apl_socket_t*   p_sock
);

/**
 *
 */
int apl_socket_bind(
    apl_socket_t* restrict      p_sock,
    apl_sockaddr_t* restrict    p_saddr
);

/**
 *
 */
int apl_socket_listen(
    apl_socket_t*   p_sock,
    int             backlog 
);

/**
 *
 */
int apl_socket_connect(
    apl_socket_t* restrict      p_sock,
    apl_sockaddr_t* restrict    p_saddr
);

/**
 *
 */
int apl_socket_accept(
    apl_socket_t* restrict  p_sock, 
    apl_socket_t* restrict  p_newsock
);

/**
 *
 */
int apl_socket_send(
    apl_socket_t* restrict      p_sock,
    apl_byte_t const* restrict  p_buf,
    size_t                      sz,
    int                         flag,
    size_t* restrict            p_rest
);

/**
 *
 */
int apl_socket_recv(
    apl_socket_t* restrict  p_sock,
    apl_byte_t* restrict    p_buf,
    size_t                  sz,
    int                     flag,
    size_t* restrict        p_rest
);

/**
 *
 */
int apl_socket_sendto(
    apl_socket_t* restrict          p_sock,
    apl_sockaddr_t const* restrict  p_saddr,
    apl_byte_t const* restrict      p_buf,
    size_t                          sz,
    size_t* restrict                p_rest
);

/**
 *
 */
int apl_socket_recvfrom(
    apl_socket_t* restrict      p_sock,
    apl_sockaddr_t* restrict    p_saddr,
    apl_byte_t* restrict        p_buf,
    size_t                      sz,
    size_t* restrict            p_rest
);

/**
 *
 */
int apl_socket_atmark(
    apl_socket_t*   p_sock,
);

/**
 *
 */
int apl_socket_atmark(
    apl_socket_t*   p_sock,
);

/**
 *
 */
int apl_socket_get_opt(
    apl_socket_t* restrict  p_sock,
    int                     level,
    int                     name,
    int* restrict           p_value 
);

/**
 *
 */
int apl_socket_set_opt(
    apl_socket_t*   p_sock,
    int             level,
    int             name,
    int             value 
);

/**
 *
 */
int apl_socket_get_timeout(
    apl_socket_t* restrict  p_sock,
    int                     buftype,
    apl_time_t* restrict    reltime,
);

/**
 *
 */
int apl_socket_set_timeout(
    apl_socket_t*   p_sock,
    int             buftype,
    apl_time_t      reltime,
);

/**
 *
 */
int apl_socket_get_linger(
    apl_socket_t* restrict  p_sock,
    int* restrict           onoff,
    apl_time_t*             reltime
);

/**
 *
 */
int apl_socket_set_linger(
    apl_socket_t*   p_sock,
    int             onoff,
    apl_time_t      reltime
);


APL_END_C_DECLARE

#endif // APL_SOCKET_H
