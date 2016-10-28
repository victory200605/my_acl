/**
 * @file SockDgram.h
 */

#ifndef ACL_SOCKDGRAM_H
#define ACL_SOCKDGRAM_H

#include "acl/Utility.h"
#include "acl/SockHandle.h"
#include "acl/TimeValue.h"

ACL_NAMESPACE_START

/**
 * class CSockDgram
 */
class CSockDgram : public CSockHandle
{
public:
    /**
     * Enumeration option.
     */
    enum EOption
    {
        OPT_UNSPEC = 0,              ///<default unspecial option
        OPT_PEEK   = APL_MSG_PEEK,   ///<Peeks at an incoming message. The data is treated as unread and the next recv() or similar function shall still return this data. 
        OPT_OOB    = APL_MSG_OOB     ///<Requests out-of-band data.
    };
    
public:
    /**
     * @brief Open socket.
     *
     * @param [in] aiFamily     the specified communications domain in which a socket is to be created 
     * @param [in] aiProtocol   the particular protocol to be used with the socket
     *
     * @retval 0 Upon successful completion.
     * @retval -1 If an error is detected.
     */
    apl_int_t Open(
        apl_int_t aiFamily = APL_AF_INET,
        apl_int_t aiProtocol = 0 );
        
    /**
     * @brief Open and bind socket.
     *
     * @param [in] aoLocal      local address
     * @param [in] aiFamily     the specified communications domain in which a socket is to be created 
     * @param [in] aiProtocol   the particular protocol to be used with the socket
     *
     * @retval 0 Upon successful completion.
     * @retval -1 If an error is detected.
     */
    apl_int_t Open(
        CSockAddr const& aoLocal,
        apl_int_t aiFamily = APL_AF_INET,
        apl_int_t aiProtocol = 0 );
    
    /**
     * @brief Receive aiN bytes to apvBuffer from connected socket.
     *
     * @param [out] apvBuffer    the buffer to store the message
     * @param [in]  aiN          the bytes to receive
     * @param [out] apoAddr      the object to store the sending address
     * @param [in]  aeFlag       the enumeration option
     * 
     * @return     Upon successful completion, the length of the message in bytes shall be returned.
     *             If no messages are available to be received and the peer has performed an orderly shutdown, 0 shall be returned. 
     *             If an error is detected, -1 shall be returned. 
     */
    apl_ssize_t Recv( 
        void* apvBuffer, 
        apl_size_t aiN,
        CSockAddr* apoAddr,
        EOption aeFlag ) const;
    
    /**
     * @brief Receive aiN bytes to apvBuffer from connected socket by default option.
     *
     * @param [out] apvBuffer    the buffer to store the message
     * @param [in]  aiN          the bytes to receive
     * @param [out] apoAddr      the object to store the sending address
     *
     * @return     Upon successful completion, the length of the message in bytes shall be returned.
     *             If no messages are available to be received and the peer has performed an orderly shutdown, 0 shall be returned. 
     *             If an error is detected, -1 shall be returned. 
     */
    apl_ssize_t Recv(
        void* apvBuffer, 
        apl_size_t aiN,
        CSockAddr* apoAddr ) const;
                    
    /**
     * @brief Receive aiN bytes to apvBuffer from connected socket.
     *
     * @param [out] apvBuffer    the buffer to store the message
     * @param [in]  aiN          the bytes to receive
     * @param [out] apoAddr      the object to store the sending address
     * @param [in]  aeFlag       the enumeration option
     * @param [in]  aoTimeout    timeout       
     * - =CTimeValue::MAXTIME : never timeout
     * - =CTimeValue::ZERO : non-blocking    
     * - >0 : blocking                       
     *
     * @return     Upon successful completion, the length of the message in bytes shall be returned.
     *             If no messages are available to be received and the peer has performed an orderly shutdown, 0 shall be returned. 
     *             If an error is detected, -1 shall be returned. 
     */
    apl_ssize_t Recv( 
        void* apvBuffer, 
        apl_size_t aiN,
        CSockAddr* apoAddr,
        EOption aeFlag,
        CTimeValue const& aoTimeout ) const;
    
    /**
     * @brief Receive aiN bytes to apvBuffer from connected socket by default option.
     *
     * @param [out] apvBuffer    the buffer to store the message
     * @param [in]  aiN          the bytes to receive
     * @param [out] apoAddr      the object to store the sending address
     * @param [in]  aoTimeout    timeout       
     * - =CTimeValue::MAXTIME : never timeout
     * - =CTimeValue::ZERO : non-blocking    
     * - >0 : blocking                       
     *
     * @return     Upon successful completion, the length of the message in bytes shall be returned.
     *             If no messages are available to be received and the peer has performed an orderly shutdown, 0 shall be returned. 
     *             If an error is detected, -1 shall be returned. 
     */
    apl_ssize_t Recv(
        void* apvBuffer, 
        apl_size_t aiN,
        CSockAddr* apoAddr,
        CTimeValue const& aoTimeout ) const;

