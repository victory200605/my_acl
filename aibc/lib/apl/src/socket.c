#include "apl/socket.h"
#include "apl/sockopt.h"
#include "apl/errno.h"

/* ---------------------------------------------------------------------- */

apl_handle_t apl_socket(
    apl_int_t  ai_family,
    apl_int_t  ai_type,
    apl_int_t  ai_protocol)
{
    return (apl_handle_t) socket(
        (int)ai_family, 
        (int)ai_type,
        (int)ai_protocol);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_bind(                 
    apl_handle_t                    ai_socket,
    struct apl_sockaddr_t const*    aps_sa,
    apl_size_t                      au_salen)
{
    return (apl_int_t) bind(
        (int)ai_socket,
        aps_sa,
        (apl_socklen_t)au_salen);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_listen(  
    apl_handle_t    ai_socket,
    apl_int_t       ai_backlog)
{
    return (apl_int_t) listen(
        (int)ai_socket,
        (int)ai_backlog);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_accept(       
    apl_handle_t            ai_socket,
    struct apl_sockaddr_t*  aps_sa,
    apl_size_t*             apu_salen)
{
    apl_socklen_t   lu_len;
    apl_int_t       li_ret;

    if (APL_NULL == apu_salen || APL_NULL == aps_sa)
    {
        aps_sa = APL_NULL;
        lu_len = 0;
    }
    else
    {
        lu_len = *apu_salen;
    }

    li_ret = (apl_int_t) accept(
        (int)ai_socket,
        aps_sa,
        (socklen_t*)&lu_len);

    if (0 == li_ret && APL_NULL != aps_sa && APL_NULL != apu_salen)
    {
        *apu_salen = lu_len;
    }

    return li_ret;
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_connect(
    apl_handle_t                    ai_socket,
    struct apl_sockaddr_t const*    aps_sa,
    apl_size_t                      au_salen)
{
    return (apl_int_t) connect(
        (int)ai_socket,
        aps_sa,
        (apl_socklen_t)au_salen);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_shutdown(
    apl_handle_t    ai_socket,
    apl_int_t       ai_how)
{
    return (apl_int_t) shutdown(
        (int)ai_socket,
        (int)ai_how);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_socketpair(
    apl_int_t       ai_family,
    apl_int_t       ai_type,
    apl_int_t       ai_protocol,
    apl_handle_t*   api_socket1,
    apl_handle_t*   api_socket2)
{
    int         lai_socketpair[2];
    apl_int_t   li_ret;
   
    li_ret = (apl_int_t) socketpair(
        (int)ai_family,
        (int)ai_type,
        (int)ai_protocol,
        lai_socketpair);
    
    if (0 == li_ret)
    {
        *api_socket1 = lai_socketpair[0];
        *api_socket2 = lai_socketpair[1];
    }

    return li_ret;
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_getsockname(
    apl_handle_t            ai_socket,
    struct apl_sockaddr_t*  aps_sa,
    apl_size_t*             apu_salen)
{
    apl_socklen_t   lu_len = *apu_salen;
    apl_int_t       li_ret;
   
    li_ret = (apl_int_t) getsockname(
        (int)ai_socket,
        aps_sa,
        (socklen_t*)&lu_len);

    if (0 == li_ret)
    {
        *apu_salen = lu_len;
    }

    return li_ret;
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_getpeername(
    apl_handle_t            ai_socket,
    struct apl_sockaddr_t*  aps_sa,
    apl_size_t*             apu_salen)
{
    apl_socklen_t   lu_len = *apu_salen;
    apl_int_t       li_ret;

    li_ret = (apl_int_t) getpeername(
        (int)ai_socket,
        aps_sa,
        (socklen_t*)&lu_len);

    if (0 == li_ret)
    {
        *apu_salen = lu_len;
    }

    return li_ret;
}

/* ---------------------------------------------------------------------- */

apl_ssize_t apl_send(
    apl_handle_t    ai_socket,
    void const*     apv_buf,
    apl_size_t      au_buflen,
    apl_int_t       ai_flags)
{
    return (apl_ssize_t) send(
        (int)ai_socket,
        apv_buf,
        (size_t)au_buflen,
        (int)ai_flags);
}

/* ---------------------------------------------------------------------- */

apl_ssize_t apl_recv(
    apl_handle_t    ai_socket,
    void*           apv_buf,
    apl_size_t      au_buflen,
    apl_int_t       ai_flags)
{
    return (apl_ssize_t) recv(
        (int)ai_socket,
        apv_buf,
        (size_t)au_buflen,
        (int)ai_flags);
}

/* ---------------------------------------------------------------------- */

apl_ssize_t apl_sendto(
    apl_handle_t                    ai_socket,
    void const*                     apv_buf,
    apl_size_t                      au_buflen,
    apl_int_t                       ai_flags,
    struct apl_sockaddr_t const*    aps_sa,
    apl_size_t                      au_salen)
{
    return (apl_ssize_t) sendto(
        (int)ai_socket,
        apv_buf,
        (size_t)au_buflen,
        (int)ai_flags,
        aps_sa,
        (apl_socklen_t)au_salen);
}

/* ---------------------------------------------------------------------- */

apl_ssize_t apl_recvfrom(
    apl_handle_t                    ai_socket,
    void*                           apv_buf,
    apl_size_t                      au_buflen,
    apl_int_t                       ai_flags,
    struct apl_sockaddr_t*          aps_sa,
    apl_size_t*                     apu_salen)
{
    apl_socklen_t   lu_len = *apu_salen;
    apl_int_t       li_ret;

    li_ret = (apl_ssize_t) recvfrom(
        (int)ai_socket,
        apv_buf,
        (size_t)au_buflen,
        (int)ai_flags,
        aps_sa,
        (socklen_t*)&lu_len);

    if (li_ret == 0)
    {
        *apu_salen = lu_len;
    }

    return li_ret;
}

/* ---------------------------------------------------------------------- */


