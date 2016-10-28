#include "apl/sockaddr.h"
#include "apl/socket.h"
#include "apl/sockopt.h"
#include "apl/errno.h"
#include "apl/str.h"
#include "apl/mem.h"
#include "apl/heap.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <net/if.h>
#include <unistd.h>

#ifdef HAVE_SYS_IOCTL_H
#   include <sys/ioctl.h>
#endif

#ifdef HAVE_SYS_SOCKIO_H
#   include <sys/sockio.h>
#endif


/* ---------------------------------------------------------------------- */

apl_int_t apl_sockaddr_setfamily(
    struct apl_sockaddr_t*  aps_sa,
    apl_int_t               ai_family)
{
    switch (ai_family)
    {
        case APL_AF_UNSPEC:
        case APL_AF_INET:
#if defined(APL_AF_INET6)
        case APL_AF_INET6:
#endif
        case APL_AF_UNIX:
            aps_sa->sa_family = ai_family;
            return 0;

        default:
            apl_set_errno(APL_EAFNOSUPPORT);
            return APL_INT_C(-1);
    }
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_sockaddr_getfamily(
    struct apl_sockaddr_t const*    aps_sa,
    apl_int_t*                      api_family)
{
    switch (aps_sa->sa_family)
    {
        case APL_AF_UNSPEC:
        case APL_AF_INET:
#if defined(APL_AF_INET6)
        case APL_AF_INET6:
#endif
        case APL_AF_UNIX:
            if (api_family != APL_NULL)
            {
                *api_family = aps_sa->sa_family;
            }
            return 0;

        default:
            apl_set_errno(APL_EAFNOSUPPORT);
            return APL_INT_C(-1);
    } 
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_sockaddr_setaddr(
    struct apl_sockaddr_t*  aps_sa,
    apl_size_t              au_salen,
    char const*             apc_addr)
{
    switch (aps_sa->sa_family) 
    {
        case APL_AF_INET: 
        {
            struct sockaddr_in*     lps_sin = (struct sockaddr_in *) aps_sa;

            if (au_salen < sizeof(*lps_sin)) { return APL_INT_C(-1); }

            return inet_pton(
                aps_sa->sa_family, 
                apc_addr, 
                &(lps_sin->sin_addr.s_addr)) == 1 ? 0 : APL_INT_C(-1);
        }
#if defined(APL_AF_INET6)
        case APL_AF_INET6: 
        {
            struct sockaddr_in6*    lps_sin6 = (struct sockaddr_in6 *) aps_sa;

            if (au_salen < sizeof(*lps_sin6)) { return APL_INT_C(-1); }

            return inet_pton(
                aps_sa->sa_family, 
                apc_addr, 
                &(lps_sin6->sin6_addr)) == 1 ? 0 : APL_INT_C(-1);
        }
#endif
        case AF_UNIX:
        {
            struct sockaddr_un*     lps_un = (struct sockaddr_un*) aps_sa;

            apl_strncpy(lps_un->sun_path, apc_addr, au_salen - APL_OFFSETOF(struct sockaddr_un, sun_path));
            return 0;
        }

        default:
            apl_set_errno(APL_EAFNOSUPPORT);
            return APL_INT_C(-1);
    }
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_sockaddr_getaddr(
    struct apl_sockaddr_t const*    aps_sa,
    char*                           apc_addr,
    apl_size_t                      au_addrlen)
{
    switch (aps_sa->sa_family) 
    {
        case APL_AF_INET: 
        {
            struct sockaddr_in* lps_sin = (struct sockaddr_in *) aps_sa;

            if (au_addrlen < INET_ADDRSTRLEN) { return APL_INT_C(-1); }

            if (APL_NULL == inet_ntop(APL_AF_INET, &(lps_sin->sin_addr.s_addr), apc_addr, (size_t)au_addrlen))
            {
                return APL_INT_C(-1);
            }

            return 0;
        }
#if defined(APL_AF_INET6)
        case APL_AF_INET6: 
        {
            struct sockaddr_in6*    lps_sin6 = (struct sockaddr_in6 *) aps_sa;

            if (au_addrlen < INET6_ADDRSTRLEN) { return APL_INT_C(-1); }

            if (APL_NULL == inet_ntop(APL_AF_INET6, &(lps_sin6->sin6_addr), apc_addr, (size_t)au_addrlen))
            {
                return APL_INT_C(-1);
            }

            return 0;
        }
#endif
        case AF_UNIX:
        {
            struct sockaddr_un*     lps_un = (struct sockaddr_un*) aps_sa;

            apl_strncpy(apc_addr, lps_un->sun_path, au_addrlen);

            return 0;
        }

        default:
            apl_set_errno(APL_EAFNOSUPPORT);
            return APL_INT_C(-1);
    }
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_sockaddr_setport(
    struct apl_sockaddr_t*  aps_sa,
    apl_size_t              au_salen,
    apl_uint16_t            au16_port)
{
    switch (aps_sa->sa_family) 
    {
        case APL_AF_INET: 
        {
            struct sockaddr_in*     lps_sin = (struct sockaddr_in *) aps_sa;

            if (au_salen < sizeof(*lps_sin)) 
            { 
                apl_set_errno(APL_EINVAL);
                return APL_INT_C(-1); 
            }

            lps_sin->sin_port = apl_hton16(au16_port);

            return 0;
        }
#if defined(APL_AF_INET6)
        case APL_AF_INET6: 
        {
            struct sockaddr_in6*    lps_sin6 = (struct sockaddr_in6 *) aps_sa;

            if (au_salen < sizeof(*lps_sin6)) 
            { 
                apl_set_errno(APL_EINVAL);
                return APL_INT_C(-1); 
            }

            lps_sin6->sin6_port = apl_hton16(au16_port);

            return 0;
        }
#endif
        default:
            apl_set_errno(APL_EAFNOSUPPORT);
            return APL_INT_C(-1);
    }
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_sockaddr_getport(
    struct apl_sockaddr_t const*    aps_sa,
    apl_uint16_t*                   apu16_port)
{
    switch (aps_sa->sa_family) 
    {
        case APL_AF_INET: 
        {
            struct sockaddr_in*     lps_sin = (struct sockaddr_in *) aps_sa;

            if (apu16_port)
            {
                *apu16_port = (apl_int_t) apl_ntoh16(lps_sin->sin_port);
            }

            return 0;
        }
#if defined(APL_AF_INET6)
        case APL_AF_INET6:
        {
            struct sockaddr_in6*    lps_sin6 = (struct sockaddr_in6 *) aps_sa;

            if (apu16_port)
            {
                *apu16_port = (apl_int_t) apl_ntoh16(lps_sin6->sin6_port);
            }

            return 0;
        }
#endif
        default:
            apl_set_errno(APL_EAFNOSUPPORT);
            return APL_INT_C(-1);
    }
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_sockaddr_setwild(
    struct apl_sockaddr_t*  aps_sa,
    apl_size_t              au_salen)
{
    switch (aps_sa->sa_family)
    {
        case APL_AF_INET:
        {
            struct sockaddr_in*     lps_sin = (struct sockaddr_in *) aps_sa;

            if (au_salen < sizeof(*lps_sin)) { return APL_INT_C(-1); }

            lps_sin->sin_addr.s_addr = apl_hton32(INADDR_ANY);

            return 0;
        }
#if defined(APL_AF_INET)
        case APL_AF_INET6:
        {
            struct sockaddr_in6*     lps_sin6 = (struct sockaddr_in6 *) aps_sa;

            if (au_salen < sizeof(*lps_sin6)) { return APL_INT_C(-1); }

            apl_memcpy(&(lps_sin6->sin6_addr), &in6addr_any, sizeof(lps_sin6->sin6_addr));

            return 0;
        }
#endif
        default:
            apl_set_errno(APL_EAFNOSUPPORT);
            return APL_INT_C(-1);
    }
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_sockaddr_checkwild(
    struct apl_sockaddr_t const*    aps_sa,
    apl_size_t                      au_salen)
{
    switch (aps_sa->sa_family)
    {
        case APL_AF_INET:
        {
            struct sockaddr_in*     lps_sin = (struct sockaddr_in *) aps_sa;

            if (au_salen < sizeof(*lps_sin)) { return APL_INT_C(-1); }

            return (lps_sin->sin_addr.s_addr == apl_hton32(INADDR_ANY)) ? 1 : 0;
        }
#if defined(APL_AF_INET)
        case APL_AF_INET6:
        {
            struct sockaddr_in6*     lps_sin6 = (struct sockaddr_in6 *) aps_sa;

            if (au_salen < sizeof(*lps_sin6)) { return APL_INT_C(-1); }

            return (0 == apl_memcmp(&(lps_sin6->sin6_addr), &in6addr_any, sizeof(lps_sin6->sin6_addr))) ? 1 : 0;
        }
#endif
        default:
            apl_set_errno(APL_EAFNOSUPPORT);
            return APL_INT_C(-1);
    }
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_sockaddr_setloopback(
    struct apl_sockaddr_t*  aps_sa,
    apl_size_t              au_salen)
{
    switch (aps_sa->sa_family)
    {
        case APL_AF_INET:
            return apl_sockaddr_setaddr(aps_sa, au_salen, "127.0.0.1");

#if defined(APL_AF_INET)
        case APL_AF_INET6:
        {
            struct sockaddr_in6*     lps_sin6 = (struct sockaddr_in6 *) aps_sa;

            if (au_salen < sizeof(*lps_sin6)) { return APL_INT_C(-1); }

            apl_memcpy(&(lps_sin6->sin6_addr), &in6addr_loopback, sizeof(lps_sin6->sin6_addr));

            return 0;
        }
#endif
        default:
            apl_set_errno(APL_EAFNOSUPPORT);
            return APL_INT_C(-1);
    }
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_sockaddr_checkloopback(
    struct apl_sockaddr_t const*    aps_sa,
    apl_size_t                      au_salen)
{
    switch (aps_sa->sa_family)
    {
        case APL_AF_INET:
        {
            struct sockaddr_in*     lps_sin = (struct sockaddr_in *) aps_sa;

            if (au_salen < sizeof(*lps_sin)) { return APL_INT_C(-1); }

            return (lps_sin->sin_addr.s_addr == apl_hton32(APL_UINT32_C(0x7F000001))) ? 1 : 0;
        }
#if defined(APL_AF_INET)
        case APL_AF_INET6:
        {
            struct sockaddr_in6*     lps_sin6 = (struct sockaddr_in6 *) aps_sa;

            if (au_salen < sizeof(*lps_sin6)) { return APL_INT_C(-1); }

            return (0 == apl_memcmp(&(lps_sin6->sin6_addr), &in6addr_loopback, sizeof(lps_sin6->sin6_addr))) ? 1 : 0;
        }
#endif
        default:
            apl_set_errno(APL_EAFNOSUPPORT);
            return APL_INT_C(-1);
    }
}

/* ---------------------------------------------------------------------- */

apl_ssize_t apl_sockaddr_getlen(
    struct apl_sockaddr_t const*    aps_sa)
{
    switch (aps_sa->sa_family)
    {
        case APL_AF_INET:
            return sizeof(struct apl_sockaddr_in_t);

#if defined(APL_AF_INET6)
        case APL_AF_INET6:
            return sizeof(struct apl_sockaddr_in6_t);
#endif 

        case APL_AF_UNIX:
            return APL_OFFSETOF(struct apl_sockaddr_un_t, sun_path)
                + apl_strlen(((struct apl_sockaddr_un_t*)aps_sa)->sun_path)
                + 1;

        default:
            apl_set_errno(APL_EAFNOSUPPORT);
            return APL_INT_C(-1);
    }
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_sockaddr_resolve(
    apl_int_t               ai_family,
    apl_int_t               ai_socktype,
    apl_int_t               ai_protocol,
    char const*             apc_hostname,
    char const*             apc_svcname,
    struct apl_sockaddr_t** apps_sas,
    apl_size_t              au_saslen)
{
    apl_size_t          lu_idx;
#if defined(HAVE_GETADDRINFO)
    struct addrinfo     ls_hint;
    struct addrinfo*    lps_addrinfo = NULL;
    struct addrinfo*    lps_itr;
    apl_int_t           li_ret;

    apl_memset(&ls_hint, 0, sizeof(ls_hint));

    ls_hint.ai_family = ai_family;
    ls_hint.ai_socktype = ai_socktype;
    ls_hint.ai_protocol = ai_protocol;

    li_ret = getaddrinfo(apc_hostname, apc_svcname, &ls_hint, &lps_addrinfo);

    if (0 != li_ret)
    {                
        apl_set_errno(APL_GAI_ERRNO_MASK | li_ret);
        return APL_INT_C(-1);
    }        
    
    lps_itr = lps_addrinfo;
    for (lu_idx = 0; lu_idx < au_saslen && lps_itr != NULL; ++lu_idx)        
    {                
        assert(lps_itr->ai_addrlen <= sizeof(struct apl_sockaddr_storage_t));

        apl_memcpy(apps_sas[lu_idx], lps_itr->ai_addr, lps_itr->ai_addrlen);

        lps_itr = lps_itr->ai_next;
    }        

    freeaddrinfo(lps_addrinfo);

    return (apl_int_t)lu_idx;

#else /* gethostbyname support IPv4 only */
    if (APL_AF_INET == ai_family)
    {
        struct hostent*     lps_hresult;
        struct servent*     lps_sresult;
        char**              lppc_pptr;

        if (apc_hostname)
        {
            lps_hresult = gethostbyname(apc_hostname);
            if (NULL == lps_hresult)
            {
                apl_set_errno(APL_H_ERRNO_MASK | h_errno);
                return APL_INT_C(-1);
            }
        }

        if (apc_svcname)
        {
            switch (ai_protocol)
            {
                case 0:
                    lps_sresult = getservbyname(apc_svcname, NULL);
                    break;

                case APL_IPPROTO_TCP:
                    lps_sresult = getservbyname(apc_svcname, "tcp");    
                    break;

                case APL_IPPROTO_UDP:
                    lps_sresult = getservbyname(apc_svcname, "udp");    
                    break;

                default:
                    apl_set_errno(APL_ENOPROTOOPT);
                    return APL_INT_C(-1);
            }

            if (NULL == lps_sresult)
            {
                apl_set_errno(0);
                return APL_INT_C(-1);
            }
        }


        if (lps_hresult)
        {
            lppc_pptr = lps_hresult->h_addr_list;

            for(lu_idx = 0; lu_idx < au_saslen && NULL != lppc_pptr; ++lu_idx, ++lppc_pptr)
            {
                struct apl_sockaddr_in_t* lps_inetaddr = (struct apl_sockaddr_in_t*)&apps_sas[lu_idx];

                assert(sizeof(struct in_addr) == lps_hresult->h_length);
                apl_memcpy(&(lps_inetaddr->sin_addr.s_addr), *lppc_pptr, lps_hresult->h_length);

                if (lps_sresult->s_port)
                {
                    lps_inetaddr->sin_port = lps_sresult->s_port;
                }
            }

            return (apl_int_t)lu_idx;
        }
        else if (lps_sresult && au_saslen >= 1)
        {
            struct apl_sockaddr_in_t* lps_inetaddr = (struct apl_sockaddr_in_t*)&apps_sas[0];

            lps_inetaddr->sin_port = lps_sresult->s_port;

            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        apl_set_errno(APL_EAFNOSUPPORT);
        return APL_INT_C(-1);
    }
#endif
}

apl_int_t apl_get_hostaddr_list(
        char** appcIPAddrList, 
        apl_size_t au_addr_len,
        apl_size_t au_count) 
{
	apl_handle_t li_sockfd;
	apl_int_t li_ret = 0;
	char *lpc_ptr = APL_NULL;
    char *lpc_buf = APL_NULL;
	struct ifconf ls_ifc;
	struct ifreq *lps_ifr;
    apl_size_t lu_len, lu_idx;
    apl_uint32_t luOffset = 0;

	if ( (li_sockfd = apl_socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        return -1;
    }

    lu_len = au_count * sizeof(struct ifreq);
    lpc_buf = (char *)apl_malloc(lu_len);
    assert(lpc_buf != APL_NULL);

	ls_ifc.ifc_len = lu_len;
	ls_ifc.ifc_req = (struct ifreq *) lpc_buf;
	if (ioctl(li_sockfd, SIOCGIFCONF, &ls_ifc) < 0)
    {
        apl_free(lpc_buf);
        close(li_sockfd);
        return -1;
    }

    //check whether the buffer is large enough, we will threat it as 
    //overflow even the buffer is equal
    if (ls_ifc.ifc_len >=  lu_len)
    {
        apl_set_errno(APL_EOVERFLOW);
        return -1;
    }

    luOffset = 0;
    for (lpc_ptr = lpc_buf, lu_idx = 0; 
            lpc_ptr < lpc_buf + ls_ifc.ifc_len && lu_idx < au_count; 
            lu_idx++) 
    {
		lps_ifr = (struct ifreq *) lpc_ptr;
#ifdef	HAVE_SOCKADDR_SA_LEN
        //AIX
		lu_len = sizeof(struct sockaddr);

		if (lps_ifr->ifr_addr.sa_len > lu_len)
			lu_len = lps_ifr->ifr_addr.sa_len;		/* length > 16 */

		lpc_ptr += sizeof(lps_ifr->ifr_name) + lu_len;	/* for next one in buffer */
#else
        lpc_ptr += sizeof(struct ifreq);
#endif
        li_ret = apl_sockaddr_getaddr(&lps_ifr->ifr_addr, appcIPAddrList[lu_idx - luOffset], au_addr_len);

        if (li_ret != 0)
        {
            //ignore the unsupport family(such as AF_LINK and INET6)
            if (apl_get_errno() == APL_EAFNOSUPPORT || lps_ifr->ifr_addr.sa_family == APL_AF_INET6)
            {
                luOffset++;
                li_ret = 0;
                continue;
            }

            break;
        }
	}

    apl_free(lpc_buf);
    close(li_sockfd);

    if (li_ret < 0)
    {
        return li_ret;
    }

    return lu_idx - luOffset;
}

/* ---------------------------------------------------------------------- */

