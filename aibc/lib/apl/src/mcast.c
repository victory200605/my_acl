#include "apl/mcast.h"
#include "apl/errno.h"
#include "apl/heap.h"
#include "apl/sockopt.h"
#include "apl/str.h"
#include "apl/mem.h"
#include <unistd.h>
#include <sys/stropts.h>
#include <net/if.h>

#ifdef HAVE_SYS_IOCTL_H
#   include <sys/ioctl.h>
#endif

#ifdef HAVE_SYS_SOCKIO_H
#   include <sys/sockio.h>
#endif

#define max(a,b)    ((a) > (b) ? (a) : (b))
/* ---------------------------------------------------------------------- */

inline
int sockfd_to_family(int sockfd)
{
    struct sockaddr_storage ls_ss;
    apl_socklen_t           lu_len;

    lu_len = sizeof(ls_ss);
    
    if (getsockname(sockfd, (struct sockaddr*)&ls_ss, (socklen_t*)&lu_len) < 0)
    {
        return APL_INT_C(-1);
    }
    
    return ls_ss.ss_family;
}

/* ---------------------------------------------------------------------- */

inline
int family_to_level(int family)
{
    switch (family) 
    {
        case APL_AF_INET:
            return IPPROTO_IP;

#if defined(APL_AF_INET6)
        case APL_AF_INET6:
            return IPPROTO_IPV6;
#endif
        default:
            return APL_INT_C(-1);
    }
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_mcast_join(
    apl_handle_t                    ai_socket,
    struct apl_sockaddr_t const*    aps_sagrp,
    apl_size_t                      au_sagrplen,
    char const*                     apc_ifname,
    apl_uint_t                      au_ifindex)
{
#if defined(MCAST_JOIN_GROUP)
    struct group_req ls_grpreq;
    if (au_ifindex > 0) 
    {
        ls_grpreq.gr_interface = au_ifindex;
    } 
    else if (apc_ifname != NULL) 
    {
        if ( (ls_grpreq.gr_interface = if_nametoindex(apc_ifname)) == 0) 
        {
            apl_set_errno(APL_ENXIO);  /* i/f name not found */
            return APL_INT_C(-1);
        }
    } 
    else
    {
        ls_grpreq.gr_interface = 0;
    }
    
    if (au_sagrplen > sizeof(ls_grpreq.gr_group)) 
    {
        apl_set_errno(APL_EINVAL);
        return APL_INT_C(-1);
    }
    
    apl_memcpy(&ls_grpreq.gr_group, aps_sagrp, au_sagrplen);

    return (apl_int_t) setsockopt(
        ai_socket, 
        family_to_level(aps_sagrp->sa_family), 
        MCAST_JOIN_GROUP, 
        &ls_grpreq, 
        sizeof(ls_grpreq));
#else
    switch (aps_sagrp->sa_family) 
    {
        case APL_AF_INET: 
        {
            struct ip_mreq          ls_mreq;
            struct ifreq            ls_ifreq;
                                                             
            apl_memcpy(
                &ls_mreq.imr_multiaddr, 
                &((const struct sockaddr_in *) aps_sagrp)->sin_addr, 
                sizeof(struct in_addr));
                                                                               
            if (au_ifindex > 0) 
            {
                if (if_indextoname(au_ifindex, ls_ifreq.ifr_name) == NULL) 
                {
                    apl_set_errno(APL_ENXIO);  /* i/f index not found */
                    return APL_INT_C(-1);
                }

                goto doioctl;
            } 
            else if (apc_ifname != NULL) 
            {
                apl_strncpy(ls_ifreq.ifr_name, apc_ifname, IFNAMSIZ);
doioctl:
                if (ioctl(ai_socket, SIOCGIFADDR, &ls_ifreq) < 0)
                {
                    return APL_INT_C(-1);
                }
                
                apl_memcpy(
                    &ls_mreq.imr_interface, 
                    &((struct sockaddr_in *) &ls_ifreq.ifr_addr)->sin_addr, 
                    sizeof(struct in_addr));
            } 
            else
            {
                ls_mreq.imr_interface.s_addr = apl_hton32(INADDR_ANY);
            }

            return(apl_int_t) setsockopt(
                ai_socket, 
                APL_IPPROTO_IP, 
                IP_ADD_MEMBERSHIP, 
                &ls_mreq, 
                sizeof(ls_mreq));
        }    

#   if defined(IPV6_JOIN_GROUP)
        case APL_AF_INET6:
        {
            struct ipv6_mreq        ls_mreq6;

            apl_memcpy(
                &ls_mreq6.ipv6mr_multiaddr, 
                &((const struct sockaddr_in6 *) aps_sagrp)->sin6_addr, 
                sizeof(struct in6_addr));

            if (au_ifindex > 0) 
            {
                ls_mreq6.ipv6mr_interface = au_ifindex;
            } 
            else if (apc_ifname != NULL) 
            {
                if ((ls_mreq6.ipv6mr_interface = if_nametoindex(apc_ifname)) == 0) 
                {
                    apl_set_errno(APL_ENXIO);  /* i/f name not found */
                    return APL_INT_C(-1);
                }
            } 
            else
            {
                ls_mreq6.ipv6mr_interface = 0;
            }
            
            return (apl_int_t) setsockopt(
                ai_socket, 
                APL_IPPROTO_IPV6, 
                IPV6_JOIN_GROUP, 
                &ls_mreq6, 
                sizeof(ls_mreq6));
        }
#   endif

        default:
            apl_set_errno(APL_EAFNOSUPPORT);
            return APL_INT_C(-1);
    }
#endif
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_mcast_leave(
    apl_handle_t                    ai_socket,
    struct apl_sockaddr_t const*    aps_sagrp,
    apl_size_t                      au_sagrplen)
{
#if defined(MCAST_LEAVE_GROUP)
    struct group_req ls_grpreq;
    ls_grpreq.gr_interface = 0;

    if (au_sagrplen > sizeof(ls_grpreq.gr_group)) 
    {
        apl_set_errno(APL_EINVAL);
        return APL_INT_C(-1);
    }

    apl_memcpy(&ls_grpreq.gr_group, aps_sagrp, au_sagrplen);
    return (apl_int_t) setsockopt(
        ai_socket, 
        family_to_level(aps_sagrp->sa_family),
        MCAST_LEAVE_GROUP, 
        &ls_grpreq, 
        sizeof(ls_grpreq));
#else
    switch (aps_sagrp->sa_family) 
    {
        case APL_AF_INET: 
        {
            struct ip_mreq          ls_mreq;

            apl_memcpy(
                &ls_mreq.imr_multiaddr,
                &((const struct sockaddr_in *) aps_sagrp)->sin_addr,
                sizeof(struct in_addr));
             
            ls_mreq.imr_interface.s_addr = apl_hton32(INADDR_ANY);

            return (apl_int_t) setsockopt(
                ai_socket,
                APL_IPPROTO_IP, 
                IP_DROP_MEMBERSHIP,
                &ls_mreq, 
                sizeof(ls_mreq));
        }

#   if defined(IPV6_DROP_MEMBERSHIP)
        case APL_AF_INET6:
        {
            struct ipv6_mreq        ls_mreq6;

            apl_memcpy(
                &ls_mreq6.ipv6mr_multiaddr,
                &((const struct sockaddr_in6 *) aps_sagrp)->sin6_addr,
                sizeof(struct in6_addr));

            ls_mreq6.ipv6mr_interface = 0;

            return (apl_int_t) setsockopt(
                ai_socket, 
                APL_IPPROTO_IPV6, 
                IPV6_LEAVE_GROUP,
                &ls_mreq6, 
                sizeof(ls_mreq6));
        }
#   endif
        default:
            apl_set_errno(APL_EAFNOSUPPORT);
            return APL_INT_C(-1);
    }
#endif
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_mcast_block_source(
    apl_handle_t                    ai_socket,
    struct apl_sockaddr_t const*    aps_sasrc,
    apl_size_t                      au_sasrclen,
    struct apl_sockaddr_t const*    aps_sagrp,
    apl_size_t                      au_sagrplen,
    char const*                     apc_ifname,
    apl_uint_t                      au_ifindex)
{
#if defined(MCAST_JOIN_SOURCE_GROUP)
    struct group_source_req ls_grpreq;

    if (au_ifindex > 0) 
    {
        ls_grpreq.gsr_interface = au_ifindex;
    } 
    else if (apc_ifname != NULL) 
    {
        if ( (ls_grpreq.gsr_interface = if_nametoindex(apc_ifname)) == 0) 
        {
            apl_set_errno(APL_ENXIO);  /* i/f name not found */
            return APL_INT_C(-1);
        }
    } 
    else
    {
        ls_grpreq.gsr_interface = 0;
    }
    
    if (au_sagrplen > sizeof(ls_grpreq.gsr_group) || au_sasrclen > sizeof(ls_grpreq.gsr_source)) 
    {
        apl_set_errno(APL_EINVAL);
        return APL_INT_C(-1);
    }

    apl_memcpy(&ls_grpreq.gsr_group, aps_sagrp, au_sagrplen);
    apl_memcpy(&ls_grpreq.gsr_source, aps_sasrc, au_sasrclen);

    return (apl_int_t) setsockopt(
        ai_socket, 
        family_to_level(aps_sagrp->sa_family),
        MCAST_JOIN_SOURCE_GROUP, 
        &ls_grpreq, 
        sizeof(ls_grpreq));
#else
    switch (aps_sagrp->sa_family) 
    {
#   if defined(IP_BLOCK_SOURCE)
        case APL_AF_INET:
        {
            struct ip_mreq_source   ls_mreq;

            apl_memcpy(
                &ls_mreq.imr_multiaddr,
                &((struct sockaddr_in *) aps_sagrp)->sin_addr,
                sizeof(struct in_addr));

            apl_memcpy(
                &ls_mreq.imr_sourceaddr,
                &((struct sockaddr_in *) aps_sasrc)->sin_addr,
                sizeof(struct in_addr));

            ls_mreq.imr_interface.s_addr = apl_hton32(INADDR_ANY);

            return (apl_int_t) setsockopt(
                ai_socket, 
                APL_IPPROTO_IP, 
                IP_BLOCK_SOURCE,
                &ls_mreq, 
                sizeof(ls_mreq));
        }
#   endif

        default:
            apl_set_errno(APL_EAFNOSUPPORT);
            return APL_INT_C(-1);
    }
#endif
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_mcast_unblock_source(
    apl_handle_t                    ai_socket, 
    struct apl_sockaddr_t const*    aps_sasrc, 
    apl_size_t                      au_sasrclen,
    struct apl_sockaddr_t const*    aps_sagrp, 
    apl_size_t                      au_sagrplen)
{
#if defined(MCAST_UNBLOCK_SOURCE)
    struct group_source_req ls_grpreq;

    ls_grpreq.gsr_interface = 0;

    if (au_sagrplen > sizeof(ls_grpreq.gsr_group) || au_sasrclen > sizeof(ls_grpreq.gsr_source))
    {
        apl_set_errno(APL_EINVAL);
        return APL_INT_C(-1);
    }

    apl_memcpy(&ls_grpreq.gsr_group, aps_sagrp, au_sagrplen);
    apl_memcpy(&ls_grpreq.gsr_source, aps_sasrc, au_sasrclen);

    return (apl_int_t) setsockopt(
        ai_socket, 
        family_to_level(aps_sagrp->sa_family),
        MCAST_UNBLOCK_SOURCE, 
        &ls_grpreq, 
        sizeof(ls_grpreq));
#else
    switch (aps_sagrp->sa_family) 
    { 
#   if defined(IP_UNBLOCK_SOURCE)
        case APL_AF_INET:
        {
            struct ip_mreq_source   ls_mreq;

            apl_memcpy(
                &ls_mreq.imr_multiaddr,
                &((struct sockaddr_in *) grp)->sin_addr,
                sizeof(struct in_addr));

            apl_memcpy(
                &ls_mreq.imr_sourceaddr,
                &((struct sockaddr_in *) aps_sasrc)->sin_addr,
                sizeof(struct in_addr));

            ls_mreq.imr_interface.s_addr = apl_hton32(INADDR_ANY);

            return (apl_int_t) setsockopt(
                ai_socket, 
                APL_IPPROTO_IP, 
                IP_UNBLOCK_SOURCE,
                &ls_mreq, 
                sizeof(ls_mreq));
        }
#   endif
    
        default:
            apl_set_errno(APL_EAFNOSUPPORT);
            return APL_INT_C(-1);
    }
#endif
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_mcast_join_source_group(
    apl_handle_t                    ai_socket,
    struct apl_sockaddr_t const*    aps_sasrc,
    apl_size_t                      au_sasrclen,
    struct apl_sockaddr_t const*    aps_sagrp,
    apl_size_t                      au_sagrplen,
    char const*                     apc_ifname,
    apl_uint_t                      au_ifindex)
{
#if defined(MCAST_JOIN_SOURCE_GROUP)
    struct group_source_req ls_grpreq;

    if (au_ifindex > 0) 
    {
        ls_grpreq.gsr_interface = au_ifindex;
    } 
    else if (apc_ifname != NULL) 
    {
        if ((ls_grpreq.gsr_interface = if_nametoindex(apc_ifname)) == 0) 
        {
            apl_set_errno(APL_ENXIO);  /* i/f name not found */
            return APL_INT_C(-1);
        }
    } 
    else
    {
        ls_grpreq.gsr_interface = 0;
    }

    if (au_sagrplen > sizeof(ls_grpreq.gsr_group) || au_sasrclen > sizeof(ls_grpreq.gsr_source)) 
    {
        apl_set_errno(APL_EINVAL);
        return APL_INT_C(-1);
    }

    apl_memcpy(&ls_grpreq.gsr_group, aps_sagrp, au_sagrplen);
    apl_memcpy(&ls_grpreq.gsr_source, aps_sasrc, au_sasrclen);

    return (apl_int_t) setsockopt(
        ai_socket, 
        family_to_level(aps_sagrp->sa_family),
        MCAST_JOIN_SOURCE_GROUP, 
        &ls_grpreq, 
        sizeof(ls_grpreq));
#else
    switch (aps_sagrp->sa_family) 
    {
#   if defined(IP_ADD_SOURCE_MEMBERSHIP)
        case AF_INET: 
        {
            struct ip_mreq_source   ls_mreq;
            struct ifreq            ls_ifreq;

            apl_memcpy(
                &ls_mreq.imr_multiaddr,
                &((struct sockaddr_in *) aps_sagrp)->sin_addr,
                sizeof(struct in_addr));

            apl_memcpy(
                &ls_mreq.imr_sourceaddr,
                &((struct sockaddr_in *) aps_sasrc)->sin_addr,
                sizeof(struct in_addr));

            if (au_ifindex > 0) 
            {
                if (if_indextoname(au_ifindex, ls_ifreq.ifr_name) == NULL) 
                {
                    apl_set_errno(APL_ENXIO);  /* i/f index not found */
                    return APL_INT_C(-1);
                }

                goto doioctl;
            } 
            else if (apc_ifname != NULL) 
            {
                apl_strncpy(ls_ifreq.ifr_name, apc_ifname, IFNAMSIZ);
doioctl:
                if (ioctl(sockfd, SIOCGIFADDR, &ifreq) < 0)
                {
                    return(-1);
                }
                
                apl_memcpy(
                    &ls_mreq.imr_interface,
                    &((struct sockaddr_in *) &ls_ifreq.ifr_addr)->sin_addr,
                    sizeof(struct in_addr));
            } 
            else
            {
                ls_mreq.imr_interface.s_addr = apl_hton32(INADDR_ANY);
            }

            return (apl_int_t) setsockopt(
                ai_socket, 
                APL_IPPROTO_IP, 
                IP_ADD_SOURCE_MEMBERSHIP,
                &ls_mreq, 
                sizeof(ls_mreq));
        }
#   endif
        default:
            apl_set_errno(APL_EAFNOSUPPORT);
            return APL_INT_C(-1);   
    }            
#endif
}

/* ---------------------------------------------------------------------- */

apl_int_t   apl_mcast_leave_source_group(
    apl_handle_t                    ai_socket, 
    struct apl_sockaddr_t const*    aps_sasrc, 
    apl_size_t                      au_sasrclen,
    struct apl_sockaddr_t const*    aps_sagrp, 
    apl_size_t                      au_sagrplen)
{
#if defined(MCAST_LEAVE_SOURCE_GROUP)
    struct group_source_req ls_grpreq;
    ls_grpreq.gsr_interface = 0;

    if (au_sagrplen > sizeof(ls_grpreq.gsr_group) 
        || au_sasrclen > sizeof(ls_grpreq.gsr_source))
    {
        apl_set_errno(APL_EINVAL);
        return APL_INT_C(-1);
    }
    
    apl_memcpy(&ls_grpreq.gsr_group, aps_sagrp, au_sagrplen);
    apl_memcpy(&ls_grpreq.gsr_source, aps_sasrc, au_sasrclen);

    return (apl_int_t) setsockopt(
        ai_socket, 
        family_to_level(aps_sagrp->sa_family),
        MCAST_LEAVE_SOURCE_GROUP, 
        &ls_grpreq, 
        sizeof(ls_grpreq));
#else
    switch (aps_sagrp->sa_family) 
    {
#   if defined(IP_DROP_SOURCE_MEMBERSHIP)
        case APL_AF_INET: 
        {
            struct ip_mreq_source   ls_mreq;

            apl_memcpy(
                &ls_mreq.imr_multiaddr,
                &((struct sockaddr_in *) aps_sagrp)->sin_addr,
                sizeof(struct in_addr));

            apl_memcpy(
                &ls_mreq.imr_sourceaddr,
                &((struct sockaddr_in *) aps_sasrc)->sin_addr,
                sizeof(struct in_addr));

            ls_mreq.imr_interface.s_addr = apl_hton32(INADDR_ANY);

            return (apl_int_t) setsockopt(
                ai_socket,
                APL_IPPROTO_IP, 
                IP_DROP_SOURCE_MEMBERSHIP,
                &ls_mreq, 
                sizeof(ls_mreq));
        }
#   endif
        default:
            apl_set_errno(APL_EAFNOSUPPORT);
            return APL_INT_C(-1);
    }
#endif
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_mcast_set_if(
    apl_handle_t    ai_socket,
    char const*     apc_ifname,
    apl_uint_t      au_ifindex)
{
    switch (sockfd_to_family(ai_socket)) 
    {
#if defined(IP_MULTICAST_IF)
        case APL_AF_INET: 
        {
            struct in_addr          ls_inaddr;
            struct ifreq            ls_ifreq;

            if (au_ifindex > 0) 
            {
                if (if_indextoname(au_ifindex, ls_ifreq.ifr_name) == NULL) 
                {
                    apl_set_errno(APL_ENXIO);  /* i/f index not found */
                    return APL_INT_C(-1);
                }

                goto doioctl;
            } 
            else if (apc_ifname != NULL) 
            {
                apl_strncpy(ls_ifreq.ifr_name, apc_ifname, IFNAMSIZ);
doioctl:
                if (ioctl(ai_socket, SIOCGIFADDR, &ls_ifreq) < 0)
                {
                    return APL_INT_C(-1);
                }

                apl_memcpy(&ls_inaddr,
                    &((struct sockaddr_in *) &ls_ifreq.ifr_addr)->sin_addr,
                    sizeof(struct in_addr));
            } 
            else
            {
                ls_inaddr.s_addr = htonl(INADDR_ANY);      /* remove prev. set default */
            }
            
            return (apl_int_t) setsockopt(
                ai_socket, 
                APL_IPPROTO_IP, 
                IP_MULTICAST_IF,
                &ls_inaddr, 
                sizeof(struct in_addr));
        }
#endif

#if defined(IPV6_MULTICAST_IF)
        case APL_AF_INET6:
        {
            u_int   idx;
            
            if ( (idx = au_ifindex) == 0) 
            {
                if (apc_ifname == NULL) 
                {
                    apl_set_errno(APL_EINVAL); /* must supply either index or name */
                    return APL_INT_C(-1);
                }

                if ( (idx = if_nametoindex(apc_ifname)) == 0) 
                {
                    apl_set_errno(APL_ENXIO);  /* i/f name not found */
                    return APL_INT_C(-1);
                }
                                                                            
            }
            
            return (apl_int_t) setsockopt(
                ai_socket, 
                APL_IPPROTO_IPV6, 
                IPV6_MULTICAST_IF,
                &idx, 
                sizeof(idx));
        }
#endif

        default:
            apl_set_errno(APL_EAFNOSUPPORT);
            return APL_INT_C(-1);
    }
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_mcast_get_if(
    apl_handle_t    ai_socket,
    apl_uint_t*     apu_if)
{
    switch (sockfd_to_family(ai_socket)) 
    {
#if defined(IP_MULTICAST_IF)
        case APL_AF_INET: 
            {
                struct in_addr          ls_inaddr;
                struct ifreq            *lps_ifreq;
                struct ifconf           ls_ifc;
                apl_uint_t              lu_idx;
                apl_socklen_t           lu_len;
                apl_size_t              lu_lastlen;
                char                    *lpc_buf = NULL;
                char                    *lpc_ptr;

                lu_len = sizeof(struct in_addr);

                /* get the interface's address */
                if (getsockopt(ai_socket, 
                            APL_IPPROTO_IP, 
                            IP_MULTICAST_IF, 
                            &ls_inaddr, 
                            (socklen_t*)&lu_len) < 0) 
                {
                    goto doerror;
                }

                /* check if INADDR_ANY or not */
                if (ls_inaddr.s_addr == htonl(INADDR_ANY))
                {
                    *apu_if = 0;
                    return APL_INT_C(0); 
                }

                lu_len = 100 * sizeof(struct ifreq);
                lu_lastlen = 0;

                /* get the list of all interfaces */
                for ( ; ; ) {
                    if ((lpc_buf = apl_malloc(lu_len)) == NULL)
                    {
                        goto doerror;
                    }

                    ls_ifc.ifc_len = lu_len;
                    ls_ifc.ifc_buf = lpc_buf;

                    if (ioctl(ai_socket, SIOCGIFCONF, &ls_ifc) < 0) {
                        if (errno != EINVAL || lu_lastlen != 0)
                        {
                            goto doerror;
                        }
                    } else {
                        if (ls_ifc.ifc_len == lu_lastlen)
                            break;		/* success, len has not changed */
                        lu_lastlen = ls_ifc.ifc_len;
                    }

                    lu_len += 10 * sizeof(struct ifreq);	/* increment */
                    apl_free(lpc_buf);
                }

                /* cmp the addr to find out the related interface*/
                for (lpc_ptr = lpc_buf; lpc_ptr < lpc_buf + ls_ifc.ifc_len; ) 
                {
                    lps_ifreq = (struct ifreq *) lpc_ptr;
#ifdef HAVE_SOCKADDR_SA_LEN 
                    lu_len = max(sizeof(struct sockaddr), lps_ifreq->ifr_addr.sa_len);
                    lpc_ptr += sizeof(lps_ifreq->ifr_name) + lu_len;
#else 
                    lpc_ptr += sizeof(struct ifreq);
#endif

                    if (apl_memcmp(&ls_inaddr, 
                                &(((struct sockaddr_in *) &lps_ifreq->ifr_addr)->sin_addr), 
                                sizeof(struct in_addr)) == 0)
                    {
                        lpc_ptr = NULL;
                        break;
                    }
                }

                if (lpc_ptr != NULL) { goto doerror; }

                lu_idx = if_nametoindex(lps_ifreq->ifr_name);

                if (lu_idx == 0) { goto doerror; }

                *apu_if = lu_idx;
                apl_free(lpc_buf);

                return lu_idx;
doerror:
                if ( lpc_buf != NULL) { apl_free(lpc_buf); }

                return APL_INT_C(-1);
            }
#endif

#if defined(IPV6_MULTICAST_IF) 
        case APL_AF_INET6: 
        {
            u_int           idx;
            apl_socklen_t   len;

            len = sizeof(idx);
            
            if (getsockopt(ai_socket, APL_IPPROTO_IPV6, IPV6_MULTICAST_IF, &idx, (socklen_t*)&len) < 0)
            {
                return APL_INT_C(-1);
            }

            *apu_if = idx;
            return 0;
        }
#endif

        default:
            apl_set_errno(APL_EAFNOSUPPORT);
            return APL_INT_C(-1);
    }
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_mcast_set_loop(
    apl_handle_t    ai_socket,
    apl_uint_t      au_loop)
{
    switch (sockfd_to_family(ai_socket)) 
    {
#if defined(IP_MULTICAST_LOOP)
        case APL_AF_INET: 
        {
            u_char          flag;

            flag = au_loop;
            
            return (apl_int_t) setsockopt(
                ai_socket, 
                APL_IPPROTO_IP, 
                IP_MULTICAST_LOOP,
                &flag, 
                sizeof(flag));
        }
#endif

#if defined(IPV6_MULTICAST_LOOP)
        case APL_AF_INET6: 
        {
            u_int           flag;

            flag = au_loop;
            
            return (apl_int_t) setsockopt(
                ai_socket, 
                APL_IPPROTO_IPV6, 
                IPV6_MULTICAST_LOOP,
                &flag, 
                sizeof(flag));
        }
#endif

        default:
            apl_set_errno(APL_EAFNOSUPPORT);
            return APL_INT_C(-1);
    }
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_mcast_get_loop(
    apl_handle_t    ai_socket,
    apl_uint_t*     apu_loop)
{
    switch (sockfd_to_family(ai_socket)) 
    {
#if defined(IP_MULTICAST_LOOP)
        case APL_AF_INET: 
        {
            u_char          flag;
            apl_socklen_t   len;

            len = sizeof(flag);

            if (getsockopt(ai_socket, APL_IPPROTO_IP, IP_MULTICAST_LOOP, &flag, (socklen_t*)&len) < 0)
            {
                return APL_INT_C(-1);
            }
           
            *apu_loop = flag; 
            return 0;
        }
#endif

#if defined(IPV6_MULTICAST_LOOP)
        case APL_AF_INET6: 
        {
            u_int           flag;
            apl_socklen_t   len;

            len = sizeof(flag);
            
            if (getsockopt(ai_socket, APL_IPPROTO_IPV6, IPV6_MULTICAST_LOOP, &flag, (socklen_t*)&len) < 0)
            {
                return APL_INT_C(-1);
            }
            
            *apu_loop = flag; 
            return 0;
        }
#endif

        default:
            apl_set_errno(APL_EAFNOSUPPORT);
            return APL_INT_C(-1);
    }
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_mcast_set_ttl(
    apl_handle_t    ai_socket,
    apl_uint_t      au_ttl)
{
    switch (sockfd_to_family(ai_socket)) 
    {
#if defined(IP_MULTICAST_TTL)
        case APL_AF_INET: 
        {
            u_char          ttl = au_ttl;
            
            return (apl_int_t) setsockopt(
                ai_socket, 
                APL_IPPROTO_IP, 
                IP_MULTICAST_TTL, 
                &ttl, 
                sizeof(ttl));
        }
#endif

#if defined(IPV6_MULTICAST_HOPS)
        case AF_INET6: 
        {
            int             hop = au_ttl;

            return (apl_int_t) setsockopt(
                ai_socket, 
                APL_IPPROTO_IPV6, 
                IPV6_MULTICAST_HOPS, 
                &hop, 
                sizeof(hop));
        }
#endif

        default:
            apl_set_errno(APL_EAFNOSUPPORT);
            return APL_INT_C(-1);
    }
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_mcast_get_ttl(
    apl_handle_t    ai_socket,
    apl_uint_t*     apu_ttl)
{
    switch (sockfd_to_family(ai_socket)) 
    {
#if defined(IP_MULTICAST_TTL)
        case APL_AF_INET: 
        {
            u_char          ttl;
            apl_socklen_t   len;

            len = sizeof(ttl);
            
            if (getsockopt(ai_socket, APL_IPPROTO_IP, IP_MULTICAST_TTL, &ttl, (socklen_t*)&len) < 0)
            {
                return APL_INT_C(-1);
            }
           
            *apu_ttl = ttl; 
            return 0;
        }
#endif

#if defined(IPV6_MULTICAST_HOPS)
        case APL_AF_INET6: 
        {
            int             hop;
            apl_socklen_t   len;

            len = sizeof(hop);
            
            if (getsockopt(ai_socket, APL_IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &hop, (socklen_t*)&len) < 0)
            {
                return APL_INT_C(-1);
            }
            
            *apu_ttl = hop; 
            return 0;
        }
#endif

        default:
            apl_set_errno(APL_EAFNOSUPPORT);
            return APL_INT_C(-1);
    }
}

/* ---------------------------------------------------------------------- */

