
#include "GFQueueClientImpl.h"

AIBC_GFQ_NAMESPACE_START

/** 
 * @brief Asynch client support Mcast for master server
 */
CMasterClient::CMasterClient(void)
    : mbIsMcastAddr(false)
{
}

apl_int_t CMasterClient::Connect( acl::CSockAddr const& aoRemoteAddress )
{
    this->moRemoteAddress.Set(aoRemoteAddress);
    
    apl_int_t liHigh = apl_strtoi32(this->moRemoteAddress.GetIpAddr(), APL_NULL, 10);

    if (liHigh >= 224 && liHigh <= 239)
    {
        this->mbIsMcastAddr = true;
    }
    else
    {
        this->mbIsMcastAddr = false;
    }
    
    apl_int_t liRetCode = this->moDgram.Open();
    ACL_ASSERT(liRetCode == 0);

    acl::CSockAddr loRemote;
    this->GetRemoteAddress(&loRemote, 2);

    return ClientType::Connect("master", loRemote, 1, true);
}

void CMasterClient::GetRemoteAddress( acl::CSockAddr* apoRemoteAddress, apl_int_t aiTryTimes )
{
    if (this->mbIsMcastAddr)
    {
        acl::CTimeValue loTimedout(2);
        char lacBuffer[64] = {0,};

        for (apl_int_t i = 0; i < aiTryTimes; i++)
        {
            if (this->moDgram.Send("who is master", 14, this->moRemoteAddress, loTimedout) != 14)
            {
                continue;
            }

            if (this->moDgram.Recv(lacBuffer, sizeof(lacBuffer) - 1, APL_NULL, loTimedout) > 0)
            {
                acl::CTokenizer loTokenizer(":");

                loTokenizer.Parse(lacBuffer);

                if (loTokenizer.GetSize() != 2)
                {
                    continue;
                }

                apoRemoteAddress->Set(
                    apl_strtoi32(loTokenizer.GetField(1), APL_NULL, 10), loTokenizer.GetField(0) );

                break;
            }
        }
    }
    else
    {
        apoRemoteAddress->Set(this->moRemoteAddress);
    }
}

void CMasterClient::HandleConnectException(
    char const* apcName,
    acl::CSockAddr const& aoRemoteAddress,
    apl_int_t aiState,
    apl_int_t aiErrno )
{
    acl::CSockAddr loRemote;
    
    this->GetRemoteAddress(&loRemote, 10);

    ClientType::Connect("master", loRemote, 1, false);
}

/** 
 * @brief Error cache to reduce repeat request times
 */
CErrorCache::CErrorCache(void)
    : miErrno(0)
{
}

bool CErrorCache::IsHitCache( acl::CTimeValue const& aoTimedout )
{
    if ( (this->miErrno == ERROR_END_OF_QUEUE 
        || this->miErrno == ERROR_NO_ENOUGH_SPACE ) )
    {
        acl::CTimestamp loCurrTime;
        loCurrTime.Update(acl::CTimestamp::PRC_MSEC);

        if (loCurrTime.Msec() - this->moTimestamp.Msec() < aoTimedout.Msec() )
        {
            return true;
        }
    }
    
    return false;
}

apl_int_t CErrorCache::GetErrno(void)
{
    return this->miErrno;
}

void CErrorCache::Update( apl_int_t aiErrno )
{
    this->miErrno = aiErrno;
    this->moTimestamp.Update();
}

// do request to server
template<class T> apl_int_t DoRequest( 
    T& aoAsyncClient, 
    acl::CIndexDict& aoRequest, 
    acl::CIndexDict** apoResponse, 
    acl::CTimeValue const& aoTimedout  )
{
    apl_int_t liRetCode = 0;
    
    if ( (liRetCode = aoAsyncClient.Request(aoRequest, apoResponse, aoTimedout) ) != 0 )
    {
        apl_errprintf( "GFQ2 Async client send request fail, (RetCode=%"APL_PRIdINT",CommandID=0x%"APL_PRIx32")\n",
            liRetCode,
            (apl_int32_t)aoRequest.GetInt(anf::BENC_TAG_CMD_ID, 0) );
        
        return liRetCode == anf::E_TIMEOUT ? ERROR_TIMEDOUT : ERROR_SOCKET_SEND;
    }

    liRetCode = (*apoResponse)->GetInt(anf::BENC_TAG_ERROR_CODE, 0);
    if (liRetCode != 0)
    {
        ACL_DELETE(*apoResponse);
    }
    
    return liRetCode;
}

CGFQueueClientImpl::CGFQueueClientImpl(void)
    : muConnNum(1)
    , muWindSize(1000)
    , moTimedout(6)
    , moErrorCacheTime(2)
    , mpoProcessor(APL_NULL)
{
    this->macUserName[0] = '\0';

    this->SetReadBufferSize(1024*10); //default 10k
}

CGFQueueClientImpl::~CGFQueueClientImpl(void)
{
    ACL_DELETE(this->mpoProcessor);
}

