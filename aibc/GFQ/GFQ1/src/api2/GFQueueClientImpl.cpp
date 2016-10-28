
#include "GFQueueClientImpl.h"
#include "acl/Process.h"

AIBC_GFQ_NAMESPACE_START

// process request error
#define GFQ_DECODE_RESPONSE( response, header, body, ret )       \
    header.Decode(response);                                        \
    if ( header.GetCmd() == GFQ_RESPONSE_ERROR )                 \
    {                                                               \
        CResponse loResponseBody;                                 \
        loResponseBody.Decode( response );                          \
                                                                    \
        ret = loResponseBody.GetStat();                             \
    }                                                               \
    else                                                            \
    {                                                               \
        body.Decode( response );                                    \
        ret = body.GetStat();                                       \
    }

// do request to server
template<class AsyncClient, class TRequest, class TResponse>
apl_int_t DoRequestTo( 
    AsyncClient& aoAsyncClient, 
    apl_int_t aiRequestCmd, 
    TRequest& aoRequest, 
    TResponse& aoResponse, 
    acl::CTimeValue const& aoTimeout  )
{
    anf::CSimpleAsyncMessage  loRequest;
    anf::CSimpleAsyncMessage* lpoResponse = NULL;
    CMessageHeader    loHeader(aiRequestCmd, &aoRequest);
    CMessageHeader    loRespHeader;
    apl_int_t liRet = NO_ERROR;
    
    loRequest.GetBody().Resize(loHeader.GetSize() );
    loHeader.Encode(loRequest.GetBody() );
    
    if ( ( liRet = aoAsyncClient.Request(loRequest, &lpoResponse, aoTimeout) ) != 0 )
    {
        apl_errprintf( "GFQApi send request fail, (Remote=%s:%"APL_PRIdINT",RetCode=%"APL_PRIdINT",CommandID=0x%X)\n",
                aoAsyncClient.moRemoteAddress.GetIpAddr(),
                (apl_int_t)aoAsyncClient.moRemoteAddress.GetPort(),
                liRet,
                aiRequestCmd);
        
        return ERROR_SOCKET_SEND;
    }
    
    GFQ_DECODE_RESPONSE( lpoResponse->GetBody(), loRespHeader, aoResponse, liRet );

    ACL_DELETE(lpoResponse);
    
    return liRet;
}

//// class define
//////////////////////////////////////////////////////////////////////////////////////
CErrorCache::CErrorCache()
    : miErrNo(NO_ERROR)
    , miTimeInterval(0)
{
}