    /**
     * @brief Receive data until the first occurrence of apcTag.
     *
     * @param [out] apvBuffer    the buffer to store the message
     * @param [in]  aiN          the bytes to receive 
     * @param [out] apoAddr      the object to store the sending address
     * @param [in]  apcTag       the tag string 
     * @param [in]  aeFlag       the enumeration option
     * @param [in]  aoTimeout    timeout       
     * - =CTimeValue::MAXTIME : never timeout
     * - =CTimeValue::ZERO : non-blocking    
     * - >0 : blocking                       
     * 
     * @return     Upon successful completion, the number of bytes actually receive shall be returned.
     *             Otherwise, -1 shall be returned.
     */
    apl_ssize_t RecvUntil(
        void* apvBuffer, 
        apl_size_t aiN,
        CSockAddr* apoAddr,
        char const* apcTag,
        EOption aeFlag,
        CTimeValue const& aoTimeout ) const;
    
    /**
     * @brief Receive data until the first occurrence of apcTag by default option.
     *
     * @param [out] apvBuffer    the buffer to store the message
     * @param [in]  aiN          the bytes to receive 
     * @param [out] apoAddr      the object to store the sending address
     * @param [in]  apcTag       the tag string 
     * @param [in]  aoTimeout    timeout       
     * - =CTimeValue::MAXTIME : never timeout
     * - =CTimeValue::ZERO : non-blocking    
     * - >0 : blocking                       
     * 
     * @return     Upon successful completion, the number of bytes actually receive shall be returned.
     *             Otherwise, -1 shall be returned.
     */
    apl_ssize_t RecvUntil(
        void* apvBuffer, 
        apl_size_t aiN,
        CSockAddr* apoAddr,
        char const* apcTag, 
        CTimeValue const& aoTimeout ) const;
    
    /**
     * @brief Send a message on a socket.
     *
     * @param [in] apvBuffer    the data to send 
     * @param [in] aiN          the bytes to send
     * @param [in] aoAddr       the receiving address 
     * @param [in] aeFlag       the enumeration option
     *
     * @return     Upon successful completion, the number of bytes sent shall be returned.
     *             Otherwise, -1 shall be returned.
     */
    apl_ssize_t Send(
        void const* apvBuffer, 
        apl_size_t aiN,
        CSockAddr const& aoAddr,
        EOption aeFlag ) const;
    
    /**
     * @brief Send a message on a socket by default option.
     *
     * @param [in] apvBuffer    the data to send 
     * @param [in] aiN          the bytes to send
     * @param [in] aoAddr       the receiving address 
     *
     * @return     Upon successful completion, the number of bytes sent shall be returned.
     *             Otherwise, -1 shall be returned.
     */
    apl_ssize_t Send(
        void const* apvBuffer, 
        apl_size_t aiN,
        CSockAddr const& aoAddr ) const;
        
    /**
     * @brief Send a message on a socket.
     *
     * @param [in] apvBuffer    the data to send 
     * @param [in] aiN          the bytes to send
     * @param [in] aoAddr       the receiving address 
     * @param [in] aeFlag       the enumeration option
     * @param [in] aoTimeout    timeout       
     * - =CTimeValue::MAXTIME : never timeout
     * - =CTimeValue::ZERO : non-blocking    
     * - >0 : blocking                       
     *
     * @return     Upon successful completion, the number of bytes sent shall be returned.
     *             Otherwise, -1 shall be returned.
     */
    apl_ssize_t Send(
        void const* apvBuffer, 
        apl_size_t aiN,
        CSockAddr const& aoAddr,
        EOption aeFlag,
        CTimeValue const& aoTimeout ) const;
    
    /**
     * @brief Send a message on a socket by default option.
     *
     * @param [in] apvBuffer    the data to send 
     * @param [in] aiN          the bytes to send
     * @param [in] aoAddr       the receiving address 
     * @param [in] aoTimeout    timeout       
     * - =CTimeValue::MAXTIME : never timeout
     * - =CTimeValue::ZERO : non-blocking    
     * - >0 : blocking                       
     *
     * @return     Upon successful completion, the number of bytes sent shall be returned.
     *             Otherwise, -1 shall be returned.
     */
    apl_ssize_t Send(
        void const* apvBuffer, 
        apl_size_t aiN,
        CSockAddr const& aoAddr,
        CTimeValue const& aoTimeout ) const;

private:
    apl_int_t GetRemoteAddr( CSockAddr& aoAddr ) const;
};

ACL_NAMESPACE_END

#endif//ACL_SOCKDGRAM_H
