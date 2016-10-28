
#ifndef ACL_SOCKSTREAM_H
#define ACL_SOCKSTREAM_H

#include "Utility.h"
#include "SockHandle.h"
#include "TimeValue.h"

ACL_NAMESPACE_START

class CSockStream : public CSockHandle
{
public:
    enum EOption
    {
        OPT_UNSPEC = 0,
        OPT_PEEK   = APL_MSG_PEEK,
        OPT_OOB    = APL_MSG_OOB
    };
    
public:
    apl_ssize_t Recv( 
        void* apvBuffer, 
        apl_size_t aiN,
        EOption aeFlag,
        CTimeValue const& aoTimeout = CTimeValue::ZERO ) const;
    
    apl_ssize_t Recv(
        void* apvBuffer, 
        apl_size_t aiN, 
        CTimeValue const& aoTimeout = CTimeValue::ZERO ) const;

    apl_ssize_t RecvV(
        apl_iovec_t apoIoVec[], 
        apl_size_t aiN, 
        CTimeValue const& aoTimeout = CTimeValue::ZERO ) const;
    
    apl_ssize_t RecvUntil(
        char const* apcTag,
        void* apvBuffer, 
        apl_size_t aiN,
        EOption aeFlag,
        CTimeValue const& aoTimeout = CTimeValue::ZERO ) const;
    
    apl_ssize_t RecvUntil(
        char const* apcTag, 
        void* apvBuffer, 
        apl_size_t aiN,
        CTimeValue const& aoTimeout = CTimeValue::ZERO ) const;

    apl_ssize_t Send(
        void const* apvBuffer, 
        apl_size_t aiN,
        EOption aeFlag,
        CTimeValue const& aoTimeout = CTimeValue::ZERO ) const;
    
    apl_ssize_t Send(
        void const* apvBuffer, 
        apl_size_t aiN, 
        CTimeValue const& aoTimeout = CTimeValue::ZERO ) const;

    //apl_ssize_t SendV(
    //    CMemBlock* apoMemBlock,
    //    CTimeValue const& aoTimeout = CTimeValue::ZERO ) const;

    apl_ssize_t SendV(
        apl_iovec_t const apoIoVec[], 
        apl_size_t aiN, 
        CTimeValue const& aoTimeout = CTimeValue::ZERO ) const;
};

ACL_NAMESPACE_END

#endif//ACL_SOCKSTREAM_H
