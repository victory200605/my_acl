
#include "acl/TimeValue.h"
#include "acl/SockAddr.h"
#include "acl/MemoryBlock.h"
#include "gfq/GFQueueClient.h"
#include "GFQueueClientImpl.h"

AIBC_GFQ_NAMESPACE_START

CGFQueueClient::CGFQueueClient(void)
    : mpoImpl(APL_NULL)
{
    ACL_NEW_ASSERT(this->mpoImpl, CGFQueueClientImpl);
}

CGFQueueClient::~CGFQueueClient(void)
{
    ACL_DELETE(this->mpoImpl);
}
	
apl_int_t CGFQueueClient::Initialize(
    char const* apcName,
    acl::CSockAddr const& aoRemoteAddress, 
    apl_size_t auConnNum, 
    acl::CTimeValue const& aoTimeout,
    apl_size_t auWindSize,
    bool abIsIgnoreConnectFail )
{
    return this->mpoImpl->Initialize(apcName, aoRemoteAddress, auConnNum, aoTimeout, auWindSize, abIsIgnoreConnectFail);
}

void CGFQueueClient::Close(void)
{
    this->mpoImpl->Close();
}

void CGFQueueClient::SetErrorCacheInterval( apl_int_t aiInterval )
{
    this->mpoImpl->SetErrorCacheInterval(aiInterval);
}

apl_int_t CGFQueueClient::GetErrorCacheInterval(void)
{
    return this->mpoImpl->GetErrorCacheInterval();
}

apl_int_t CGFQueueClient::Get( const char* apcQueueName, acl::CMemoryBlock* apoData, apl_int_t aiWhenSec )
{
    return this->mpoImpl->Get(apcQueueName, apoData, aiWhenSec);
}

apl_int_t CGFQueueClient::Put( const char* apcQueueName, acl::CMemoryBlock const& aoData )
{
    return this->mpoImpl->Put(apcQueueName, aoData);
}

apl_int_t CGFQueueClient::GetStat( const char* apcQueueName, apl_int_t* apiSize, apl_int_t* apiCapacity )
{
    return this->mpoImpl->GetStat(apcQueueName, apiSize, apiCapacity);
}

apl_int_t CGFQueueClient::GetModuleCount( apl_int_t* apiCount )
{
    return this->mpoImpl->GetModuleCount(apiCount);
}
 
apl_int_t CGFQueueClient::ReloadQueueInfo(void)
{
    return this->mpoImpl->ReloadQueueInfo();
}

apl_int_t CGFQueueClient::PrintQueues(void)
{
    return this->mpoImpl->PrintQueues();
}

apl_int_t CGFQueueClient::PrintModules(void)
{
    return this->mpoImpl->PrintModules();
}
   
AIBC_GFQ_NAMESPACE_END