void CGFQueueClientImpl::SetReadBufferSize( apl_size_t auBufferSize )
{
    this->moConfiguration.SetReadBufferSize(auBufferSize);
}

void CGFQueueClientImpl::SetWriteBufferSize( apl_size_t auBufferSize )
{
    this->moConfiguration.SetWriteBufferSize(auBufferSize);
}

void CGFQueueClientImpl::SetMessageWindSize( apl_size_t auWindSize )
{
    this->muWindSize = auWindSize;
}
    
void CGFQueueClientImpl::SetErrorCacheTime( acl::CTimeValue const& aoTimedout )
{
    this->moErrorCacheTime = aoTimedout;
}

apl_int_t CGFQueueClientImpl::Initialize(
    char const* apcName,
    acl::CSockAddr const& aoRemoteAddress, 
    apl_size_t auConnNum, 
    acl::CTimeValue const& aoTimedout,
    apl_int_t aiOpt )
{
    this->moMaster.GetFilterChain()->AddFirst("bencoding-message", new anf::CBencodingMessageFilter);
    this->moMaster.GetConnector()->SetConfiguration(&this->moConfiguration);

    if (this->moMaster.Initialize(this->muWindSize, aoTimedout) != 0)
    {
        return ERROR_SYSTEM;
    }
    
    if (this->moMaster.Connect(aoRemoteAddress) <= 0 && aiOpt != IGNORE_CONNECT_FAIL)
    {
        return ERROR_CONNECT_REJECT;
    }

    this->muConnNum  = auConnNum;
    this->moTimedout  = aoTimedout;

    if (this->muConnNum > 1)
    {
        ACL_NEW_INIT_ASSERT(this->mpoProcessor, anf::TIoSimpleProcessorPool<anf::CIoPollingProcessor>, this->muConnNum);
    }
    else
    {
        ACL_NEW_ASSERT(this->mpoProcessor, anf::CIoPollingProcessor);
    }

    this->mpoProcessor->SetConfiguration(&this->moConfiguration);

    apl_snprintf(this->macUserName, sizeof(this->macUserName),
        "%s-%"APL_PRIu64, apcName, (apl_uint64_t)acl::CProcess::GetPid() );
    
    return 0;
}

void CGFQueueClientImpl::Close(void)
{
    this->moMaster.Close();

    for (std::map<std::string, ClientType*>::iterator loIter = this->moServers.begin(); 
         loIter != this->moServers.end(); ++loIter )
    {
        loIter->second->Close();
        ACL_DELETE(loIter->second);
    }

    this->mpoProcessor->Dispose();

    this->moServers.clear();
}

apl_int_t CGFQueueClientImpl::Get( const char* apcQueueName, acl::CIndexDict& aoRequest, acl::CIndexDict** appoResponse )
{
    apl_int_t    liRetCode = 0;
    CSegmentInfo loSegmentInfo;
    
    do
    {
        if (this->moReadableSegments.Find(apcQueueName, loSegmentInfo) )
        {
            if (loSegmentInfo.moErrorCache.GetErrno() == 0)
            {
                break;
            }
            else if (loSegmentInfo.moErrorCache.IsHitCache(this->moErrorCacheTime) )
            {
                return loSegmentInfo.moErrorCache.GetErrno();
            }
        }
        
        if ( (liRetCode = this->RequestAndUpdateReadableSegment(
            apcQueueName, 
            INVALID_ID, 
            APL_NULL, 
            0, 
            loSegmentInfo) ) != 0)
        {
            return liRetCode;
        }
    }
    while(false);

    for( int liIt = 0; liIt < DEAD_CIRCLE_NUM; liIt++ ) //Ignore dead circle
    {
        aoRequest.Set(GFQ_TAG_SEGMENT_ID, loSegmentInfo.muSegmentID);
        aoRequest.Set(GFQ_TAG_TIMESTAMP, loSegmentInfo.mi64Timestamp);
        aoRequest.Set(GFQ_TAG_QUEUE_NAME, loSegmentInfo.macQueueName);

        liRetCode = DoRequest(*loSegmentInfo.mpoSegmentServer, aoRequest, appoResponse, this->moTimedout);
        if (liRetCode == 0
            || liRetCode == ERROR_TIMEDOUT
            || liRetCode == ERROR_TIMEDOUT_UNEXISTED)
        {
            return liRetCode;
        } 
        
        // Dead circle, is impossible, it will be break and save ret code liRet
        if (liIt >= DEAD_CIRCLE_NUM - 1) break;
        
        if ( (liRetCode = this->RequestAndUpdateReadableSegment(
            apcQueueName,
            loSegmentInfo.muSegmentID,
            loSegmentInfo.macServerNodeName,
            liRetCode,
            loSegmentInfo) ) != 0)
        {
            return liRetCode;
        }
        //Yes, second get read segment success
    }
    
    // Break, dead circle
    return liRetCode;
}

