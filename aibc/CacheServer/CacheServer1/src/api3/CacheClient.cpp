
#include "cacheserver/CacheClient.h"
#include "CacheClientImpl.h"

AIBC_CACHESERVER_NAMESPACE_START

CCacheClient::CCacheClient(void)
    : mpoImpl(APL_NULL)
{
    ACL_NEW_ASSERT(this->mpoImpl, CCacheClientImpl);
}

CCacheClient::~CCacheClient(void)
{
    ACL_DELETE(this->mpoImpl);
}

apl_int_t CCacheClient::Initialize(void)
{
    return this->mpoImpl->Initialize();
}

apl_int_t CCacheClient::Initialize2(void)
{
    return this->mpoImpl->Initialize2();
}

void CCacheClient::Close(void)
{
    this->mpoImpl->Close();
}

void CCacheClient::AddServer(
    acl::CSockAddr const& aoRemoteAddress,
    apl_size_t auConnNum,
    apl_size_t auProcessorPoolSize,
    apl_size_t auHashLower,
    apl_size_t auHashUpper,
    acl::CTimeValue const& aoTimeout,
    apl_size_t auWindSize )
{
    this->mpoImpl->AddServer(
        aoRemoteAddress,
        auConnNum,
        auProcessorPoolSize,
        auHashLower,
        auHashUpper,
        aoTimeout,
        auWindSize);
}

apl_int_t CCacheClient::Put( 
    char const* apcKey, 
    apl_size_t auKeyLen, 
    char const* apcValue, 
    apl_size_t auValueLen, 
    acl::CTimeValue const& aoTimeout )
{
    return this->mpoImpl->Put(apcKey, auKeyLen, apcValue, auValueLen, aoTimeout);
}

apl_int_t CCacheClient::Put(
    char const* apcKey,
    char const* apcValue,
    apl_size_t auValueLen,
    acl::CTimeValue const& aoTimeout )
{
    return this->Put(apcKey, apl_strlen(apcKey) + 1, apcValue, auValueLen, aoTimeout);
}

apl_int_t CCacheClient::Put(
    char const* apcKey,
    acl::CMemoryBlock const& aoValue,
    acl::CTimeValue const& aoTimeout )
{
    return this->Put(apcKey, apl_strlen(apcKey) + 1, aoValue.GetReadPtr(), aoValue.GetLength(), aoTimeout);
}

apl_int_t CCacheClient::Get(
    char const* apcKey,
    apl_size_t auKeyLen,
    acl::CMemoryBlock* apoValue,
    acl::CTimeValue const& aoTimeout )
{
    return this->mpoImpl->Get(apcKey, auKeyLen, apoValue, aoTimeout);
}

apl_int_t CCacheClient::Get(
    char const* apcKey,
    acl::CMemoryBlock* apoValue,
    acl::CTimeValue const& aoTimeout )
{
    return this->Get(apcKey, apl_strlen(apcKey) + 1, apoValue, aoTimeout);
}

apl_int_t CCacheClient::GetTimeout(
    acl::CMemoryBlock* apoKey,
    acl::CMemoryBlock* apoValue,
    acl::CTimeValue const& aoTimeout )
{
    return this->mpoImpl->GetTimeout(apoKey, apoValue, aoTimeout);
}

apl_int_t CCacheClient::GetStat(
    apl_int32_t aiServerIdx,
    apl_int32_t& aiCapacity,
    apl_int32_t& aiSize,
    apl_int32_t& aiFileCount,
    acl::CTimeValue const& aoTimeout )
{
    return this->mpoImpl->GetStat(aiServerIdx, aiCapacity, aiSize, aiFileCount, aoTimeout);
}


AIBC_CACHESERVER_NAMESPACE_END

