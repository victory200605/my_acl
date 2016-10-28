#include "apl/sockopt.h"
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>

/* ---------------------------------------------------------------------- */

static inline 
apl_int_t do_setsockopt_int(
    apl_int_t       ai_socket,
    apl_int_t       ai_level,
    apl_int_t       ai_optname, 
    apl_int_t       ai_flag) 
{
    int li_value = (int)ai_flag;
    return (apl_int_t) setsockopt(
        (int)ai_socket, 
        (int)ai_level,
        (int)ai_optname,
        &li_value,
        (apl_socklen_t)sizeof(li_value));
}

/* ---------------------------------------------------------------------- */

static inline
apl_int_t do_getsockopt_int(
    apl_int_t       ai_socket,
    apl_int_t       ai_level,
    apl_int_t       ai_optname, 
    apl_int_t*      api_flag)
{
    int             li_value;
    apl_socklen_t   li_vallen = (apl_socklen_t)sizeof(li_value);
    apl_int_t       li_ret;
       
    li_ret = (apl_int_t) getsockopt(
        (int)ai_socket,
        (int)ai_level,
        (int)ai_optname,
        &li_value,
        (socklen_t*)&li_vallen);

    if (li_ret == 0 && api_flag != APL_NULL)
    {
        *api_flag = (apl_int_t)li_value;
    }

    return li_ret;
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_setsockopt_broadcast(
    apl_int_t         ai_socket,
    apl_int_t         ai_flag) 
{
    return do_setsockopt_int(
        ai_socket,
        SOL_SOCKET,
        SO_BROADCAST,
        ai_flag);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_getsockopt_broadcast(
    apl_int_t         ai_socket,
    apl_int_t*        api_flag)
{
    return do_getsockopt_int(
        ai_socket,
        SOL_SOCKET,
        SO_BROADCAST,
        api_flag);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_setsockopt_debug(
    apl_int_t         ai_socket,
    apl_int_t         ai_flag)
{  
    return do_setsockopt_int(
        ai_socket,
        SOL_SOCKET,
        SO_DEBUG,
        ai_flag);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_getsockopt_debug(
    apl_int_t         ai_socket,
    apl_int_t*        api_flag)
{
    return do_getsockopt_int(
        ai_socket,
        SOL_SOCKET,
        SO_DEBUG,
        api_flag);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_setsockopt_dontroute(
    apl_int_t         ai_socket,
    apl_int_t         ai_flag)
{ 
    return do_setsockopt_int(
        ai_socket,
        SOL_SOCKET,
        SO_DONTROUTE,
        ai_flag);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_getsockopt_dontroute(
    apl_int_t         ai_socket,
    apl_int_t*        api_flag)
{
    return do_getsockopt_int(
        ai_socket,
        SOL_SOCKET,
        SO_DONTROUTE,
        api_flag);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_setsockopt_keepalive(
    apl_int_t         ai_socket,
    apl_int_t         ai_flag)
{
    return do_setsockopt_int(
        ai_socket,
        SOL_SOCKET,
        SO_KEEPALIVE,
        ai_flag);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_getsockopt_keepalive(
    apl_int_t         ai_socket,
    apl_int_t*        api_flag) 
{
    return do_getsockopt_int(
        ai_socket,
        SOL_SOCKET,
        SO_KEEPALIVE,
        api_flag);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_setsockopt_rcvbuf(
    apl_int_t         ai_socket,
    apl_int_t         ai_flag)
{
    return do_setsockopt_int(
        ai_socket,
        SOL_SOCKET,
        SO_RCVBUF,
        ai_flag);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_getsockopt_rcvbuf(
    apl_int_t         ai_socket,
    apl_int_t*        api_flag)
{
    return do_getsockopt_int(
        ai_socket,
        SOL_SOCKET,
        SO_RCVBUF,
        api_flag);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_setsockopt_sndbuf(
    apl_int_t         ai_socket,
    apl_int_t         ai_flag)
{
    return do_setsockopt_int(
        ai_socket,
        SOL_SOCKET,
        SO_SNDBUF,
        ai_flag);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_getsockopt_sndbuf(
    apl_int_t         ai_socket,
    apl_int_t*        api_flag) 
{
    return do_getsockopt_int(
        ai_socket,
        SOL_SOCKET,
        SO_SNDBUF,
        api_flag);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_setsockopt_reuseaddr(
    apl_int_t         ai_socket,
    apl_int_t         ai_flag)
{
    return do_setsockopt_int(
        ai_socket,
        SOL_SOCKET,
        SO_REUSEADDR,
        ai_flag);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_getsockopt_reuseaddr(
    apl_int_t         ai_socket,
    apl_int_t*        api_flag)
{
    return do_getsockopt_int(
        ai_socket,
        SOL_SOCKET,
        SO_REUSEADDR,
        api_flag);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_setsockopt_linger(
    apl_int_t         ai_socket,
    apl_int_t         ai_onoff,
    apl_int_t         ai_seconds) 
{
    struct linger ls_linger;
    ls_linger.l_onoff = (int)ai_onoff;
    ls_linger.l_linger = (int)ai_seconds;

    return (apl_int_t) setsockopt(
        (int)ai_socket, 
        SOL_SOCKET,
        SO_LINGER,
        &ls_linger,
        (apl_socklen_t)sizeof(ls_linger));
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_getsockopt_linger(
    apl_int_t         ai_socket,
    apl_int_t*        api_onoff,
    apl_int_t*        api_seconds)
{
    struct linger ls_linger;
    apl_socklen_t li_vallen = (apl_socklen_t)sizeof(ls_linger);
 
    apl_int_t li_ret;
   
    li_ret = (apl_int_t) getsockopt(
        (int)ai_socket,
        SOL_SOCKET,
        SO_LINGER,
        &ls_linger,
        (socklen_t*)&li_vallen);

    if (li_ret == 0)
    {
        if ( api_onoff!= APL_NULL)
        {
            *api_onoff = (apl_int_t)ls_linger.l_onoff;
        }

        if (api_seconds)
        {
            *api_seconds = (apl_int_t)ls_linger.l_linger;
        }
    }

    return li_ret;

}

/* ---------------------------------------------------------------------- */

apl_int_t apl_getsockopt_error(
    apl_int_t         ai_socket,
    apl_int_t*        api_flag)
{
    return do_getsockopt_int(
        ai_socket,
        SOL_SOCKET,
        SO_ERROR,
        api_flag);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_getsockopt_type(
    apl_int_t         ai_socket,
    apl_int_t*        api_flag)
{
    return do_getsockopt_int(
        ai_socket,
        SOL_SOCKET,
        SO_TYPE,
        api_flag);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_getsockopt_acceptconn(
    apl_int_t         ai_socket,
    apl_int_t*        api_flag)
{
    return do_getsockopt_int(
        ai_socket,
        SOL_SOCKET,
        SO_ACCEPTCONN,
        api_flag);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_setsockopt_tcpnodelay(
    apl_int_t         ai_socket,
    apl_int_t         ai_flag)
{
    return do_setsockopt_int(
        ai_socket,
        IPPROTO_TCP,
        TCP_NODELAY,
        ai_flag);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_getsockopt_tcpnodelay(
    apl_int_t         ai_socket,
    apl_int_t*        api_flag)
{
    return do_getsockopt_int(
        ai_socket,
        IPPROTO_TCP,
        TCP_NODELAY,
        api_flag);
}

/* ---------------------------------------------------------------------- */