apl_int_t CGFQueueClientImpl::Get( const char* apcQueueName, std::string* apoData )
{
    apl_int_t        liRetCode = 0;
    acl::CIndexDict  loRequest;
    acl::CIndexDict* lpoResponse = APL_NULL;
    
    loRequest.Set(anf::BENC_TAG_CMD_ID, GFQ_REQUEST_POP);
    loRequest.Set(GFQ_TAG_RECORD_LIMIT, 1);

    if ( (liRetCode = this->Get(apcQueueName, loRequest, &lpoResponse) ) == 0)
    {
        if (apoData != APL_NULL)
        {
            acl::CIndexDict::ValueType loValue = lpoResponse->GetStr(GFQ_TAG_RECORD_DATA, "");
            apoData->assign(loValue.c_str(), loValue.size() );
        }

        ACL_DELETE(lpoResponse);
    }

    return liRetCode;
}

apl_int_t CGFQueueClientImpl::Get( const char* apcQueueName, acl::CMemoryBlock* apoData )
{
    apl_int_t        liRetCode = 0;
    acl::CIndexDict  loRequest;
    acl::CIndexDict* lpoResponse = APL_NULL;
    
    loRequest.Set(anf::BENC_TAG_CMD_ID, GFQ_REQUEST_POP);
    loRequest.Set(GFQ_TAG_RECORD_LIMIT, 1);

    if ( (liRetCode = this->Get(apcQueueName, loRequest, &lpoResponse) ) == 0)
    {
        if (apoData != APL_NULL)
        {
            acl::CIndexDict::ValueType loValue = lpoResponse->GetStr(GFQ_TAG_RECORD_DATA, "");
            apoData->Reset();
            apoData->Resize(loValue.size() );
            apl_memcpy(apoData->GetWritePtr(), loValue.c_str(), loValue.size() );
            apoData->SetWritePtr(loValue.size() );
        }

        ACL_DELETE(lpoResponse);
    }

    return liRetCode;
}

apl_ssize_t CGFQueueClientImpl::Get( const char* apcQueueName, apl_size_t auNumber, std::vector<std::string>* apoDatas )
{
    apl_int_t        liRetCode = 0;
    acl::CIndexDict  loRequest;
    acl::CIndexDict* lpoResponse = APL_NULL;
    
    loRequest.Set(anf::BENC_TAG_CMD_ID, GFQ_REQUEST_POP);
    loRequest.Set(GFQ_TAG_RECORD_LIMIT, auNumber);

    if ( (liRetCode = this->Get(apcQueueName, loRequest, &lpoResponse) ) == 0)
    {
        if (apoDatas != APL_NULL)
        {
            apl_size_t luCount = lpoResponse->Count(GFQ_TAG_RECORD_DATA);
            apoDatas->resize(luCount);
            for (apl_size_t luN = 0; luN < luCount; luN++)
            {
                acl::CIndexDict::ValueType loValue = lpoResponse->GetStr2(GFQ_TAG_RECORD_DATA, luN, "");
                (*apoDatas)[luN].assign(loValue.c_str(), loValue.size() );
            }
        }

        ACL_DELETE(lpoResponse);

        return apoDatas->size();
    }
    else
    {
        return liRetCode;
    }
}

apl_int_t CGFQueueClientImpl::GetTimedout( const char* apcQueueName, apl_int_t aiTimedout, std::string* apoData )
{
    apl_int_t        liRetCode = 0;
    acl::CIndexDict  loRequest;
    acl::CIndexDict* lpoResponse = APL_NULL;
    
    loRequest.Set(anf::BENC_TAG_CMD_ID, GFQ_REQUEST_POP_TIMEDOUT);
    loRequest.Set(GFQ_TAG_TIMEDOUT, aiTimedout);
    loRequest.Set(GFQ_TAG_RECORD_LIMIT, 1);

    if ( (liRetCode = this->Get(apcQueueName, loRequest, &lpoResponse) ) == 0)
    {
        if (apoData != APL_NULL)
        {
            acl::CIndexDict::ValueType loValue = lpoResponse->GetStr(GFQ_TAG_RECORD_DATA, "");
            apoData->assign(loValue.c_str(), loValue.size() );
        }

        ACL_DELETE(lpoResponse);
    }

    return liRetCode;
}

apl_int_t CGFQueueClientImpl::GetTimedout( const char* apcQueueName, apl_int_t aiTimedout, acl::CMemoryBlock* apoData )
{
    apl_int_t        liRetCode = 0;
    acl::CIndexDict  loRequest;
    acl::CIndexDict* lpoResponse = APL_NULL;
    
    loRequest.Set(anf::BENC_TAG_CMD_ID, GFQ_REQUEST_POP_TIMEDOUT);
    loRequest.Set(GFQ_TAG_TIMEDOUT, aiTimedout);
    loRequest.Set(GFQ_TAG_RECORD_LIMIT, 1);

    if ( (liRetCode = this->Get(apcQueueName, loRequest, &lpoResponse) ) == 0)
    {
        if (apoData != APL_NULL)
        {
            acl::CIndexDict::ValueType loValue = lpoResponse->GetStr(GFQ_TAG_RECORD_DATA, "");
            apoData->Reset();
            apoData->Resize(loValue.size() );
            apl_memcpy(apoData->GetWritePtr(), loValue.c_str(), loValue.size() );
            apoData->SetWritePtr(loValue.size() );
        }

        ACL_DELETE(lpoResponse);
    }

    return liRetCode;
}

