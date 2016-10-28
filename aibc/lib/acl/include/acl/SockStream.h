/**
 * @file SockStream.h
 */

#ifndef ACL_SOCKSTREAM_H
#define ACL_SOCKSTREAM_H

#include "acl/Utility.h"
#include "acl/SockHandle.h"
#include "acl/MemoryBlock.h"
#include "acl/TimeValue.h"

ACL_NAMESPACE_START

/**
 * class CSockStream
 */
class CSockStream : public CSockHandle
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
     * @brief Receive aiN bytes to apvBuffer from connected socket.
     *
     * @param [out]   apvBuffer    the buffer to store the message
     * @param [in]    aiN          the bytes to receive
     * @param [in]    aeFlag       the enumeration option
     * @retval >0    Successful, the length of the message in bytes shall be returned.
     * @retval 0     No messages are available to be received and the peer has performed an orderly shutdown.
     * @retval -1    An error is detected. 
     */
    apl_ssize_t Recv( 
        void* apvBuffer, 
        apl_size_t aiN,
        EOption aeFlag ) const;
    
    /**
     * @brief Receive aiN bytes to apvBuffer from connected socket by default option.
     *
     * @param [out]    apvBuffer    the buffer to store the message
     * @param [in]     aiN          the bytes to receive
     * @retval >0    Successful, the length of the message in bytes shall be returned.
     * @retval 0     No messages are available to be received and the peer has performed an orderly shutdown.
     * @retval -1    An error is detected. 
     */
    apl_ssize_t Recv(
        void* apvBuffer, 
        apl_size_t aiN ) const;
    
    /**
     * @brief Read aiN bytes to apvBuffer from connected socket by default option.
     *
     * @param [out]    apvBuffer    the buffer to store the message
     * @param [in]     aiN          the bytes to receive
     * @retval >0    Successful, the length of the message in bytes shall be returned.
     * @retval 0     No messages are available to be received and the peer has performed an orderly shutdown.
     * @retval -1    An error is detected. 
     */
    apl_ssize_t Read(
        void* apvBuffer, 
        apl_size_t aiN ) const;

    /**
     * @brief Receive data and place the input data into the aiN buffers specified by the members of the apoIoVec array.
     *
     * @param [out]    apoIoVec    the array to place the input data
     * @param [in]     aiN         the array size
     *
     * @retval >0    Successful, the number of bytes actually read shall be returned.
     * @retval -1    An error is detected.
     */
    apl_ssize_t RecvV(
        apl_iovec_t apoIoVec[], 
        apl_size_t aiN ) const;
        
    /**
     * @brief Receive aiN bytes to apvBuffer from connected socket.
     *
     * @param [out]    apvBuffer    the buffer to store the message
     * @param [in]     aiN          the bytes to receive
     * @param [in]     aeFlag       the enumeration option
     * @param [in]     aoTimeout    timeout
     * - =CTimeValue::MAXTIME : never timeout
     * - =CTimeValue::ZERO : non-blocking
     * - >0 : blocking
     *
     * @retval >0    Successful, the number of bytes actually read shall be returned.
     * @retval -1    An error is detected.
     */
    apl_ssize_t Recv( 
        void* apvBuffer, 
        apl_size_t aiN,
        EOption aeFlag,
        CTimeValue const& aoTimeout ) const;
    
    /**
     * @brief Receive aiN bytes to apvBuffer from connected socket by default option.
     *
     * @param [out]    apvBuffer    the buffer to store the message
     * @param [in]     aiN          the bytes to receive
     * @param [in]     aoTimeout    timeout
     * - =CTimeValue::MAXTIME : never timeout
     * - =CTimeValue::ZERO : non-blocking
     * - >0 : blocking
     *
     * @retval >0    Successful, the number of bytes actually recv shall be returned.
     * @retval -1    An error is detected.
     */
    apl_ssize_t Recv(
        void* apvBuffer, 
        apl_size_t aiN, 
        CTimeValue const& aoTimeout ) const;
    
    /**
     * @brief Receive data and place the input data into the aiN buffers specified by the members of the apoIoVec array.
     *
     * @param [out]    apoIoVec    the array to place the input data
     * @param [in]     aiN         the array size
     * @param [in]     aoTimeout   timeout
     * - =CTimeValue::MAXTIME : never timeout
     * - =CTimeValue::ZERO : non-blocking
     * - >0 : blocking
     *
     * @retval >0    Successful, the number of bytes actually recv shall be returned.
     * @retval -1    An error is detected.
     */
    apl_ssize_t RecvV(
        apl_iovec_t apoIoVec[], 
        apl_size_t aiN, 
        CTimeValue const& aoTimeout ) const;
    
    /**
     * @brief Receive data until the first occurrence of apcTag.
     *
     * @param [out]    apvBuffer    the buffer to store the message
     * @param [in]     aiN          the bytes to receive
     * @param [in]     apcTag       the tag string
     * @param [in]     aeFlag       the enumeration option
     * @param [in]     aoTimeout    timeout
     * - =CTimeValue::MAXTIME : never timeout
     * - =CTimeValue::ZERO : non-blocking
     * - >0 : blocking
     *
     * @retval >0    Successful, the number of bytes actually receive shall be returned.
     * @retval -1    An error is detected.
     */
    apl_ssize_t RecvUntil(
        void* apvBuffer, 
        apl_size_t aiN,
        char const* apcTag,
        EOption aeFlag,
        CTimeValue const& aoTimeout ) const;
    
    /**
     * @brief Receive data until the first occurrence of apcTag by default option.
     *
     * @param [out]    apvBuffer    the buffer to store the message
     * @param [in]     aiN          the bytes to receive
     * @param [in]     apcTag       the tag string
     * @param [in]     aoTimeout    timeout
     * - =CTimeValue::MAXTIME : never timeout
     * - =CTimeValue::ZERO : non-blocking
     * - >0 : blocking
     *
     * @retval >0    Successful, the number of bytes actually receive shall be returned.
     * @retval -1    An error is detected.
     */
    apl_ssize_t RecvUntil(
        void* apvBuffer,
        apl_size_t aiN,
        char const* apcTag,
        CTimeValue const& aoTimeout ) const;

    /**
     * @brief Send a message on a socket.
     *
     * @param [in]    apvBuffer    the data to send
     * @param [in]    aiN          the bytes to send
     * @param [in]    aeFlag       the enumeration option
     *
     * @retval >0    Successful, the number of bytes sent shall be returned.
     * @retval -1    An error is detected.
     */
    apl_ssize_t Send(
        void const* apvBuffer, 
        apl_size_t aiN,
        EOption aeFlag ) const;
    
    /**
     * @brief Send a message on a socket by default option.
     *
     * @param [in]    apvBuffer    the data to send
     * @param [in]    aiN          the bytes to send
     *
     * @retval >0    Successful, the number of bytes sent shall be returned.
     * @retval -1    An error is detected.
     */
    apl_ssize_t Send(
        void const* apvBuffer, 
        apl_size_t aiN ) const;
    
    /**
     * @brief Send a message on a socket by default option.
     *
     * @param [in]    apvBuffer    the data to send
     * @param [in]    aiN          the bytes to send
     *
     * @retval >0    Successful, the number of bytes sent shall be returned.
     * @retval -1    An error is detected.
     */
    apl_ssize_t Write(
        void const* apvBuffer, 
        apl_size_t aiN ) const;

    /**
     * @brief Gather output data from the aiN buffers specified by the members of the apoIoVec array and send it.
     *
     * @param [in]    apoIoVec    the array to place output data.
     * @param [in]    aiN         the array size
     *
     * @retval >0    Successful, the number of bytes sent shall be returned.
     * @retval -1    An error is detected.
     */
    apl_ssize_t SendV(
        apl_iovec_t const apoIoVec[], 
        apl_size_t aiN ) const;
        
    /**
     * @brief Send a message on a socket.
     *
     * @param [in]    apvBuffer    the data to send
     * @param [in]    aiN          the bytes to send
     * @param [in]    aeFlag       the enumeration option
     * @param [in]    aoTimeout    timeout
     * - =CTimeValue::MAXTIME : never timeout
     * - =CTimeValue::ZERO : non-blocking
     * - >0 : blocking
     *
     * @retval >0    Successful, the number of bytes sent shall be returned.
     * @retval -1    An error is detected.
     */
    apl_ssize_t Send(
        void const* apvBuffer, 
        apl_size_t aiN,
        EOption aeFlag,
        CTimeValue const& aoTimeout ) const;
    
    /**
     * @brief Send a message on a socket by default option.
     *
     * @param [in]    apvBuffer    the data to send
     * @param [in]    aiN          the bytes to send
     * @param [in]    aoTimeout    timeout
     * - =CTimeValue::MAXTIME : never timeout
     * - =CTimeValue::ZERO : non-blocking
     * - >0 : blocking
     *
     * @retval >0    Successful, the number of bytes sent shall be returned.
     * @retval -1    An error is detected.
     */
    apl_ssize_t Send(
        void const* apvBuffer, 
        apl_size_t aiN, 
        CTimeValue const& aoTimeout ) const;
    
    /**
     * @brief Send all the memory_block's chained through their cont pointers.
     *
     * @param [in]    apoMemoryBlock    memory_block pointer.
     * @param [in]    aoTimeout   timeout
     * - =CTimeValue::MAXTIME : never timeout
     * - =CTimeValue::ZERO : non-blocking
     * - >0 : blocking
     *
     * @retval >0    Successful, the number of bytes sent shall be returned.
     * @retval -1    An error is detected.
     */
    apl_ssize_t SendV(
        CMemoryBlock* apoMemoryBlock,
        CTimeValue const& aoTimeout ) const;

    /**
     * @brief Gather output data from the aiN buffers specified by the members of the poIoVec array and send it.
     *
     * @param [in]    apoIoVec    the array to place output data.
     * @param [in]    aiN         the array size
     * @param [in]    aoTimeout   timeout
     * - =CTimeValue::MAXTIME : never timeout
     * - =CTimeValue::ZERO : non-blocking
     * - >0 : blocking
     *
     * @retval >0    Successful, the number of bytes sent shall be returned.
     * @retval -1    An error is detected.
     */
    apl_ssize_t SendV(
        apl_iovec_t const apoIoVec[], 
        apl_size_t aiN, 
        CTimeValue const& aoTimeout ) const;
};

ACL_NAMESPACE_END

#endif//ACL_SOCKSTREAM_H
