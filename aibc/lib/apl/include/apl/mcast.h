/**
 * @file mcast.h
 * @author  $Author: hezk $
 * @date    $Date: 2009/08/18 09:31:11 $
 * @version $Revision: 1.6 $
 */
#ifndef APL_MCAST_H
#define APL_MCAST_H

#include "aibc_config.h"
#include "apl/sys.h"
#include "apl/inttypes.h"
#include "apl/types.h"
#include "apl/sockaddr.h"

/* -------------------- */ APL_DECLARE_BEGIN /* -------------------- */

/** 
 * This function shall set the socket option that specifies the socket to 
 * join in a any-source multicast.
 *
 * @param[in]   ai_socket   file descriptor that identifies the socket.
 * @param[in]   aps_sagrp   points to a apl_sockaddr_t structure that holds 
 *                          the multicast address.
 * @param[in]   au_sagrplen specifies the length of the apl_sockaddr_t 
 *                          structure pointed by aps_sagrp.
 * @param[in]   apc_ifname  points to a charactor array that holds the name 
 *                          of the interface of the multicast or a null 
 *                          pointer.
 * @param[in]   au_ifindex  specifies the interface index of the interface.
 *
 * @retval 0    success.
 * @retval -1   failure, and errno indicate the error.
 */
apl_int_t apl_mcast_join(
    apl_int_t                       ai_socket,
    struct apl_sockaddr_t const*    aps_sagrp,
    apl_size_t                      au_sagrplen,
    char const*                     apc_ifname,
    apl_uint_t                      au_ifindex);


/** 
 * This function shall set the socket option that specifies the socket to 
 * leave a any-source multicast.
 *
 * @param[in]   ai_socket   file descriptor that identifies the socket.
 * @param[in]   aps_sagrp   points to a apl_sockaddr_t structure that holds 
 *                          the multicast address.
 * @param[in]   au_sagrplen specifies the length of the apl_sockaddr_t
 *                          structure pointed by aps_sagrp.
 * @retval 0    success.
 * @retval -1   failure, and errno indicate the error.
 */
apl_int_t apl_mcast_leave(
    apl_int_t                       ai_socket,
    struct apl_sockaddr_t const*    aps_sagrp,
    apl_size_t                      au_sagrplen) ;

/** 
 * This function shall set the socket option that specifies the socket shall 
 * block data from a given source to a given multicast.
 *
 * @param[in]   ai_socket   file descriptor that identifies the socket.
 * @param[in]   aps_sasrc   points to a apl_sockaddr_t structure that holds 
 *                          the source address.
 * @param[in]   au_sasrclen specifies the length of the apl_sockaddr_t 
 *                          structure pointed by aps_sasrc.
 * @param[in]   aps_sagrp   points to a apl_sockaddr_t structure that holds 
 *                          the multicast address.
 * @param[in]   au_sagrplen specifies the length of the apl_sockaddr_t
 *                          structure pointed by aps_sagrp.
 * @param[in]   apc_ifname  points to a charactor array that holds the name 
 *                          of the interface of the multicast or a null 
 *                          pointer.
 * @param[in]   au_ifindex  specifies the interface index of the interface.
 *
 * @retval 0    success.
 * @retval -1   failure, and errno indicate the error.
 */
apl_int_t apl_mcast_block_source(
    apl_int_t                       ai_socket,
    struct apl_sockaddr_t const*    aps_sasrc,
    apl_size_t                      au_sasrclen,
    struct apl_sockaddr_t const*    aps_sagrp,
    apl_size_t                      au_sagrplen,
    char const*                     apc_ifname,
    apl_uint_t                      au_ifindex);

/** 
 * This function shall set the scoket option that specifies the socket shall 
 * permit receiving data from a given source to a given multicast.
 *
 * @param[in]   ai_socket   file descriptor that identifies the socket.
 * @param[in]   aps_sasrc   points to a apl_sockaddr_t structure that holds 
 *                          the source address.
 * @param[in]   au_sasrclen specifies the length of the apl_sockaddr_t 
 *                          structure pointed by aps_sasrc.
 * @param[in]   aps_sagrp   points to a apl_sockaddr_t structure that holds 
 *                          the multicast address.
 * @param[in]   au_sagrplen specifies the length of the apl_sockaddr_t 
 *                          structure pointed by aps_sagrp.
 *
 * @retval 0    success.
 * @retval -1   failure, and errno indicate the error.
 */
apl_int_t   apl_mcast_unblock_source(
    apl_int_t                       ai_socket, 
    struct apl_sockaddr_t const*    aps_sasrc, 
    apl_size_t                      au_sasrclen,
    struct apl_sockaddr_t const*    aps_sagrp, 
    apl_size_t                      au_sagrplen);