bool CErrorCache::IsHitCache()
{
    if ( (this->miErrNo == ERROR_END_OF_QUEUE 
        || this->miErrNo == ERROR_NO_ENOUGH_SPACE ) )
    {
        acl::CTimestamp loCurrTime;
        loCurrTime.Update(acl::CTimestamp::PRC_MSEC);

        if (loCurrTime.Msec() - this->moTimestamp.Msec() < this->miTimeInterval)
        {
            return true;
        }
    }
    
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////
CSegmentInfo::CSegmentInfo(void)
    : miSegmentID(-1)
	, miModuleID(-1)
	, miQueueGlobalID(-1)
	, mpoSegmentServer(NULL)
{
    apl_memset(this->macQueueName, 0, sizeof(this->macQueueName) );
}

//////////////////////////////////////////////////////////////////////////////////////
CGFQueueClientImpl::CGFQueueClientImpl(void) 
    : muConnNum(1)
    , muWindSize(1000)
    , moTimeout(6)
    , miErrorCacheInterval(2000)
{
    apl_memset(this->macUserName, 0, sizeof(this->macUserName) );
}

CGFQueueClientImpl::~CGFQueueClientImpl(void)
{
}
	
apl_int_t CGFQueueClientImpl::Initialize(
    char const* apcName,
    acl::CSockAddr const& aoRemoteAddress, 
    apl_size_t auConnNum, 
    acl::CTimeValue const& aoTimeout,
    apl_size_t auWindSize,
    bool abIsIgnoreConnectFail )
{
    this->moMaster.GetFilterChain()->AddFirst("simple-async-message", new anf::CSimpleAsyncMessageFilter);
    this->moMaster.GetConfiguration()->SetReadIdleTime(30);
    this->moMaster.GetConfiguration()->SetWriteIdleTime(3);
    this->moMaster.GetConfiguration()->SetWriteBufferSize(MAX_DATA_LEN + 512);
    this->moMaster.GetConfiguration()->SetReadBufferSize(MAX_DATA_LEN + 512);

    if (this->moMaster.Initialize(auWindSize, aoTimeout) != 0)
    {
        return -1;
    }

	if (this->moMaster.Connect("main", aoRemoteAddress, auConnNum, true) <= 0 && !abIsIgnoreConnectFail )
	{
		return -1;
	}
    
    this->moMaster.moRemoteAddress = aoRemoteAddress;
    this->muConnNum  = auConnNum;
    this->muWindSize = auWindSize;
    this->moTimeout  = aoTimeout;

    apl_snprintf(this->macUserName, sizeof(this->macUserName),
        "%s-%"APL_PRIu64, apcName, (apl_uint64_t)acl::CProcess::GetPid() );
	
	return NO_ERROR;
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

    this->moServers.clear();
}

void CGFQueueClientImpl::SetErrorCacheInterval( apl_int_t aiInterval )
{
    this->miErrorCacheInterval = aiInterval;
}

apl_int_t CGFQueueClientImpl::GetErrorCacheInterval(void)
{
    return this->miErrorCacheInterval;
}

apl_int_t CGFQueueClientImpl::Get( const char* apcQueueName, acl::CMemoryBlock* apoData, apl_int_t aiWhenSec /* = 0 */)
{
	apl_int_t    liRet = NO_ERROR;
	CRequestGet  loReqGet;
	CResponseGet loRespGet;
	CSegmentInfo loSegmentInfo;
	
	if (this->GetSegmentInfo(this->moReadableSegments, apcQueueName, loSegmentInfo) != NO_ERROR )
	{
	    // Check error cache, if hit and return it immediately
	    if (loSegmentInfo.moErrorCache.IsHitCache() )
	    {
	        liRet = loSegmentInfo.moErrorCache.miErrNo;
	        
	        return liRet;
	    }
	    
		if ( (liRet = this->GetReadSegment(apcQueueName, loSegmentInfo) ) != NO_ERROR )
		{
			return liRet;
		}
	}
	
	for( int liIt = 0; liIt < DEAD_CIRCLE_NUM; liIt++ ) //Ignore dead circle
	{
		loReqGet.SetSegmentID(loSegmentInfo.miSegmentID);
		loReqGet.SetQueueGlobalID(loSegmentInfo.miQueueGlobalID);
		loReqGet.SetQueueName(loSegmentInfo.macQueueName);
		loReqGet.SetWhenSec(aiWhenSec);
		loReqGet.SetUserName(this->macUserName);
		
		liRet = DoRequestTo(
            *loSegmentInfo.mpoSegmentServer, 
            GFQ_REQUEST_GET, 
            loReqGet, 
            loRespGet,
            this->moTimeout );
		if (liRet == NO_ERROR )
		{
            if (apoData != NULL)
            {
                loRespGet.GetData(*apoData);
            }
						
			return liRet;
		}

		// Dead circle, is impossible, it will be break and save ret code liRet
		if (liIt >= DEAD_CIRCLE_NUM - 1) break;
		
		if ( (liRet = this->GetReadSegmentSecond(apcQueueName, liRet, loSegmentInfo) ) != NO_ERROR)
		{
			return liRet;
		}
		//Yes, second get read segment success
	}
	
    // Break, dead circle
	return liRet;
}

apl_int_t CGFQueueClientImpl::Put( const char* apcQueueName, acl::CMemoryBlock const& aoData )
{
	apl_int_t liRet = NO_ERROR;
	CRequestPut  loReqPut;
	CResponsePut loRespPut;
	CSegmentInfo loSegmentInfo;
	
	// Data length check
	if ( aoData.GetSize() <= 0 || aoData.GetLength() > MAX_DATA_LEN )
	{
		return ERROR_OUT_OF_RANGE;
	}
	
	if (this->GetSegmentInfo(this->moWriteableSegments, apcQueueName, loSegmentInfo) != NO_ERROR)
	{
	    // Check error cache, if hit and return it immediately
	    if (loSegmentInfo.moErrorCache.IsHitCache() )
	    {
	        liRet = loSegmentInfo.moErrorCache.miErrNo;
	        
	        return liRet;
	    }
	    
		if ( (liRet = this->GetWriteSegment(apcQueueName, loSegmentInfo) ) != NO_ERROR )
		{
			return liRet;
		}
	}
	
	for( int liIt = 0; liIt < DEAD_CIRCLE_NUM; liIt++ ) //Ignore dead circle
	{
		loReqPut.SetSegmentID(loSegmentInfo.miSegmentID);
		loReqPut.SetQueueGlobalID(loSegmentInfo.miQueueGlobalID);
		loReqPut.SetQueueName(loSegmentInfo.macQueueName);
		loReqPut.SetUserName(this->macUserName);
		loReqPut.SetData(aoData);
		
		liRet = DoRequestTo(*loSegmentInfo.mpoSegmentServer, GFQ_REQUEST_PUT, loReqPut, loRespPut, this->moTimeout);
		//liRet = AI_ERROR_SOCKET_RECV;
		if (liRet == NO_ERROR)
		{
			return liRet;
		}

		// Dead circle, is impossible, it will be break and save ret code liRet
		if (liIt >= DEAD_CIRCLE_NUM - 1) break;

		if ( (liRet = this->GetWriteSegmentSecond(apcQueueName, liRet, loSegmentInfo) ) != NO_ERROR)
		{
			return liRet;
		}
		//Yes, second get read segment success
	}
	
    // Break, dead circle
	return liRet;
}

apl_int_t CGFQueueClientImpl::GetStat( const char* apcQueueName, apl_int_t* apiSize, apl_int_t* apiCapaciy )
{
	CRequestQueueStat  loRequestStat;
	CResponseQueueStat loResponseStat;
	apl_int_t liRet = NO_ERROR;

	loRequestStat.SetUserName(this->macUserName);
	loRequestStat.SetQueueName(apcQueueName);
	
	liRet = DoRequestTo(this->moMaster, GFQ_REQUEST_QUEUE_STAT, loRequestStat, loResponseStat, this->moTimeout);
	
	if (liRet == NO_ERROR)
	{
		if (apiSize != NULL) 
        {
            *apiSize = loResponseStat.GetQueueSize();
        }

		if (apiCapaciy != NULL)
        {
            *apiCapaciy = loResponseStat.GetQueueCapacity();
        }
	}
	
	return liRet;
}
 
apl_int_t CGFQueueClientImpl::GetModuleCount( apl_int_t* apiCount )
{
	CRequestModuleCount  loRequest;
	CResponseModuleCount loResponse;
	apl_int_t liRet = NO_ERROR;

	loRequest.SetUserName(this->macUserName);
	
	liRet = DoRequestTo(this->moMaster, GFQ_REQUEST_MODULE_COUNT, loRequest, loResponse, this->moTimeout);
	
	if (liRet == NO_ERROR)
	{
		if (apiCount != NULL) 
        {
            *apiCount = loResponse.GetModuleCount();
        }
	}
	
	return liRet;
}

apl_int_t CGFQueueClientImpl::ReloadQueueInfo(void)
{
    CRequestReloadQueueInfo  loRequest;
	CResponseReloadQueueInfo loResponse;
	apl_int_t liRet = NO_ERROR;
	
	loRequest.SetUserName(this->macUserName);
	
	liRet = DoRequestTo(this->moMaster, GFQ_REQUEST_RELOADQUEUEINFO, loRequest, loResponse, this->moTimeout);
	
	return liRet;
}

apl_int_t CGFQueueClientImpl::PrintQueues(void)
{
    CRequestPrintQueue  loRequest;
	CResponsePrintQueue loResponse;
	apl_int_t liRet = NO_ERROR;
	
	loRequest.SetUserName(this->macUserName);

	liRet = DoRequestTo(this->moMaster, GFQ_REQUEST_PRINT_QUEUES, loRequest, loResponse, this->moTimeout);
	
	return liRet;
}
    
apl_int_t CGFQueueClientImpl::PrintModules(void)
{
    CRequestPrintModule  loRequest;
	CResponsePrintModule loResponse;
	apl_int_t liRet = NO_ERROR;
	
	loRequest.SetUserName(this->macUserName);

	liRet = DoRequestTo(this->moMaster, GFQ_REQUEST_PRINT_MODULES, loRequest, loResponse, this->moTimeout);
	
	return liRet;
}

apl_int_t CGFQueueClientImpl::GetWriteSegment( const char* apcQueueName, CSegmentInfo& aoSegmentInfo )
{
    apl_int_t liRet = NO_ERROR;
	CRequestWriteSeg  loReqWriteSeg;
	CResponseWriteSeg loRespWriteSeg;
	
	loReqWriteSeg.SetUserName(this->macUserName);
	loReqWriteSeg.SetQueueName(apcQueueName);
	
	{
		liRet = DoRequestTo(this->moMaster, GFQ_REQUEST_WRITESEG, loReqWriteSeg, loRespWriteSeg, this->moTimeout);
		if (liRet != NO_ERROR)
		{
		    aoSegmentInfo.moErrorCache.miErrNo        = liRet;
		    aoSegmentInfo.moErrorCache.moTimestamp.Update(acl::CTimestamp::PRC_MSEC);
		    aoSegmentInfo.moErrorCache.miTimeInterval = this->miErrorCacheInterval;
		    this->SetSegmentInfo(this->moWriteableSegments, apcQueueName, aoSegmentInfo);
		    
			return liRet;
		}

        aoSegmentInfo.moErrorCache.miErrNo = liRet;
		aoSegmentInfo.miSegmentID          = loRespWriteSeg.GetSegmentID();
		aoSegmentInfo.miModuleID           = loRespWriteSeg.GetModuleID();
		aoSegmentInfo.miQueueGlobalID      = loRespWriteSeg.GetQueueGlobalID();
		apl_strncpy(aoSegmentInfo.macQueueName, apcQueueName, sizeof(aoSegmentInfo.macQueueName) );
		
		if ( (liRet = this->CreateSegmentServerConnection( 
			loRespWriteSeg.GetIpAddr(),
            loRespWriteSeg.GetPort(),
            &(aoSegmentInfo.mpoSegmentServer) ) ) != NO_ERROR )
		{
			return this->GetWriteSegmentSecond(apcQueueName, liRet, aoSegmentInfo );
		}

		this->SetSegmentInfo(this->moWriteableSegments, apcQueueName, aoSegmentInfo );
	}
	
	return NO_ERROR;
}

apl_int_t CGFQueueClientImpl::GetReadSegment( const char* apcQueueName, CSegmentInfo& aoSegmentInfo )
{
	apl_int_t liRet = NO_ERROR;
	CRequestReadSeg  loReqReadSeg;
	CResponseReadSeg loRespReadSeg;
	
	loReqReadSeg.SetUserName(this->macUserName);
	loReqReadSeg.SetQueueName(apcQueueName);
	
	{
		liRet = DoRequestTo(this->moMaster, GFQ_REQUEST_READSEG, loReqReadSeg, loRespReadSeg, this->moTimeout);
		if (liRet != NO_ERROR)
		{
		    aoSegmentInfo.moErrorCache.miErrNo        = liRet;
		    aoSegmentInfo.moErrorCache.moTimestamp.Update(acl::CTimestamp::PRC_MSEC);
		    aoSegmentInfo.moErrorCache.miTimeInterval = this->miErrorCacheInterval;
		    this->SetSegmentInfo(this->moReadableSegments, apcQueueName, aoSegmentInfo);
		    
			return liRet;
		}
        
        aoSegmentInfo.moErrorCache.miErrNo = liRet;
		aoSegmentInfo.miSegmentID          = loRespReadSeg.GetSegmentID();
		aoSegmentInfo.miModuleID           = loRespReadSeg.GetModuleID();
		aoSegmentInfo.miQueueGlobalID      = loRespReadSeg.GetQueueGlobalID();
		apl_strncpy(aoSegmentInfo.macQueueName, apcQueueName, sizeof(aoSegmentInfo.macQueueName) );

		if ( (liRet = this->CreateSegmentServerConnection( 
			loRespReadSeg.GetIpAddr(),
            loRespReadSeg.GetPort(), 
            &(aoSegmentInfo.mpoSegmentServer) ) ) != NO_ERROR )
		{
			return this->GetReadSegmentSecond(apcQueueName, liRet, aoSegmentInfo);
		}
		
	    this->SetSegmentInfo(this->moReadableSegments, apcQueueName, aoSegmentInfo);
	}
	
	return NO_ERROR;
}

apl_int_t CGFQueueClientImpl::GetWriteSegmentSecond( const char* apcQueueName, apl_int_t aiRetcode, CSegmentInfo& aoSegmentInfo )
{
	apl_int_t liRet = NO_ERROR;
	CRequestWriteSegSecond  loReqWriteSeg;
	CResponseWriteSegSecond loRespWriteSeg;
	
	loReqWriteSeg.SetUserName(this->macUserName);
	loReqWriteSeg.SetQueueName(apcQueueName);
	loReqWriteSeg.SetSegmentID(aoSegmentInfo.miSegmentID);
	loReqWriteSeg.SetModuleID(aoSegmentInfo.miModuleID);
	loReqWriteSeg.SetRetcode(aiRetcode );

	do
	{
		liRet = DoRequestTo(this->moMaster, GFQ_REQUEST_WRITESEG_SECOND, loReqWriteSeg, loRespWriteSeg, this->moTimeout);
		if (liRet != NO_ERROR)
		{
		    aoSegmentInfo.moErrorCache.miErrNo        = liRet;
		    aoSegmentInfo.moErrorCache.moTimestamp.Update(acl::CTimestamp::PRC_MSEC);
		    aoSegmentInfo.moErrorCache.miTimeInterval = this->miErrorCacheInterval;
		    this->SetSegmentInfo(this->moWriteableSegments, apcQueueName, aoSegmentInfo);
		    
			return liRet;
		}
		
		aoSegmentInfo.moErrorCache.miErrNo = liRet;
		aoSegmentInfo.miSegmentID          = loRespWriteSeg.GetSegmentID();
		aoSegmentInfo.miModuleID           = loRespWriteSeg.GetModuleID();
		aoSegmentInfo.miQueueGlobalID      = loRespWriteSeg.GetQueueGlobalID();

		if ( ( liRet = this->CreateSegmentServerConnection( 
			loRespWriteSeg.GetIpAddr(),
            loRespWriteSeg.GetPort(), 
            &(aoSegmentInfo.mpoSegmentServer) ) ) != NO_ERROR )
		{
			return this->GetWriteSegmentSecond(apcQueueName, liRet, aoSegmentInfo);
		}

		this->SetSegmentInfo(this->moWriteableSegments, apcQueueName, aoSegmentInfo);
		
		return NO_ERROR;
		
	}while(false);
	
	return liRet;
}

apl_int_t CGFQueueClientImpl::GetReadSegmentSecond( const char* apcQueueName, apl_int_t aiRetcode, CSegmentInfo& aoSegmentInfo )
{
	apl_int_t liRet = NO_ERROR;
	CRequestReadSegSecond  loReqReadSeg;
	CResponseReadSegSecond loRespReadSeg;
	
	loReqReadSeg.SetUserName(this->macUserName);
	loReqReadSeg.SetQueueName( apcQueueName );
	loReqReadSeg.SetSegmentID( aoSegmentInfo.miSegmentID );
	loReqReadSeg.SetModuleID( aoSegmentInfo.miModuleID );
	loReqReadSeg.SetRetcode( aiRetcode );

	do
	{
		liRet = DoRequestTo(this->moMaster, GFQ_REQUEST_READSEG_SECOND, loReqReadSeg, loRespReadSeg, this->moTimeout);
		if (liRet != NO_ERROR)
		{
		    aoSegmentInfo.moErrorCache.miErrNo        = liRet;
		    aoSegmentInfo.moErrorCache.moTimestamp.Update(acl::CTimestamp::PRC_MSEC);
		    aoSegmentInfo.moErrorCache.miTimeInterval = this->miErrorCacheInterval;
		    SetSegmentInfo(this->moReadableSegments, apcQueueName, aoSegmentInfo );
		    
			return liRet;
		}
		
		aoSegmentInfo.moErrorCache.miErrNo = liRet;
		aoSegmentInfo.miSegmentID          = loRespReadSeg.GetSegmentID();
		aoSegmentInfo.miModuleID           = loRespReadSeg.GetModuleID();
		aoSegmentInfo.miQueueGlobalID      = loRespReadSeg.GetQueueGlobalID();

		if ( ( liRet = this->CreateSegmentServerConnection( 
			loRespReadSeg.GetIpAddr(),
            loRespReadSeg.GetPort(), 
            &(aoSegmentInfo.mpoSegmentServer) ) ) != NO_ERROR )
		{
			return this->GetReadSegmentSecond(apcQueueName, liRet, aoSegmentInfo);
		}
		
		SetSegmentInfo(this->moReadableSegments, apcQueueName, aoSegmentInfo);
		
		return NO_ERROR;
		
	}while(false);
	
	return liRet;
}

apl_int_t CGFQueueClientImpl::SetSegmentInfo( CSegmentInfoMap& aoSegmentInfoMap, const char* apcQueueName, CSegmentInfo& aoSegmentInfo )
{
	// lock scope
    acl::TSmartLock<acl::CLock> loGuard(aoSegmentInfoMap.moLock);

    CSegmentInfo& loSegmentInfo = aoSegmentInfoMap.moMap[apcQueueName];

    loSegmentInfo.miSegmentID      = aoSegmentInfo.miSegmentID;
    loSegmentInfo.miModuleID       = aoSegmentInfo.miModuleID;
    loSegmentInfo.miQueueGlobalID  = aoSegmentInfo.miQueueGlobalID;
    loSegmentInfo.mpoSegmentServer = aoSegmentInfo.mpoSegmentServer;
    loSegmentInfo.moErrorCache     = aoSegmentInfo.moErrorCache;
    apl_strncpy( loSegmentInfo.macQueueName, apcQueueName, sizeof(loSegmentInfo.macQueueName) );
		
	return NO_ERROR;
}

apl_int_t CGFQueueClientImpl::GetSegmentInfo( CSegmentInfoMap& aoSegmentInfoMap, const char* apcQueueName, CSegmentInfo& aoSegmentInfo )
{
	// lock scope
    acl::TSmartLock<acl::CLock> loGuard(aoSegmentInfoMap.moLock);

    std::map<std::string, CSegmentInfo>::iterator loIter = aoSegmentInfoMap.moMap.find(apcQueueName);

    if (loIter != aoSegmentInfoMap.moMap.end() )
    {
        CSegmentInfo& loSegmentInfo = loIter->second;

        aoSegmentInfo.miSegmentID      = loSegmentInfo.miSegmentID;
        aoSegmentInfo.miModuleID       = loSegmentInfo.miModuleID;
        aoSegmentInfo.miQueueGlobalID  = loSegmentInfo.miQueueGlobalID;
        aoSegmentInfo.mpoSegmentServer = loSegmentInfo.mpoSegmentServer;
        aoSegmentInfo.moErrorCache     = loSegmentInfo.moErrorCache;
        apl_strncpy( aoSegmentInfo.macQueueName, apcQueueName, sizeof(aoSegmentInfo.macQueueName) );
	    
        return aoSegmentInfo.moErrorCache.miErrNo;
    }
	
    return ERROR_NO_FOUND_QUEUE;
}

apl_int_t CGFQueueClientImpl::CreateSegmentServerConnection( const char* apcServerIp, apl_int_t aiPort, ClientType** appoSegmentServer )
{
	// lock scope
    acl::TSmartLock<acl::CLock> loGuard(this->moLock);
		
	char lacIpPort[IP_ADDR_LEN];
	apl_snprintf(lacIpPort, sizeof(lacIpPort), "%s:%d", apcServerIp, aiPort );

    std::map<std::string, ClientType*>::iterator loIter = this->moServers.find(lacIpPort);
	if (loIter == this->moServers.end() )
	{
        apl_int_t      liRetCode = 0;
        acl::CSockAddr loRemote(aiPort, apcServerIp); 
		ClientType*    lpoSegmentServer = NULL;
        ACL_NEW_ASSERT(lpoSegmentServer, ClientType);
    
        lpoSegmentServer->moRemoteAddress = loRemote;
        lpoSegmentServer->GetFilterChain()->AddFirst("simple-async-message", new anf::CSimpleAsyncMessageFilter);
        lpoSegmentServer->GetConfiguration()->SetWriteBufferSize(MAX_DATA_LEN + 512);
        lpoSegmentServer->GetConfiguration()->SetReadBufferSize(MAX_DATA_LEN + 512);
        lpoSegmentServer->GetConfiguration()->SetReadIdleTime(30);
        lpoSegmentServer->GetConfiguration()->SetWriteIdleTime(3);
        lpoSegmentServer->GetConfiguration()->SetProcessorPoolSize(this->muConnNum);

        if ( (liRetCode = lpoSegmentServer->Initialize(this->muWindSize, this->moTimeout) ) != 0)
        {
		    apl_errprintf( "GFQApi initialize segment server client fail, (Remote=%s:%"APL_PRIdINT",RetCode=%"APL_PRIdINT")", 
                apcServerIp, aiPort, liRetCode );

            ACL_DELETE(lpoSegmentServer);
            
            return ERROR_SYSTEM;
        }

        if ( (liRetCode = lpoSegmentServer->Connect("main", loRemote, this->muConnNum) ) <= 0)
        {
		    apl_errprintf( "GFQApi connect to segment server fail, (Remote=%s:%"APL_PRIdINT",RetCode=%"APL_PRIdINT")", 
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
	
	return NO_ERROR;
}

AIBC_GFQ_NAMESPACE_END

