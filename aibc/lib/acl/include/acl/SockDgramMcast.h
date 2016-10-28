/**
 * @file SockDgramMcast.h
 */

#ifndef ACL_SOCKDGRAMMCAST_H
#define ACL_SOCKDGRAMMCAST_H

#include "acl/Utility.h"
#include "acl/TimeValue.h"
#include "acl/SockDgram.h"

ACL_NAMESPACE_START

/**
 * class CSockDgramMcast
 */
class CSockDgramMcast : public CSockDgram
{
public:
    /**
     * Bind option enumeration.
     */
    enum EBindOption
    {
        OPT_BINDADDR_NO  = 0,   ///<Don't bind address.
        
        OPT_BINDADDR_YES        ///<Bind address.
    };
    
public:
    /**
     * @brief A constructor.
     *
     * @param [in] aeOpt    bind option enumeration
     */
    CSockDgramMcast( EBindOption aeOpt = OPT_BINDADDR_NO );
    
    /**
     * @brief A destructor.
     */
    ~CSockDgramMcast(void);
    
    /**
     * @brief Open sock dgram handle and set the default interface name/interface index.
     *
     * @param [in] aoMcastAddr    the specified multicase address
     * @param [in] apcIfname      the interface name
     * @param [in] auIfindex      the interface index
     *
     * @retval 0 Upon successful completion.
     * @retval -1 If an error is detected.
     */
    apl_int_t Open( 
        CSockAddr const& aoMcastAddr,
        char const* apcIfname = NULL,
        apl_uint_t  auIfindex = 0 );
                    
    /**
     * @brief Join in a any-source multicast group.
     *
     * @param [in] aoGrpAddr     the multicast group address
     * @param [in] apcIfname     the interface name
     * @param [in] auIfindex     the interface index
     *
     * @retval 0 Upon successful completion.
     * @retval -1 Join failed.
     */
    apl_int_t Join(
        CSockAddr const& aoGrpAddr,
        char const* apcIfname = NULL,
        apl_uint_t  auIfindex = 0 );
    
    /**
     * @brief Leave a any-source multicast group.
     *
     * @param [in] aoGrpAddr    the multicast group address
     *
     * @retval 0 Upon successful completion.
     * @retval -1 Leave failed.
     */
    apl_int_t Leave( const CSockAddr& aoGrpAddr );
    
    /**
     * @brief Block datagram from a given unicase source to a given multicast group.
     *
     * @param [in] aoSrcAddr    the given unicase source address
     * @param [in] aoGrpAddr    the given multicast group address
     * @param [in] apcIfname    the interface name
     * @param [in] auIfindex    the interface index
     *
     * @retval 0 Upon successful completion.
     * @retval -1 Failed.
     */
    apl_int_t BlockSource( 
        CSockAddr const& aoSrcAddr,
        CSockAddr const& aoGrpAddr,
        char const* apcIfname = NULL,
        apl_uint_t  auIfindex = 0 );
    
    /**
     * @brief Permit receiving data from a given unicase source to a given multicast group.
     *
     * @param [in] aoSrcAddr    the given unicase source address
     * @param [in] aoGrpAddr    the given multicast group address
     *
     * @retval 0 Upon successful completion.
     * @retval -1 Failed.
     */
    apl_int_t UnblockSource( const CSockAddr& aoSrcAddr, const CSockAddr& aoGrpAddr );
    
    /**
     * @brief Join in a source-specific multicast group.
     *
     * @param [in] aoSrcAddr    the given unicase source address 
     * @param [in] aoGrpAddr    the given multicast group address
     * @param [in] apcIfname    the interface name 
     * @param [in] auIfindex    the interface index
     *
     * @retval 0 Upon successful completion.
     * @retval -1 Failed.
     */
    apl_int_t JoinSourceGroup( 
        CSockAddr const& aoSrcAddr,
        CSockAddr const& aoGrpAddr,
        char const* apcIfname = NULL,
        apl_uint_t  auIfindex = 0 );
    
