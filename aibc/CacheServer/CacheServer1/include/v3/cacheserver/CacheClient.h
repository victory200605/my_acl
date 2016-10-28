
#ifndef AIBC_CACHECLIENT_H
#define AIBC_CACHECLIENT_H

#include "CacheUtility.h"
#include "apl/apl.h"

namespace acl
{
    class CSockAddr;
    class CTimeValue;
    class CMemoryBlock;
};

AIBC_CACHESERVER_NAMESPACE_START

class CCacheClient
{
public:
    CCacheClient(void);

    ~CCacheClient(void);

    apl_int_t Initialize(void);

    apl_int_t Initialize2(void);

    void Close(void);

    void AddServer(
        acl::CSockAddr const& aoRemoteAddress,
        apl_size_t auConnNum,
        apl_size_t auProcessorPoolSize,
        apl_size_t auHashLower,
        apl_size_t auHashUpper,
        acl::CTimeValue const& aoTimeout,
        apl_size_t auWindSize = 1000 );

    apl_int_t Put( 
        char const* apcKey, 
        apl_size_t auKeyLen, 
        char const* apcValue, 
        apl_size_t auValueLen, 
        acl::CTimeValue const& aoTimeout );

    apl_int_t Put(
        char const* apcKey,
        char const* apcValue,
        apl_size_t auValueLen,
        acl::CTimeValue const& aoTimeout );

    apl_int_t Put(
        char const* apcKey,
        acl::CMemoryBlock const& aoValue,
        acl::CTimeValue const& aoTimeout );

    apl_int_t Get(
        char const* apcKey,
        apl_size_t auKeyLen,
        acl::CMemoryBlock* apoValue,
        acl::CTimeValue const& aoTimeout );
    
    apl_int_t Get(
        char const* apcKey,
        acl::CMemoryBlock* apoValue,
        acl::CTimeValue const& aoTimeout );

    apl_int_t GetTimeout(
        acl::CMemoryBlock* apoKey,
        acl::CMemoryBlock* apoValue,
        acl::CTimeValue const& aoTimeout );

    apl_int_t GetStat(
        apl_int32_t aiServerIdx,
        apl_int32_t& aiCapacity,
        apl_int32_t& aiSize,
        apl_int32_t& aiFileCount,
        acl::CTimeValue const& aoTimeout );

private:
    class CCacheClientImpl* mpoImpl;
};

AIBC_CACHESERVER_NAMESPACE_END

#endif//AIBC_CACHECLIENT_H