apl_ssize_t CGFQueueClientImpl::GetTimedout( 
    const char* apcQueueName, 
    apl_size_t auNumber,
    apl_int_t aiTimedout, 
    std::vector<std::string>* apoDatas )
{
    apl_int_t        liRetCode = 0;
    acl::CIndexDict  loRequest;
    acl::CIndexDict* lpoResponse = APL_NULL;
    
    loRequest.Set(anf::BENC_TAG_CMD_ID, GFQ_REQUEST_POP_TIMEDOUT);
    loRequest.Set(GFQ_TAG_TIMEDOUT, aiTimedout);
    loRequest.Set(GFQ_TAG_RECORD_LIMIT, auNumber);

    if ( (liRetCode = this->Get(apcQueueName, loRequest, &lpoResponse) ) == 0)
    {
        if (apoDatas != APL_NULL)
        {
            apl_size_t luCount = lpoResponse->Count(GFQ_TAG_RECORD_DATA);
            apoDatas->resize(luCount);
            for (apl_size_t luN = 0; luN < luCount; luN++)
            {
                acl::CIndexDict::ValueType loValue = lpoResponse->GetStr2(GFQ_TAG_RECORD_DATA, luN, "");
                (*apoDatas)[luN].assign(loValue.c_str(), loValue.size() );
            }
        }

        ACL_DELETE(lpoResponse);

        return apoDatas->size();
    }
    else
    {
        return liRetCode;
    }
}

apl_int_t CGFQueueClientImpl::Put( const char* apcQueueName, acl::CIndexDict& aoRequest, acl::CIndexDict** appoResponse )
{
    apl_int_t    liRetCode = 0;
    CSegmentInfo loSegmentInfo;
    
    do
    {
        if (this->moWriteableSegments.Find(apcQueueName, loSegmentInfo) )
        {
            if (loSegmentInfo.moErrorCache.GetErrno() == 0)
            {
                break;
            }
            else if (loSegmentInfo.moErrorCache.IsHitCache(this->moErrorCacheTime) )
            {
                return loSegmentInfo.moErrorCache.GetErrno();
            }
        }
        
        if ( (liRetCode = this->RequestAndUpdateWriteableSegment(
            apcQueueName, 
            INVALID_ID, 
            APL_NULL, 
            0, 
            loSegmentInfo) ) != 0)
        {
            return liRetCode;
        }
    }
    while(false);

    for( int liIt = 0; liIt < DEAD_CIRCLE_NUM; liIt++ ) //Ignore dead circle
    {
        aoRequest.Set(GFQ_TAG_SEGMENT_ID, loSegmentInfo.muSegmentID);
        aoRequest.Set(GFQ_TAG_TIMESTAMP, loSegmentInfo.mi64Timestamp);
        aoRequest.Set(GFQ_TAG_QUEUE_NAME, loSegmentInfo.macQueueName);

        liRetCode = DoRequest(*loSegmentInfo.mpoSegmentServer, aoRequest, appoResponse, this->moTimedout);
        if (liRetCode == 0
            || liRetCode == ERROR_TIMEDOUT
            || liRetCode == ERROR_OUT_OF_RANGE)
        {
            return liRetCode;
        }

        // Dead circle, is impossible, it will be break and save ret code liRet
        if (liIt >= DEAD_CIRCLE_NUM - 1) break;

        if ( (liRetCode = this->RequestAndUpdateWriteableSegment(
            apcQueueName,
            loSegmentInfo.muSegmentID,
            loSegmentInfo.macServerNodeName,
            liRetCode,
            loSegmentInfo) ) != 0)
        {
            return liRetCode;
        }
        //Yes, second get read segment success
    }
    
    // Break, dead circle
    return liRetCode;
}

apl_int_t CGFQueueClientImpl::Put( const char* apcQueueName, char const* apoBuffer, apl_size_t auLength )
{
    apl_int_t        liRetCode = 0;
    acl::CIndexDict  loRequest;
    acl::CIndexDict* lpoResponse = APL_NULL;
    
    if (auLength == 0)
    {
        return ERROR_INVALID_DATA;
    }

    loRequest.Set(anf::BENC_TAG_CMD_ID, GFQ_REQUEST_PUSH);
    loRequest.Set(GFQ_TAG_RECORD_DATA, apoBuffer, auLength);

    if ( (liRetCode = this->Put(apcQueueName, loRequest, &lpoResponse) ) == 0)
    {
        ACL_DELETE(lpoResponse);
    }

    return liRetCode;
}

