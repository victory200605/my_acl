/**
 * @file sockaddr.h
 * @author  $Author: hezk $
 * @date    $Date: 2010/02/02 03:32:59 $
 * @version $Revision: 1.14 $
 */
#ifndef APL_SOCKADDR_H
#define APL_SOCKADDR_H

#include "aibc_config.h"
#include "apl/inttypes.h"
#include "apl/types.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>


/* ------------------------------ */ APL_DECLARE_BEGIN /* ------------------------------ */


/**
 * define a socket address data structure
 */
#define apl_sockaddr_t          sockaddr

/**
 * declaring storage for automatic variables which is both large enough and aligned enough for
 * storing the socket address data structure of any family
 */
#define apl_sockaddr_storage_t  sockaddr_storage

/**
 * define a socket address data structure used in IPv4 family
 */
#define apl_sockaddr_in_t       sockaddr_in

/**
 * define a socket address data structure used in IPv6 family
 */
#define apl_sockaddr_in6_t      sockaddr_in6

/**
 * define a socket address data structure used to store addresses for UNIX domain sockets
 */
#define apl_sockaddr_un_t       sockaddr_un


/** 
 * Set socket address' family
 *
 * This function shall set the address family to the apl_sockaddr_t structure 
 * pointed by aps_sa.
 *
 * @param[out]    aps_sa       a pointer points to the apl_sockaddr_t structure.
 * @param[in]     ai_family    specifies the address family.
 *
 * @retval 0    successful.
 * @retval -1   fail.
 */
apl_int_t apl_sockaddr_setfamily(
    struct apl_sockaddr_t*  aps_sa,
    apl_int_t               ai_family) ;


/** 
 * Get socket address's family
 *
 * This function shall get the address family to the apl_sockaddr_t structure 
 * pointed by aps_sa.
 *
 * @param[in]   aps_sa      a pointer points to the apl_sockaddr_t structure.
 * @param[out]  api_family  a pointer points to the location that hold the 
 *                          address family.
 * @retval 0    successful.
 * @retval -1   fail.
 */
apl_int_t apl_sockaddr_getfamily(
    struct apl_sockaddr_t const*    aps_sa,
    apl_int_t*                      api_family) ;

/** 
 * Set socket address' IP number.
 *
 * This function shall set the address to the apl_sockaddr_t structure 
 * pointed by aps_sa.
 *
 * @param[out]  aps_sa      a pointer points to the apl_sockaddr_t structure.
 * @param[in]   au_salen    specifies the length of the structure pointed by 
 *                          aps_sa.
 * @param[in]   apc_addr    a pointer points to the charactor array that 
 *                          holds the address.
 * @retval 0    successful.
 * @retval -1   fail.
 */
apl_int_t apl_sockaddr_setaddr(
    struct apl_sockaddr_t*  aps_sa,
    apl_size_t              au_salen,
    char const*             apc_addr) ;

/** 
 * Get socket address' IP number
 *
 * This function shall get the address to the apl_sockaddr_t structure 
 * pointed by aps_sa.
 *
 * @param[in]   aps_sa      a pointer points to the apl_sockaddr_t 
 *                          structure.
 * @param[out]  apc_addr    a pointer points to the location that hold the 
 *                          address.
 * @param[in]   au_addrlen  specifies the length of the charactor pointed by 
 *                          apc_addr.
 *
 * @retval 0    successful.
 * @retval -1   fail.
 */
apl_int_t apl_sockaddr_getaddr(
    struct apl_sockaddr_t const*    aps_sa,
    char*                           apc_addr,
    apl_size_t                      au_addrlen);

/** 
 * Set socket address' port
 *
 * This function shall set the port to the apl_sockaddr_t structure pointed 
 * by aps_sa.
 *
 * @param[out]  aps_sa      a pointer points to the apl_sockaddr_t structure.
 * @param[in]   au_salen    specifies the length of the structure pointed by 
 *                          aps_sa.
 * @param[in]   au16_port   specifies the port wanted to set.
 *
 * @retval 0    successful.
 * @retval -1   fail.
 */
apl_int_t apl_sockaddr_setport(
    struct apl_sockaddr_t*  aps_sa,
    apl_size_t              au_salen,
    apl_uint16_t            au16_port) ;

/** 
 * Get socket address' port number
 *
 * This function shall get the port to the apl_sockaddr_t structure pointed 
 * by aps_sa.
 *
 * @param[in]   aps_sa      a pointer points to the apl_sockaddr_t structure.
 * @param[out]  apu16_port  a pointer points to the location that hold the 
 *                          port.
 *
 * @retval 0    successful.
 * @retval -1   fail.
 */
apl_int_t apl_sockaddr_getport(
    struct apl_sockaddr_t const*    aps_sa,
    apl_uint16_t*                   apu16_port) ;