/** 
 * This function shall set the socket option that specifies the socket to 
 * join in a source-specific multicast.
 *
 * @param[in]   ai_socket   file descriptor that identifies the socket.
 * @param[in]   aps_sasrc   points to a apl_sockaddr_t structure that holds 
 *                          the source address.
 * @param[in]   au_sasrclen specifies the length of the apl_sockaddr_t 
 *                          structure pointed by aps_sasrc.
 * @param[in]   aps_sagrp   points to a apl_sockaddr_t structure that holds 
 *                          the multicast address.
 * @param[in]   au_sagrplen specifies the length of the apl_sockaddr_t 
 *                          structure pointed by aps_sagrp.
 * @param[in]   apc_ifname  points to a charactor array that holds the name 
 *                          of the interface of the multicast or a null 
 *                          pointer.
 * @param[in]   au_ifindex  specifies the interface index of the interface.
 * 
 * @retval 0    success.
 * @retval -1   failure, and errno indicate the error.
 */
apl_int_t apl_mcast_join_source_group(
    apl_int_t                       ai_socket,
    struct apl_sockaddr_t const*    aps_sasrc,
    apl_size_t                      au_sasrclen,
    struct apl_sockaddr_t const*    aps_sagrp,
    apl_size_t                      au_sagrplen,
    char const*                     apc_ifname,
    apl_uint_t                      au_ifindex) ;

/** 
 * This function shall set the socket option that specifies the socket to 
 * leave  a source-specific multicast.
 *
 * @param[in]   ai_socket   file descriptor that identifies the socket.
 * @param[in]   aps_sasrc   points to a apl_sockaddr_t structure that holds 
 *                          the source address.
 * @param[in]   au_sasrclen specifies the length of the apl_sockaddr_t 
 *                          structure pointed by aps_sasrc.
 * @param[in]   aps_sagrp   points to a apl_sockaddr_t structure that holds 
 *                          the multicast address.
 * @param[in]   au_sagrplen specifies the length of the apl_sockaddr_t 
 *                          structure pointed by aps_sagrp.
 *
 * @retval 0    success.
 * @retval -1   failure, and errno indicate the error.
 */
apl_int_t   apl_mcast_leave_source_group(
    apl_int_t                       ai_socket, 
    struct apl_sockaddr_t const*    aps_sasrc, 
    apl_size_t                      au_sasrclen,
    struct apl_sockaddr_t const*    aps_sagrp, 
    apl_size_t                      au_sagrplen);

/** 
 * This function shall set the default interface index of the interface of 
 * the socket.
 *
 * @param[in]   ai_socket   file descriptor that identifies the socket.
 * @param[in]   apc_ifname  points to a charactor array that holds the name 
 *                          of the interface of the multicast or a null 
 *                          pointer.
 * @param[in]   au_ifindex  specifies the interface index of the interface.
 *
 * @retval 0    success.
 * @retval -1   failure, and errno indicate the error.
 */
apl_int_t apl_mcast_set_if(
    apl_int_t       ai_socket,
    char const*     apc_ifname,
    apl_uint_t      au_ifindex) ;

/** 
 * This function shall get the default interface index of the interface of 
 * the socket.
 *
 * @param[in]   ai_socket   file descriptor that identifies the socket.
 * @param[out]  apu_if      points to the location that holds the interface 
 *                          index.
 *
 * @retval >0   the interface index.
 * @retval 0    INADDR_ANY.
 * @retval -1   failure, and errno indicate the error.
 */
apl_int_t apl_mcast_get_if(
    apl_int_t       ai_socket,
    apl_uint_t*     apu_if) ;

/** 
 * This function shall set the return socket option to 0 or 1.
 *
 * @param[in]    ai_socket    file descriptor that identifies the socket.
 * @param[in]    au_loop      the flag to be set.
 *
 * @retval 0    success.
 * @retval -1   failure, and errno indicate the error.
 */
apl_int_t apl_mcast_set_loop(
    apl_int_t       ai_socket,
    apl_uint_t      au_loop) ;

/** 
 * This function shall get the return socket flag of the socket.
 *
 * @param[in]    ai_socket  file descriptor that identifies the socket.
 * @param[out]   apu_loop   points to the location that hold the return 
 *                          socket flag.
 *
 * @retval >=0  success, return loop flags.
 * @retval -1   failure, and errno indicate the error.
 */
apl_int_t apl_mcast_get_loop(
    apl_int_t       ai_socket,
    apl_uint_t*     apu_loop) ;

/** 
 * This function shall set the TTL value of the socket.
 *
 * @param[in]    ai_socket    file descriptor that identifies the socket.
 * @param[in]    au_ttl       the TTL value which wanted to be set.
 *
 * @retval 0    success.
 * @retval -1   failure, and errno indicate the error.
 */
apl_int_t apl_mcast_set_ttl(
    apl_int_t       ai_socket,
    apl_uint_t      au_ttl) ;

/** 
 * This function shall get the TTL value of the socket.
 *
 * @param[in]   ai_socket   file descriptor that identifies the socket.
 * @param[in]   apu_ttl     points to the location that hold the TTL value.
 *
 * @retval >=0  success, return the TTL value.
 * @retval -1   failure, and errno indicate the error.
 */
apl_int_t apl_mcast_get_ttl(
    apl_int_t       ai_socket,
    apl_uint_t*     apu_ttl) ;


/* -------------------- */  APL_DECLARE_END  /* -------------------- */

#endif /* APL_MCAST_H */