apl_int_t CGFQueueClientImpl::Put( const char* apcQueueName, std::string const& aoData )
{
    return this->Put(apcQueueName, aoData.c_str(), aoData.length() );
}

apl_int_t CGFQueueClientImpl::Put( const char* apcQueueName, acl::CMemoryBlock const& aoData )
{
    return this->Put(apcQueueName, aoData.GetReadPtr(), aoData.GetLength() );
}

apl_int_t CGFQueueClientImpl::CreateQueue( char const* apcQueueName, apl_int_t aiOpt, CQueueInfo* apoInfo )
{
    apl_int_t        liRetCode = 0;
    acl::CIndexDict  loRequest;
    acl::CIndexDict* lpoResponse = APL_NULL;
    
    loRequest.Set(anf::BENC_TAG_CMD_ID, GFQ_REQUEST_CREATE_QUEUE);
    loRequest.Set(GFQ_TAG_QUEUE_NAME, apcQueueName);
    loRequest.Set(GFQ_TAG_OPTION, aiOpt);
    
    liRetCode = DoRequest(this->moMaster, loRequest, &lpoResponse, this->moTimedout);
    if (liRetCode == 0)
    {
        if (apoInfo != APL_NULL)
        {
            apoInfo->SetQueueName(apcQueueName);
            apoInfo->SetQueueID(lpoResponse->GetInt(GFQ_TAG_QUEUE_ID, 0) );
            apoInfo->SetCapacity(lpoResponse->GetInt(GFQ_TAG_QUEUE_CAPACITY, (apl_intmax_t)0) );
            apoInfo->SetSize(lpoResponse->GetInt(GFQ_TAG_QUEUE_SIZE, (apl_intmax_t)0) );
            apoInfo->SetSegmentCount(lpoResponse->GetInt(GFQ_TAG_SEGMENT_COUNT, (apl_intmax_t)0) );
        }

        ACL_DELETE(lpoResponse);

        return 0;
    }

    return liRetCode;
}

apl_int_t CGFQueueClientImpl::GetQueueInfo( char const* apcQueueName, CQueueInfo& aoInfo )
{
    apl_int_t        liRetCode = 0;
    acl::CIndexDict  loRequest;
    acl::CIndexDict* lpoResponse = APL_NULL;
    
    loRequest.Set(anf::BENC_TAG_CMD_ID, GFQ_REQUEST_QUEUE_INFO);
    loRequest.Set(GFQ_TAG_QUEUE_NAME, apcQueueName);
    
    liRetCode = DoRequest(this->moMaster, loRequest, &lpoResponse, this->moTimedout);
    if (liRetCode == 0)
    {
        aoInfo.SetQueueName(apcQueueName);
        aoInfo.SetQueueID(lpoResponse->GetInt(GFQ_TAG_QUEUE_ID, 0) );
        aoInfo.SetCapacity(lpoResponse->GetInt(GFQ_TAG_QUEUE_CAPACITY, (apl_intmax_t)0) );
        aoInfo.SetSize(lpoResponse->GetInt(GFQ_TAG_QUEUE_SIZE, (apl_intmax_t)0) );
        aoInfo.SetSegmentCount(lpoResponse->GetInt(GFQ_TAG_SEGMENT_COUNT, (apl_intmax_t)0) );

        ACL_DELETE(lpoResponse);

        return 0;
    }

    return liRetCode;
}

apl_int_t CGFQueueClientImpl::GetQueueInfo( apl_size_t auQueueID, CQueueInfo& aoInfo )
{
    apl_int_t        liRetCode = 0;
    acl::CIndexDict  loRequest;
    acl::CIndexDict* lpoResponse = APL_NULL;
    
    loRequest.Set(anf::BENC_TAG_CMD_ID, GFQ_REQUEST_QUEUE_INFO);
    loRequest.Set(GFQ_TAG_QUEUE_ID, auQueueID);
    
    liRetCode = DoRequest(this->moMaster, loRequest, &lpoResponse, this->moTimedout);
    if (liRetCode == 0)
    {
        aoInfo.SetQueueID(auQueueID);
        aoInfo.SetQueueName(lpoResponse->Get(GFQ_TAG_QUEUE_NAME, "") );
        aoInfo.SetCapacity(lpoResponse->GetInt(GFQ_TAG_QUEUE_CAPACITY, (apl_intmax_t)0) );
        aoInfo.SetSize(lpoResponse->GetInt(GFQ_TAG_QUEUE_SIZE, (apl_intmax_t)0) );
        aoInfo.SetSegmentCount(lpoResponse->GetInt(GFQ_TAG_SEGMENT_COUNT, (apl_intmax_t)0) );

        ACL_DELETE(lpoResponse);

        return 0;
    }

    return liRetCode;
}

