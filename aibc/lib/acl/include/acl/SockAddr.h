/**
 * @file SockAddr.h
 */

#ifndef ACL_SOCKADDR_H
#define ACL_SOCKADDR_H

#include "acl/Utility.h"

ACL_NAMESPACE_START

/**
 * class CSockAddr
 */
class CSockAddr
{
public:

    /**
     * @brief A constructor.
     */
    CSockAddr(void);
    
    /**
     * @brief Copy constructor.
     */
    CSockAddr( CSockAddr const& aoOther );
    
    /**
     * @brief A constructor.
     *
     * @param [in] apoAddr         the socket address
     * @param [in] aiLen           the socket address length
     */
    CSockAddr( apl_sockaddr_t const* apoAddr, apl_size_t aiLen );
    
    /**
     * @brief A constructor.
     *
     * @param [in] aiPort          the port
     * @param [in] apcIpAddr       the Ip address
     * @param [in] aiFamily        the specified communications domain in which a socket is to be created
     */
    CSockAddr( apl_uint16_t aiPort, char const* apcIpAddr = NULL, apl_int_t aiFamily = APL_AF_INET );
    
    /**
     * @brief Sock address copy operator
     *
     * @param [in] aoRhs right handle value
     */
    CSockAddr& operator = ( CSockAddr const& aoRhs );

    /**
     * @brief Check SockAddr is wildcard address or not.
     *
     * @retval ture If the SockAddr is wildcard address.
     * @retval false If the SockAddr is not wildcard address.
     */
    bool IsAny(void) const;
    
    /**
     * @brief Check SockAddr is loopback address or not.
     *
     * @retval ture If the SockAddr is loopback address. 
     * @retval false If the SockAddr is not loopback address.
     */
    bool IsLoopBack(void) const;
    
    /**
     * @brief Set wildcard address.
     *
     * @retval 0 If successful.
     * @retval -1 If failed.
     */
    apl_int_t SetAny(void);
    
    /**
     * @brief Set loopback address.
     *
     * @retval 0 If successful.
     * @retval -1 If failed.
     */
    apl_int_t SetLoopBack(void);
    
    /**
     * @brief Set SockAddr according to CSockAddr object.
     *
     * @retval 0 Upon successful completion.
     */
    apl_int_t Set( CSockAddr const& aoOther );
    
    /**
     * @brief Set SockAddr according to apl_sockaddr_t struct.
     *
     * @param [in] apoAddr       the apl_sockaddr_t struct address
     * @param [in] aiLen         the address length
     *
     * @retval 0 Upon successful completion.
     */
    apl_int_t Set( apl_sockaddr_t const* apoAddr, apl_size_t aiLen );
    
    /**
     * @brief Set SockAddr according to port,Ip address and its family.
     *
     * @param [in] aiPort         the specified port
     * @param [in] apcIpAddr      the Ip address
     * @param [in] aiFamily       the specified communications domain in which a socket is to be created
     *
     * @retval 0 Upon successful completion.
     * @retval -1 On any error.
     */
    apl_int_t Set( apl_uint16_t aiPort, char const* apcIpAddr = NULL, apl_int_t aiFamily = APL_AF_INET );
    
    /**
     * @brief Set SockAddr.
     *
     * @param [in] apcHostName         the specified hostname which wanted to convert to address
     * @param [in] apcSvcName          the specified service name which wanted to convert to port
     * @param [in] aiFamily            the specified communications domain in which a socket is to be created
     * @param [in] aiType              the specified type of socket to be created
     * @param [in] aiProto             the particular protocol to be used with the socket
     *
     * @retval 0 Upon successful completion.
     * @retval -1 If an error is detected.
     */
    apl_int_t Set( 
        char const* apcHostName,
        char const* apcSvcName = NULL,
        apl_int_t aiFamily = APL_AF_INET,
        apl_int_t aiType = 0,
        apl_int_t aiProto = 0 );
    
    /**
     * @brief Set Ip address.
     *
     * @param [in] apcIpAddr         the specified Ip address
     *
     * @retval 0 Upon successful completion.
     * @retval -1 If an error is detected.
     */
    apl_int_t SetIpAddr( char const* apcIpAddr );
    
    /**
     * @brief Set port.
     *
     * @param [in] aiPort        the port
     *
     * @retval 0 Upon successful completion. 
     * @retval -1 If an error is detected.
     */
    apl_int_t SetPort( apl_uint16_t aiPort );
    
    /**
     * @brief Set communications domain.
     *
     * @param [in] aiFamily       the specified communications domain in which a socket is to be created
     * @retval 0 Upon successful completion.
     * @retval -1 If an error is detected.
     */
    apl_int_t SetFamily( apl_int_t aiFamily );
    
    /**
     * @brief Get address in the struct of apl_sockaddr_t.
     *
     * @return    The pointer to the address.
     */
    apl_sockaddr_t* GetAddr(void);
    
    /**
     * @brief Get address in the struct of apl_sockaddr_t.
     *
     * @return    The pointer to the address.
     */
    apl_sockaddr_t const* GetAddr(void) const;
    
    /**
     * @brief Get the actual length of the address.
     *
     * @return    The actual length of the address.
     */
    apl_size_t GetLength(void) const;
    
    /**
     * @brief Get the capacity of the address.
     *
     * @return    The capacity of the address.
     */
    apl_size_t GetCapacity(void) const;
    
    /**
     * @brief Get the communications domain.
     *
     * @return    The communications domain.
     */
    apl_int_t GetFamily(void) const;
    
    /**
     * @brief Get the port.
     *
     * @return Upon successful completion, GetPort() shall return a greater than 0 unsigned short.
     *         Otherwise, 0 shall be returned
     */
    apl_uint16_t GetPort(void) const;
    
    /**
     * @brief Get the port.
     *
     * @return Upon successful completion, GetPort() shall return 0. Otherwise, -1 shall be returned
     */
    apl_int_t GetPort( apl_uint16_t* apu16Port ) const;
    
    /**
     * @brief Get the Ip address to the apcIpAddrBuff.
     *
     * @param [out] apcIpAddrBuff       the output buffer to store the Ip address
     * @param [in]  aiSize              the output buffer size
     *
     * @retval 0 Upon successful completion.
     * @retval -1 If an error is detected.
     */
    apl_int_t GetIpAddr( char* apcIpAddrBuff, apl_size_t aiSize ) const;
    
    /**
     * @brief Get the Ip address.
     *
     * @return Upon successful completion, GetIpAddr() shall return a pointer to the ip string
     *         Otherwise, "" shall be returned
     */
    char const* GetIpAddr(void);
    
    /**
     * @brief Compare two addresses for equality. 
     * The addresses are considered equal if they contain the same IP address and port number
     *
     * @param [in] aoRhs right handle value
     *
     * @return true shall be return if the address is the same, otherwise false shall be return
     */
    bool operator == ( CSockAddr const& aoRhs );

    /**
     * @brief Compare two addresses for inequality
     *
     * @param [in] aoRhs right handle value
     *
     * @return false shall be return if the address is the same, otherwise true shall be return
     */
    bool operator != ( CSockAddr const& aoRhs );

protected:
    void Reset(void);

protected:
    char macIpBuffer[64];
    apl_sockaddr_storage_t moInetAddr;
};

ACL_NAMESPACE_END

#endif//ACL_SOCKADDR_H