    /**
     * @brief Leave a source-specific multicast group.
     *
     * @param [in] aoSrcAddr    the given unicase source address 
     * @param [in] aoGrpAddr    the given multicast group address
     *
     * @retval 0 Upon successful completion.
     * @retval -1 Failed.                   
     */
    apl_int_t LeaveSourceGroup( CSockAddr const& aoSrcAddr, CSockAddr const& aoGrpAddr );
    
    /**
     * @brief Set the default interface index of the interface of the socket.
     *
     * @param [in] apcIfname    the interface name
     * @param [in] auIfindex    the interface index
     *
     * @retval 0 Upon successful completion.
     * @retval -1 Failed.                   
     */
    apl_int_t SetIf( char const* apcIfname, apl_uint_t  auIfindex = 0 );
    
    /**
     * @brief Get the default interface index of the interface of the socket.
     *
     * @param [out] apuIfindex    pointer to the location that holds the interface index
     *
     * @retval 0 Upon successful completion.
     * @retval -1 Failed.                   
     */
    apl_int_t GetIf( apl_uint_t* apuIfindex );
    
    /**
     * @brief Set the return socket option to 0 or 1.
     *
     * @param [in] auLoop    the flag to be set
     *
     * @retval 0 Upon successful completion.
     * @retval -1 Failed.                   
     */
    apl_int_t SetLoop( apl_uint_t auLoop );
    
    /**
     * @brief Get the return socket flag of the socket.
     *
     * @param [out] apuLoop    pointer to the location that hold the return socket flag
     *
     * @retval 0 Upon successful completion.
     * @retval -1 Failed.                   
     */
    apl_int_t GetLoop( apl_uint_t* apuLoop );
    
    /**
     * @brief Set the TTL value of the socket.
     *
     * @param [in] auTTL    the TTL value which wanted to be set
     *
     * @retval 0 Upon successful completion.
     * @retval -1 Failed.                   
     */
    apl_int_t SetTTL( apl_uint_t auTTL );
    
    /**
     * @brief Get the TTL value of the socket.
     *
     * @param [out] apuTTL    pointer to the location that hold the TTL value
     *
     * @retval 0 Upon successful completion.
     * @retval -1 Failed.                   
     */
    apl_int_t GetTTL( apl_uint_t* apuTTL );
    
    /**
     * @brief Send <aiN> bytes in <apvBuffer>, using the multicast address and network interface.
     *
     * @param [in] apvBuffer    the data to send 
     * @param [in] aiN          the bytes to send      
     * @param [in] aeFlag       the enumeration option
     * @param [in] aoTimeout    timeout       
     * - =CTimeValue :: MAXTIME : never timeout
     * - =CTimeValue :: ZERO : non-blocking    
     * - >0 : blocking                       
     *
     * @return   Upon successful completion, the number of bytes sent shall be returned.
     *           Otherwise, -1 shall be returned.                                       
     */
    apl_ssize_t SendM(
        void const* apvBuffer,
        apl_size_t aiN,
        CSockDgram::EOption aeFlag,
        CTimeValue const& aoTimeout = CTimeValue::ZERO ) const;
                        
    /**
     * @brief Send <aiN> bytes in <apvBuffer>, using the multicast address and network interface
     *        by default option.
     *
     * @param [in] apvBuffer    the data to send 
     * @param [in] aiN          the bytes to send      
     * @param [in] aoTimeout    timeout       
     * - =CTimeValue :: MAXTIME : never timeout
     * - =CTimeValue :: ZERO : non-blocking    
     * - >0 : blocking                       
     *
     * @return    Upon successful completion, the number of bytes sent shall be returned.
     *             Otherwise, -1 shall be returned.                                       
     */
    apl_ssize_t SendM(
        void const* apvBuffer,
        apl_size_t aiN,
        CTimeValue const& aoTimeout = CTimeValue::ZERO ) const;

protected:
    apl_int_t OpenI( CSockAddr const& aoMcastAddr, char const* apcIfname, apl_uint_t  auIfindex );

protected:
    EBindOption meOpt;
    
    CSockAddr moSendAddr;
};

ACL_NAMESPACE_END

#endif//ACL_SOCKDGRAMMCAST_H