apl_int_t CGFQueueClientImpl::GetAllQueueInfo( std::vector<CQueueInfo>& aoInfos )
{
    apl_int_t        liRetCode = 0;
    apl_size_t       luOffset = 0;
    apl_size_t       luLimit = 100;
    acl::CIndexDict* lpoResponse = APL_NULL;
    CQueueInfo loInfo;

    aoInfos.clear();
    
    while(true)
    {
        acl::CIndexDict loRequest;

        loRequest.Set(anf::BENC_TAG_CMD_ID, GFQ_REQUEST_QUEUE_LIST);
        loRequest.Set(GFQ_TAG_RECORD_OFFSET, luOffset);
        loRequest.Set(GFQ_TAG_RECORD_LIMIT, luLimit);
    
        liRetCode = DoRequest(this->moMaster, loRequest, &lpoResponse, this->moTimedout);
        if (liRetCode == 0)
        {
            apl_size_t luCount = lpoResponse->Count(GFQ_TAG_QUEUE_NAME);
            for (apl_size_t luN = 0; luN < luCount; luN++)
            {
                char const* lpoQueueName = lpoResponse->Get2(GFQ_TAG_QUEUE_NAME, luN, "");
                if (this->GetQueueInfo(lpoQueueName, loInfo) == 0)
                {
                    aoInfos.push_back(loInfo);
                }
            }

            ACL_DELETE(lpoResponse);

            if (luCount == 0 || luCount < luLimit)
            {
                break;
            }
            else
            {
                luOffset += luCount;
            }

            continue;
        }
        
        break;
    }

    return liRetCode;
}

apl_int_t CGFQueueClientImpl::GetServerNodeInfo( char const* apcNodeName, CServerNodeInfo& aoInfo )
{
    apl_int_t        liRetCode = 0;
    acl::CIndexDict  loRequest;
    acl::CIndexDict* lpoResponse = APL_NULL;
    
    loRequest.Set(anf::BENC_TAG_CMD_ID, GFQ_REQUEST_NODE_INFO);
    loRequest.Set(GFQ_TAG_NODE_NAME, apcNodeName);
    
    liRetCode = DoRequest(this->moMaster, loRequest, &lpoResponse, this->moTimedout);
    if (liRetCode == 0)
    {
        aoInfo.SetServerNodeName(apcNodeName);
        aoInfo.SetServerIp(lpoResponse->Get(GFQ_TAG_NODE_SERVER_IP) );
        aoInfo.SetServerPort(lpoResponse->GetInt(GFQ_TAG_NODE_SERVER_PORT, 0) );
        aoInfo.SetGroupName(lpoResponse->Get(GFQ_TAG_GROUP_NAME, "") );
        aoInfo.SetStorageType(lpoResponse->Get(GFQ_TAG_NODE_STORAGE_TYPE, "") );
        aoInfo.SetSegmentCount(lpoResponse->GetInt(GFQ_TAG_SEGMENT_COUNT, 0) );
        aoInfo.SetFreeSegmentCount(lpoResponse->GetInt(GFQ_TAG_NODE_FREE_SEGMENT_COUNT, 0) );
        aoInfo.SetUsedSegmentCount(lpoResponse->GetInt(GFQ_TAG_NODE_USED_SEGMENT_COUNT, 0) );
        aoInfo.SetConnectionCount(lpoResponse->GetInt(GFQ_TAG_NODE_CONNECTION_COUNT, 0) );
        aoInfo.SetReadTimes(lpoResponse->GetInt(GFQ_TAG_NODE_READ_TIMES, 0) );
        aoInfo.SetWriteTimes(lpoResponse->GetInt(GFQ_TAG_NODE_WRITE_TIMES, 0) );
        aoInfo.SetFreePageCount(lpoResponse->GetInt(GFQ_TAG_NODE_FREE_PAGE_COUNT, 0) );
        aoInfo.SetUsedPageCount(lpoResponse->GetInt(GFQ_TAG_NODE_USED_PAGE_COUNT, 0) );
        aoInfo.SetCacheFreePageCount(lpoResponse->GetInt(GFQ_TAG_NODE_CACHE_FREE_PAGE_COUNT, 0) );
        aoInfo.SetCacheUsedPageCount(lpoResponse->GetInt(GFQ_TAG_NODE_CACHE_USED_PAGE_COUNT, 0) );
        aoInfo.SetCacheReadTimes(lpoResponse->GetInt(GFQ_TAG_NODE_CACHE_READ_TIMES, 0) );
        aoInfo.SetCacheReadMissedTimes(lpoResponse->GetInt(GFQ_TAG_NODE_CACHE_READ_MISSED_TIMES, 0) );
        aoInfo.SetCacheWriteTimes(lpoResponse->GetInt(GFQ_TAG_NODE_CACHE_WRITE_TIMES, 0) );
        aoInfo.SetCacheWriteMissedTimes(lpoResponse->GetInt(GFQ_TAG_NODE_CACHE_WRITE_MISSED_TIMES, 0) );
        aoInfo.SetCacheSyncTimes(lpoResponse->GetInt(GFQ_TAG_NODE_CACHE_SYNC_TIMES, 0) );

        ACL_DELETE(lpoResponse);

        return 0;
        }

    return liRetCode;
}

