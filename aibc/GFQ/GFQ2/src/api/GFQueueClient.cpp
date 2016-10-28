
#include "gfq2/GFQueueClient.h"
#include "GFQueueClientImpl.h"
#include "QueuePollImpl.h"

AIBC_GFQ_NAMESPACE_START

////////////////////////////////////////////////////////////////////////////////////////////////////
CGFQueueClient::CGFQueueClient(void)
    : mpoImpl(APL_NULL)
{
    ACL_NEW_ASSERT(mpoImpl, CGFQueueClientImpl);
}

CGFQueueClient::~CGFQueueClient(void)
{
    ACL_DELETE(mpoImpl);
}

void CGFQueueClient::SetReadBufferSize( apl_size_t auBufferSize )
{
    this->mpoImpl->SetReadBufferSize(auBufferSize);
}

void CGFQueueClient::SetWriteBufferSize( apl_size_t auBufferSize )
{
    this->mpoImpl->SetWriteBufferSize(auBufferSize);
}

void CGFQueueClient::SetMessageWindSize( apl_size_t auWindSize )
{
    this->mpoImpl->SetMessageWindSize(auWindSize);
}

void CGFQueueClient::SetErrorCacheTime( acl::CTimeValue const& aoTimedout )
{
    this->mpoImpl->SetErrorCacheTime(aoTimedout);
}
    
apl_int_t CGFQueueClient::Initialize(
    char const* apcName,
    acl::CSockAddr const& aoRemoteAddress, 
    apl_size_t auConnNum, 
    acl::CTimeValue const& aoTimedout,
    apl_int_t aiOpt )
{
    return this->mpoImpl->Initialize(
        apcName, 
        aoRemoteAddress,
        auConnNum,
        aoTimedout,
        aiOpt);
}

void CGFQueueClient::Close(void)
{
    this->mpoImpl->Close();
}

apl_int_t CGFQueueClient::Get( 
    const char* apcQueueName, 
    std::string* apoData )
{
    return this->mpoImpl->Get(apcQueueName, apoData);
}

apl_int_t CGFQueueClient::Get( 
    const char* apcQueueName, 
    acl::CMemoryBlock* apoData )
{
    return this->mpoImpl->Get(apcQueueName, apoData);
}

apl_ssize_t CGFQueueClient::Get(
    const char* apcQueueName, 
    apl_size_t auNumber,
    std::vector<std::string>* apoDatas )
{
    return this->mpoImpl->Get(apcQueueName, auNumber, apoDatas);
}

apl_int_t CGFQueueClient::GetTimedout( 
    const char* apcQueueName,
    apl_int_t aiTimedout, 
    std::string* apoData )
{
    return this->mpoImpl->GetTimedout(apcQueueName, aiTimedout, apoData);
}

apl_int_t CGFQueueClient::GetTimedout( 
    const char* apcQueueName,
    apl_int_t aiTimedout,
    acl::CMemoryBlock* apoData )
{
    return this->mpoImpl->GetTimedout(apcQueueName, aiTimedout, apoData);
}

apl_ssize_t CGFQueueClient::GetTimedout(
    const char* apcQueueName, 
    apl_size_t auNumber,
    apl_int_t aiTimedout,
    std::vector<std::string>* apoDatas )
{
    return this->mpoImpl->GetTimedout(apcQueueName, auNumber, aiTimedout, apoDatas);
}

apl_int_t CGFQueueClient::Put(
    const char* apcQueueName,
    const char* apoBuffer,
    apl_size_t auLength )
{
    return this->mpoImpl->Put(apcQueueName, apoBuffer, auLength);
}

apl_int_t CGFQueueClient::Put(
    const char* apcQueueName,
    std::string const& aoData )
{
    return this->mpoImpl->Put(apcQueueName, aoData);
}

apl_int_t CGFQueueClient::Put(
    const char* apcQueueName,
    acl::CMemoryBlock const& aoData )
{
    return this->mpoImpl->Put(apcQueueName, aoData);
}

apl_int_t CGFQueueClient::GetQueueInfo( char const* apcQueueName, CQueueInfo& aoInfo )
{
    return this->mpoImpl->GetQueueInfo(apcQueueName, aoInfo);
}

apl_int_t CGFQueueClient::GetQueueInfo( apl_size_t auQueueID, CQueueInfo& aoInfo )
{
    return this->mpoImpl->GetQueueInfo(auQueueID, aoInfo);
}

apl_int_t CGFQueueClient::GetAllQueueInfo( std::vector<CQueueInfo>& aoInfos )
{
    return this->mpoImpl->GetAllQueueInfo(aoInfos);
}

apl_int_t CGFQueueClient::GetServerNodeInfo( char const* apcNodeName, CServerNodeInfo& aoInfo )
{
    return this->mpoImpl->GetServerNodeInfo(apcNodeName, aoInfo);
}

apl_int_t CGFQueueClient::GetAllServerNodeInfo( std::vector<CServerNodeInfo>& aoInfos )
{
    return this->mpoImpl->GetAllServerNodeInfo(aoInfos);
}

apl_int_t CGFQueueClient::GetStartupTimestamp( acl::CTimestamp& aoTimestamp )
{
    return this->mpoImpl->GetStartupTimestamp(aoTimestamp);
}

apl_int_t CGFQueueClient::BindNodeGroup( char const* apcQueueName, char const* apcGroupName )
{
    return this->mpoImpl->BindNodeGroup(apcQueueName, apcGroupName);
}

CGFQueueClientImpl* CGFQueueClient::GetImpl(void)
{
    return this->mpoImpl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
CQueuePoll::CQueuePoll( CGFQueueClient& aoClient )
    : mpoImpl(APL_NULL)
    , mpoClient(&aoClient)
{
    ACL_NEW_ASSERT(this->mpoImpl, CQueuePollImpl(aoClient.GetImpl() ) );
}

CQueuePoll::~CQueuePoll(void)
{
    ACL_DELETE(this->mpoImpl);
}

void CQueuePoll::Add( char const* apcQueueName )
{
    this->mpoImpl->Add(apcQueueName);
}

void CQueuePoll::Remove( char const* apcQueueName )
{
    this->mpoImpl->Remove(apcQueueName);
}

void CQueuePoll::SetResultCacheTime( acl::CTimeValue const& aoTimedout )
{
    this->mpoImpl->SetResultCacheTime(aoTimedout);
}

apl_ssize_t CQueuePoll::Poll( acl::CTimeValue const& aoTimedout, std::vector<std::string>* apoResult )
{
    return this->mpoImpl->Poll(aoTimedout, apoResult);
}

CGFQueueClient* CQueuePoll::GetClient(void)
{
    return this->mpoClient;
}

CQueuePollImpl* CQueuePoll::GetImpl(void)
{
    return this->mpoImpl;
}

AIBC_GFQ_NAMESPACE_END

