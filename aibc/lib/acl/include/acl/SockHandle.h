/**
 * @file SockHandle.h
 */

#ifndef ACL_SOCKHANDLE_H
#define ACL_SOCKHANDLE_H

#include "acl/Utility.h"
#include "acl/IOHandle.h"
#include "acl/SockAddr.h"

ACL_NAMESPACE_START

/**
 * class CSockHandle
 */
class CSockHandle : public CIOHandle
{
public:
    /**
     * Int value option enumeration
     */
    enum EIOption //Int <value> option
    {
        /**
         * \<getsockopt/setsockopt\> system call with SOL_SOCKET and SO_BROADCAST 
         */
        OPT_BROADCAST = 1,
        
        /**
         * \<getsockopt/setsockopt\> system call with SOL_SOCKET and SO_DEBUG
         */
        OPT_DEBUG,
        
        /**
         * \<getsockopt/setsockopt\> system call with SOL_SOCKET and SO_DONTROUTE
         */
        OPT_DONTROUTE,
        
        /**
         * \<getsockopt/setsockopt\> system call with SOL_SOCKET and SO_KEEPALIVE
         */
        OPT_KEEPALIVE,
        
        /**
         * \<getsockopt/setsockopt\> system call with SOL_SOCKET and SO_RCVBUF
         */
        OPT_RCVBUF,
        
        /**
         * \<getsockopt/setsockopt\> system call with SOL_SOCKET and SO_SNDBUF
         */
        OPT_SNDBUF,
        
        /**
         * \<getsockopt/setsockopt\> system call with SOL_SOCKET and SO_REUSEADDR
         */
        OPT_REUSEADDR,
        
        /**
         * \<getsockopt/setsockopt\> system call with SOL_SOCKET and SO_TCPNODELAY
         */
        OPT_TCPNODELAY
    };
    
    /**
     * Linger option enumeration
     */
    enum EOptionLinger
    {
        /**
         * \<getsockopt/setsockopt\> system call with SOL_SOCKET and SO_LINGER
         */
        OPT_LINGER,
    };
    
    /**
     * Int value and get only option enumeration
     */
    enum EIGOOption //Int <value> and get only option
    {
        /**
         * \<getsockopt\> system call with SOL_SOCKET and SO_ERROR
         */
        OPT_ERROR = 1, 
        
        /**
         * \<getsockopt\> system call with SOL_SOCKET and SO_TYPE
         */
        OPT_TYPE,
        
        /**
         * \<getsockopt\> system call with SOL_SOCKET and SO_ACCEPTCONN
         */
        OPT_ACCEPTCONN
    };
    
public:
    /**
     * @brief Open socket handle.
     *
     * @param [in] aiFamily         the specified communications domain in which a socket is to be created  
     * @param [in] aiType           the specified type of socket to be created 
     * @param [in] aiProtocol       the particular protocol to be used with the socket
     *
     * @retval 0 Upon successful completion.
     * @retval -1 If an error is detected.
     */
    apl_int_t Open( apl_int_t aiFamily, apl_int_t aiType, apl_int_t aiProtocol = 0 );

    /**
     * @brief Get local address from socket handle.
     *
     * @param [out] aoAddr    the CSockAddr object to store local address
     *
     * @retval 0 Upon successful completion.
     * @retval -1 If an error is detected.
     */
    apl_int_t GetLocalAddr( CSockAddr& aoAddr ) const;

    /**
     * @brief Get remote address from socket handle.
     *
     * @param [out] aoAddr   the CSockAddr object to store remote address
     *
     * @retval 0 Upon successful completion.
     * @retval -1 If an error is detected. 
     */
    apl_int_t GetRemoteAddr( CSockAddr& aoAddr ) const;
    
    /**
     * @brief Get Int value option.
     *
     * @param [in]  aeOpt        Int value option
     * @param [out] apiOptVal    the output int value
     *
     * @retval 0 Upon successful completion.
     * @retval -1 If an error is detected.
     */
    apl_int_t GetOption( EIOption aeOpt, apl_int_t* apiOptVal ) const;

    /**
     * @brief Set Int value option.
     *
     * @param [in] aeOpt       Int value option
     * @param [in] aiOptVal    the input int value
     *
     * @retval 0 Upon successful completion.
     * @retval -1 If an error is detected.
     */
    apl_int_t SetOption( EIOption aeOpt, apl_int_t aiOptVal ) const;

    /**
     * @brief Get Int value and get only option.
     *
     * @param [in]  aeOpt        the Int value and get only option
     * @param [out] apiOptVal    the output Int value and get only option
     *
     * @retval 0 Upon successful completion.
     * @retval -1 If an error is detected.
     */
    apl_int_t GetOption( EIGOOption aeOpt, apl_int_t* apiOptVal ) const;
    
    /**
     * @brief Get linger option.
     *
     * @param [in]  aeOpt      the linger option
     * @param [out] apiOnoff   the flag to indicate whether linger option is enabled
     * @param [out] apiLonger  the linger time, in seconds
     *
     * @retval 0 Upon successful completion.
     * @retval -1 If an error is detected.
     */
    apl_int_t GetOption( EOptionLinger aeOpt, apl_int_t* apiOnoff, apl_int_t* apiLonger ) const;

    /**
     * @brief Set linger option.
     *
     * @param [in] aeOpt       the linger option                                        
     * @param [in] apiOnoff    the flag to indicate whether linger option is enabled
     * @param [in] apiLonger   the linger time, in seconds                         
     *
     * @retval 0 Upon successful completion.
     * @retval -1 If an error is detected.
     */
    apl_int_t SetOption( EOptionLinger aeOpt, apl_int_t apiOnoff, apl_int_t apiLonger ) const;
};

//////////////////////////////////////////// SockHandlePair ///////////////////////////////////
/**
 * class CSockHandlePair
 */
class CSockHandlePair
{
public:
    /**
     * @brief Open connected sockets pair.
     *
     * @param [in] aiFamily     the specified communications domain in which the sockets are to be created
     * @param [in] aiType       the type of sockets to be created
     * @param [in] aiProtocol   a particular protocol to be used with the sockets
     *
     * @retval 0 Upon successful completion.
     * @retval -1 If an error is detected.
     */
    apl_int_t Open( apl_int_t aiFamily, apl_int_t aiType, apl_int_t aiProtocol = 0 );
    
    /**
     * @brief Get the first socket handle.
     *
     * @return    The first socket handle.
     */
    CSockHandle& GetFirst(void);
    
    /**
     * @brief Get the second socket handle.
     *
     * @return    The second socket handle.
     */
    CSockHandle& GetSecond(void);

private:
    CSockHandle maoHandle[2];
};

ACL_NAMESPACE_END

#endif//ACL_SOCKHANDLE_H