apl_int_t CGFQueueClientImpl::GetAllServerNodeInfo( std::vector<CServerNodeInfo>& aoInfos )
{
    apl_int_t        liRetCode = 0;
    acl::CIndexDict  loRequest;
    acl::CIndexDict* lpoResponse = APL_NULL;
    
    loRequest.Set(anf::BENC_TAG_CMD_ID, GFQ_REQUEST_NODE_LIST);
    
    liRetCode = DoRequest(this->moMaster, loRequest, &lpoResponse, this->moTimedout);
    if (liRetCode == 0)
    {
        CServerNodeInfo loInfo;
        apl_size_t luCount = lpoResponse->Count(GFQ_TAG_NODE_NAME);
        for (apl_size_t luN = 0; luN < luCount; luN++)
        {
            char const* lpcNodeName = lpoResponse->Get2(GFQ_TAG_NODE_NAME, luN, "");

            if (this->GetServerNodeInfo(lpcNodeName, loInfo) == 0)
            {
                aoInfos.push_back(loInfo);
            }
        }

        ACL_DELETE(lpoResponse);

        return 0;
    }

    return liRetCode;
}

apl_int_t CGFQueueClientImpl::GetStartupTimestamp( acl::CTimestamp& aoTimestamp )
{
    apl_int_t        liRetCode = 0;
    acl::CIndexDict  loRequest;
    acl::CIndexDict* lpoResponse = APL_NULL;
    
    loRequest.Set(anf::BENC_TAG_CMD_ID, GFQ_REQUEST_STARTUP_TIMESTAMP);
    
    liRetCode = DoRequest(this->moMaster, loRequest, &lpoResponse, this->moTimedout);
    if (liRetCode == 0)
    {
        aoTimestamp.Nsec(lpoResponse->GetInt(GFQ_TAG_TIMESTAMP) );
        
        ACL_DELETE(lpoResponse);

        return 0;
    }

    return liRetCode;
}

apl_int_t CGFQueueClientImpl::BindNodeGroup( char const* apcQueueName, char const* apcGroupName )
{
    apl_int_t        liRetCode = 0;
    acl::CIndexDict  loRequest;
    acl::CIndexDict* lpoResponse = APL_NULL;
    
    loRequest.Set(anf::BENC_TAG_CMD_ID, GFQ_REQUEST_BIND_NODE_GROUP);
    loRequest.Set(GFQ_TAG_QUEUE_NAME, apcQueueName);
    loRequest.Set(GFQ_TAG_GROUP_NAME, apcGroupName);
    
    liRetCode = DoRequest(this->moMaster, loRequest, &lpoResponse, this->moTimedout);
    if (liRetCode == 0)
    {
        ACL_DELETE(lpoResponse);

        return 0;
    }

    return liRetCode;
}

apl_ssize_t CGFQueueClientImpl::CommonRequest( acl::CIndexDict& aoRequest, acl::CIndexDict** appoResponse, acl::CTimeValue const& aoTimedout )
{
    return DoRequest(this->moMaster, aoRequest, appoResponse, aoTimedout);
}

apl_int_t CGFQueueClientImpl::RequestAndUpdateReadableSegment(
    char const* apcQueueName, 
    apl_size_t auOldSegmentID,
    char const* apcOldNodeName,
    apl_int_t aiOldSegmentStatus,
    CSegmentInfo& aoSegmentInfo )
{
    apl_int_t liRetCode = 0;

    liRetCode = this->RequestQueueSegment(
        GFQ_REQUEST_READABLE_SEGMENT,
        apcQueueName,
        auOldSegmentID,
        apcOldNodeName,
        aiOldSegmentStatus,
        aoSegmentInfo);

    aoSegmentInfo.moErrorCache.Update(liRetCode);

    this->moReadableSegments.Insert(apcQueueName, aoSegmentInfo);
    
    return liRetCode;
}

apl_int_t CGFQueueClientImpl::RequestAndUpdateWriteableSegment(
    char const* apcQueueName, 
    apl_size_t auOldSegmentID,
    char const* apcOldNodeName,
    apl_int_t aiOldSegmentStatus,
    CSegmentInfo& aoSegmentInfo )
{
    apl_int_t liRetCode = 0;

    liRetCode = this->RequestQueueSegment(
        GFQ_REQUEST_WRITEABLE_SEGMENT,
        apcQueueName,
        auOldSegmentID,
        apcOldNodeName,
        aiOldSegmentStatus,
        aoSegmentInfo);
    
    aoSegmentInfo.moErrorCache.Update(liRetCode);

    this->moWriteableSegments.Insert(apcQueueName, aoSegmentInfo);

    return liRetCode;
}

