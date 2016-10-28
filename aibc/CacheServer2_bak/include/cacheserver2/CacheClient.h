
#ifndef AIBC_CACHECLIENT_H
#define AIBC_CACHECLIENT_H

#include "anf/util/AsyncClient.h"
#include "anf/protocol/SimpleAsyncMessage.h"
#include "anf/filter/SimpleAsyncMessageFilter.h"
#include "acl/TimeValue.h"
#include "acl/Synch.h"
#include "acl/SockAddr.h"
#include "acl/MemoryBlock.h"
#include "acl/stl/vector.h"
#include "acl/stl/string.h"
#include "Utility.h"

AIBC_CACHESERVER_NAMESPACE_START

class CCacheClient
{
    typedef anf::TAsyncClient<anf::CSimpleAsyncMessage> ClientType;

public:
    CCacheClient(void);

    ~CCacheClient(void);

    apl_int_t Initialize(void);

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

private:
    bool CheckHashRangeValue( apl_size_t auLower, apl_size_t auUpper );
    bool CheckAllHashRangeValue( apl_int_t* apiUndefineValue );
    void SetServer( apl_size_t auLower, apl_size_t auUpper, ClientType* apoServer );
    ClientType* GetServer( char const* apcKey, apl_size_t auLen );

private:
    acl::CLock moLock;
    std::vector<ClientType*> moClients;
    std::vector<ClientType*> moClientMap;

    struct CServerInfo
    {
        acl::CSockAddr moRemoteAddress;
        apl_size_t muConnNum;
        apl_size_t muProcessorPoolSize;
        apl_size_t muHashLower;
        apl_size_t muHashUpper;
        apl_size_t muWindSize;
        acl::CTimeValue moTimeout;
    };

    std::vector<CServerInfo> moServerInfos;
};

AIBC_CACHESERVER_NAMESPACE_END

#endif//AIBC_CACHECLIENT_H