/** 
 * Set socket address to a wildcard IP.
 *
 * This function shall specify the system will determinate the address and 
 * set to the apl_sockaddr_t structure pointed by aps_sa.
 *
 * @param[out]  aps_sa      a pointer points to the apl_sockaddr_t structure.
 * @param[in]   au_salen    specifies the length of the structure pointed by 
 *                          aps_sa.
 * @retval 0    successful.
 * @retval -1   fail.
 */
apl_int_t apl_sockaddr_setwild(
    struct apl_sockaddr_t*  aps_sa,
    apl_size_t              au_salen) ;


/** 
 * Check if a wildcard IP.
 *
 * This function shall check whether the IPv4/IPv6 address in the 
 * apl_sockaddr_t structure is a wildcard address.
 *
 * @param[out]  aps_sa      a pointer points to the apl_sockaddr_t structure.
 * @param[in]   au_salen    specifies the length of the structure pointed by 
 *                          aps_sa.
 * @retval 1    it is a wildcard address.
 * @retval 0    it is NOT a wildcard address.
 * @retval -1   fail.
 */
apl_int_t apl_sockaddr_checkwild(
    struct apl_sockaddr_t const*    aps_sa,
    apl_size_t                      au_salen);
    

/** 
 * Set socket address to a loopback IP.
 *
 * This function shall set the loopback address to the apl_sockaddr_t 
 * structure pointed by aps_sa.
 *
 * @param[in,out]   aps_sa      a pointer points to the apl_sockaddr_t 
 *                              structure.
 * @param[in]       au_salen    specifies the length of the structure pointed 
 *                              by aps_sa.
 *
 * @retval 0    successful.
 * @retval -1   fail, errno indicate the error.
 */
apl_int_t apl_sockaddr_setloopback(
    struct apl_sockaddr_t*  aps_sa,
    apl_size_t              au_salen);


/** 
 * Check if a loopback IP.
 *
 * This function shall check whether the IPv4/IPv6 address in the 
 * apl_sockaddr_t structure is a loopback address.
 *
 * @param[in]   aps_sa      a pointer points to the apl_sockaddr_t structure.
 * @param[in]   au_salen    specifies the length of the structure pointed by 
 *                          aps_sa.
 *
 * @retval 1    it is a loopback address.
 * @retval 0    it is NOT a loopback address.
 * @retval -1   fail.
 */
apl_int_t apl_sockaddr_checkloopback(
    struct apl_sockaddr_t const*    aps_sa,
    apl_size_t                      au_salen);


/** 
 * Get the available length of a socket address.
 *
 * This function shall get the actually used length of the apl_sockaddr_t 
 * structure.
 *
 * @param[out]  aps_sa  a pointer points to the apl_sockaddr_t structure.
 *
 * @retval >=0  successful, the value is the length of the structure that 
 *              used to store the address.
 * @retval -1   fail.errno indicate the error.
 */
apl_ssize_t apl_sockaddr_getlen(
    struct apl_sockaddr_t const*    aps_sa);


/** 
 * Network name resulve.
 *
 * This function shall provide the conversion of name  to address and service 
 * to port.
 *
 * @param[in]   ai_family       indicate the family of address which shall 
 *                              convert according by apc_hostname.
 * @param[in]   ai_socktype     indicate the socket type.
 * @param[in]   ai_protocol     indicate the protocol type.
 * @param[in]   apc_hostname    a pointer points to the hostname which wanted 
 *                              to convert to address.
 * @param[in]   apc_svcname     a pointer points to the service name which 
 *                              wanted to convert to port.
 * @param[out]  apps_sas        a pointer points to the location that holds 
 *                              the pointers point to the result address.
 * @param[in]   au_saslen       specifies the length of the buffer pointed by 
 *                              au_saslen.
 *
 * @retval >=0  successful,the return value is the number of address that 
 *              return.
 * @retval -1   fail, errno indicate the error.
 */
apl_int_t apl_sockaddr_resolve(
    apl_int_t               ai_family,
    apl_int_t               ai_socktype,
    apl_int_t               ai_protocol,
    char const*             apc_hostname,
    char const*             apc_svcname,
    struct apl_sockaddr_t** apps_sas,
    apl_size_t              au_saslen);


/** 
 * Get the list of host IP addresses.
 *
 * This function shall get the list of host IP addresses.
 *
 * @param[out]  appcIPAddrList String array buffer to hold the IP addresses list in 
 *                             c string format.
 * @param[in]   au_addr_len    Size of each buffer in appcIPAddrList. Usually it is 
 *                             set to INET_ADDRSTRLEN in INET, and set to 
 *                             INET6_ADDRSTRLEN in INET6.
 * @param[in]   au_count       How many buffers in appcIPAddrList.
 *
 * @retval >=0   the counts of IP addresses copied to appcIPAddrList.
 * @retval -1    fail.
 */
apl_int_t apl_get_hostaddr_list(
        char** appcIPAddrList, 
        apl_size_t au_addr_len,  
        apl_size_t au_count) ;

/* ------------------------------ */  APL_DECLARE_END  /* ------------------------------ */

#endif /* APL_SOCKADDR_H */