apl_int_t CGFQueueClientImpl::RequestQueueSegment(
    apl_int_t aiCommandID,
    char const* apcQueueName, 
    apl_size_t auOldSegmentID,
    char const* apcOldNodeName,
    apl_int_t aiOldSegmentStatus,
    CSegmentInfo& aoSegmentInfo )
{
    apl_int_t liRetCode = 0;
    acl::CIndexDict  loRequest;
    acl::CIndexDict* lpoResponse = APL_NULL;
    
    loRequest.Set(anf::BENC_TAG_CMD_ID, aiCommandID);
    loRequest.Set(GFQ_TAG_QUEUE_NAME, apcQueueName);

    if (auOldSegmentID != INVALID_ID)
    {
        loRequest.Set(GFQ_TAG_OLD_SEGMENT_ID, auOldSegmentID);
        loRequest.Set(GFQ_TAG_OLD_NODE_NAME, apcOldNodeName);
        loRequest.Set(GFQ_TAG_OLD_SEGMENT_STATUS, aiOldSegmentStatus);
    }

    liRetCode = DoRequest(this->moMaster, loRequest, &lpoResponse, this->moTimedout);
    if (liRetCode != 0)
    {
        return liRetCode;
    }

    aoSegmentInfo.moErrorCache.Update(0);
    aoSegmentInfo.muSegmentID          = lpoResponse->GetInt(GFQ_TAG_SEGMENT_ID, INVALID_ID);
    aoSegmentInfo.mi64Timestamp        = lpoResponse->GetInt(GFQ_TAG_TIMESTAMP, 0);
    apl_strncpy(aoSegmentInfo.macServerNodeName, lpoResponse->Get(GFQ_TAG_NODE_NAME, ""), sizeof(aoSegmentInfo.macQueueName) );
    apl_strncpy(aoSegmentInfo.macQueueName, apcQueueName, sizeof(aoSegmentInfo.macQueueName) );
    
    if ( (liRetCode = this->ConnectToSegmentServer( 
        lpoResponse->Get(GFQ_TAG_NODE_SERVER_IP, ""),
        (apl_int_t)lpoResponse->GetInt(GFQ_TAG_NODE_SERVER_PORT, 0),
        &(aoSegmentInfo.mpoSegmentServer) ) ) != 0 )
    {
        ACL_DELETE(lpoResponse);

        return this->RequestQueueSegment(
            aiCommandID,
            apcQueueName, 
            aoSegmentInfo.muSegmentID, 
            aoSegmentInfo.macServerNodeName, 
            liRetCode, 
            aoSegmentInfo );
    }
    else
    {
        ACL_DELETE(lpoResponse);
        return 0;
    }
}

apl_int_t CGFQueueClientImpl::ConnectToSegmentServer( 
    const char* apcServerIp, 
    apl_int_t aiPort, 
    ClientType** appoSegmentServer )
{
    acl::TSmartLock<acl::CLock> loGuard(this->moLock);
        
    char lacIpPort[32];
    apl_snprintf(lacIpPort, sizeof(lacIpPort), "%s:%d", apcServerIp, aiPort );

    std::map<std::string, ClientType*>::iterator loIter = this->moServers.find(lacIpPort);
    if (loIter == this->moServers.end() )
    {
        apl_int_t      liRetCode = 0;
        acl::CSockAddr loRemote(aiPort, apcServerIp); 
        ClientType*    lpoSegmentServer = NULL;
        ACL_NEW_ASSERT(lpoSegmentServer, ClientType);
    
        lpoSegmentServer->GetFilterChain()->AddFirst("bencoding-message", new anf::CBencodingMessageFilter);
        lpoSegmentServer->GetConnector()->SetConfiguration(&this->moConfiguration);
        lpoSegmentServer->GetConnector()->SetProcessor(this->mpoProcessor);

        if ( (liRetCode = lpoSegmentServer->Initialize(this->muWindSize, this->moTimedout) ) != 0)
        {
            apl_errprintf( "GFQ2 initialize segment server client fail, (Remote=%s:%"APL_PRIdINT",RetCode=%"APL_PRIdINT")\n", 
                apcServerIp, aiPort, liRetCode );

            ACL_DELETE(lpoSegmentServer);
            
            return ERROR_SYSTEM;
        }

        if ( (liRetCode = lpoSegmentServer->Connect("main", loRemote, this->muConnNum) ) <= 0)
        {
            apl_errprintf( "GFQ2 connect to segment server fail, (Remote=%s:%"APL_PRIdINT",RetCode=%"APL_PRIdINT")\n", 
                apcServerIp, aiPort, liRetCode );
            
            ACL_DELETE(lpoSegmentServer);
            
            return ERROR_CONNECT_REJECT;
        }

        this->moServers[lacIpPort] = lpoSegmentServer;

        *appoSegmentServer = lpoSegmentServer;
    }
    else
    {
        *appoSegmentServer = loIter->second;
    }
    
    return 0;
}

AIBC_GFQ_NAMESPACE_END


