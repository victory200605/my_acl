
#include "aaf/LogService.h"
#include "anf/IoSockAcceptor.h"
#include "acl/IndexDict.h"
#include "acl/TimerQueueAdapter.h"
#include "ClientHandler.h"
#include "ServerHandler.h"

AIBC_GFQ_NAMESPACE_START

/** 
 * @brief Client handler timer handle adapter for segment info sync
 */
class CSegmentSyncHandler : public acl::IEventHandler
{
public:
    CSegmentSyncHandler( CClientHandler* apcHandler, apl_int_t aiInterval )
        : mpoHandler(apcHandler)
    {
        this->miTimerID = acl::Instance<acl::CTimerQueueAdapter>()->Schedule(
            this, 
            APL_NULL, 
            acl::CTimeValue(aiInterval), 
            acl::CTimeValue(aiInterval) );
        ACL_ASSERT(this->miTimerID != -1);
    }
    
    virtual ~CSegmentSyncHandler(void)
    {
        acl::Instance<acl::CTimerQueueAdapter>()->Cancel(this->miTimerID, APL_NULL);
    }
    
    virtual apl_int_t HandleTimeout( acl::CTimestamp const& aoCurrTime, void const* apcAct )
    {
        anf::SessionPtrType loSession = this->mpoHandler->GetSession();
        
        if (loSession != APL_NULL)
        {
            this->mpoHandler->SendUpdateSegmentRequest(loSession);
        }

        return 0;
    }

private:
    CClientHandler* mpoHandler;
    
    apl_int_t miTimerID;
};

/** 
 * @brief Client handler timer handle adapter for server stat info sync
 */
class CStatInfoSyncHandler : public acl::IEventHandler
{
public:
    CStatInfoSyncHandler( CClientHandler* apoHandler, apl_int_t aiInterval )
        : mpoHandler(apoHandler)
        , miInterval(aiInterval)
    {
        this->miTimerID = acl::Instance<acl::CTimerQueueAdapter>()->Schedule(
            this, 
            APL_NULL, 
            acl::CTimeValue(aiInterval), 
            acl::CTimeValue(aiInterval) );
        ACL_ASSERT(this->miTimerID != -1);
    }
    
    virtual ~CStatInfoSyncHandler(void)
    {
        acl::Instance<acl::CTimerQueueAdapter>()->Cancel(this->miTimerID, APL_NULL);
    }
    
    virtual apl_int_t HandleTimeout( acl::CTimestamp const& aoCurrTime, void const* apcAct )
    {
        anf::SessionPtrType loSession = this->mpoHandler->GetSession();
    
        if (loSession != APL_NULL && !loSession->IsClosing() )
        {
            this->mpoHandler->SendUpdateStatInfoRequest(loSession, this->miInterval);
        }

        return 0;
    }

private:
    CClientHandler* mpoHandler;

    apl_int_t miInterval;
    
    apl_int_t miTimerID;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////
CClientHandler::CClientHandler( 
    char const* apcNodeName,
    char const* apcGroupName,
    char const* apcStorageType,
    char const* apcServerIp,
    apl_uint16_t au16ServerPort,
    apl_int_t aiNodePriority,
    apl_int_t aiSegmentInfoSyncInterval,
    apl_int_t aiStatInfoSyncInterval,
    ISegmentQueue* apoSegmentQueue )
    : moNodeName(apcNodeName)
    , moGroupName(apcGroupName)
    , moStorageType(apcStorageType)
    , moServerIp(apcServerIp)
    , mu16ServerPort(au16ServerPort)
    , miNodePriority(aiNodePriority)
    , mpoSegmentQueue(apoSegmentQueue)
    , mbIsOnlined(false)
{
    ACL_NEW_ASSERT(this->mpoSegmentSyncHandler, CSegmentSyncHandler(this, aiSegmentInfoSyncInterval) );
    
    ACL_NEW_ASSERT(this->mpoStatInfoSyncHandler, CStatInfoSyncHandler(this, aiStatInfoSyncInterval) );
}

CClientHandler::~CClientHandler(void)
{
    ACL_DELETE(this->mpoSegmentSyncHandler);

    ACL_DELETE(this->mpoStatInfoSyncHandler);
}

anf::SessionPtrType CClientHandler::GetSession(void)
{
    return this->moSession;
}
    
void CClientHandler::Exception( anf::CThrowable const& aoCause )
{
    AAF_LOG_ERROR("ClientHandler catch a exception,%s (State=%"APL_PRIdINT",Errno=%"APL_PRIdINT")",
        aoCause.GetMessage(),
        (apl_int_t)aoCause.GetState(),
        (apl_int_t)aoCause.GetErrno() )
}
    
void CClientHandler::SessionException( anf::SessionPtrType& aoSession, anf::CThrowable const& aoCause )
{
    AAF_LOG_ERROR("ClientHandler catch session exception,%s (Remote=%s:%"APL_PRIdINT",SessionID=%"APL_PRIu64",State=%"APL_PRIdINT",Errno=%"APL_PRIdINT")",
        aoCause.GetMessage(),
        aoSession->GetRemoteAddress().GetIpAddr(),
        (apl_int_t)aoSession->GetRemoteAddress().GetPort(),
        (apl_uint64_t)aoSession->GetSessionID(),
        (apl_int_t)aoCause.GetState(),
        (apl_int_t)aoCause.GetErrno() );
}

void CClientHandler::SessionOpened( anf::SessionPtrType& aoSession )
{
    AAF_LOG_INFO("ClientHandler add connection to master, (Remote=%s:%"APL_PRIdINT",SessionID=%"APL_PRIu64")",
        aoSession->GetRemoteAddress().GetIpAddr(),
        (apl_int_t)aoSession->GetRemoteAddress().GetPort(),
        (apl_uint64_t)aoSession->GetSessionID() );

    //Connect to master success and start to register server node now
    this->mbIsOnlined = false;

    this->SendRegisterRequest(aoSession);
}

void CClientHandler::SessionClosed( anf::SessionPtrType& aoSession )
{
    AAF_LOG_INFO("ClientHandler closed connection to master, (Remote=%s:%"APL_PRIdINT",SessionID=%"APL_PRIu64")",
        aoSession->GetRemoteAddress().GetIpAddr(),
        (apl_int_t)aoSession->GetRemoteAddress().GetPort(),
        (apl_uint64_t)aoSession->GetSessionID() );
    
    //Close connection and server node is offlined
    this->mbIsOnlined = false;
}

void CClientHandler::SessionIdle( anf::SessionPtrType& aoSession, apl_int_t aiStatus )
{
    if (!this->mbIsOnlined)
    {
        aoSession->Close(aoSession);
        return;
    }

    switch(aiStatus)
    {
        case anf::SESSION_WRITE_IDLE:
        {
            this->SendHeartbeatRequest(aoSession);
            break;
        }
    };
}

void CClientHandler::MessageReceived( anf::SessionPtrType& aoSession, acl::CAny const& aoMessage )
{
    acl::CIndexDict* lpoMessage = NULL;
    
    //Cast message to scp message type
    aoMessage.CastTo<acl::CIndexDict*>(lpoMessage);
    if (lpoMessage == NULL)//Exception
    {
        //Exception, invalid message
        AAF_LOG_ERROR("Clienthandler session received invalid message, (Remote=%s:%"APL_PRIdINT",SessionID=%"APL_PRIu64")",
            aoSession->GetRemoteAddress().GetIpAddr(),
            (apl_int_t)aoSession->GetRemoteAddress().GetPort(),
            (apl_uint64_t)aoSession->GetSessionID() );

        aoSession->Close(aoSession);
        return;
    }
   
    apl_int_t liCommandID = lpoMessage->GetInt(anf::BENC_TAG_CMD_ID, 0);
    switch(liCommandID)
    {
        case GFQ_REQUEST_ALLOCATE:
        {
            //TODO segment allocate
            this->HandleAllocateRequest(aoSession, lpoMessage);
            break;
        }
        case GFQ_REQUEST_DEALLOCATE:
        {
            //TODO segment deallocate
            this->HandleDeallocateRequest(aoSession, lpoMessage);
            break;
        }
        case GFQ_RESPONSE_UPDATE_SEGMENT:
        {
            //TODO segment size
            this->HandleUpdateSegmentResponse(aoSession, lpoMessage);
            break;
        };
        case GFQ_RESPONSE_UPDATE_STAT_INFO:
        {
            //TODO segment size
            this->HandleUpdateStatInfoResponse(aoSession, lpoMessage);
            break;
        };
        case GFQ_RESPONSE_REGISTER:
        {
            //TODO add server node
            this->HandleRegisterResponse(aoSession, lpoMessage);
            break;
        }
        case GFQ_RESPONSE_UNREGISTER:
        {
            //TODO del server node
            this->HandleUnregisterResponse(aoSession, lpoMessage);
            break;
        }
        case GFQ_RESPONSE_ADD_SEGMENT:
        {
            //TODO del server node
            this->HandleAddSegmentResponse(aoSession, lpoMessage);
            break;
        }
        case GFQ_RESPONSE_ONLINE:
        {
            //TODO del server node
            this->HandleOnlineResponse(aoSession, lpoMessage);
            break;
        }
        case GFQ_RESPONSE_HEARTBEAT:
        {
            //TODO del server node
            this->HandleHeartbeatResponse(aoSession, lpoMessage);
            break;
        }
        default:
        {
            //Exception, invalid message
            AAF_LOG_ERROR("Clienthandler session received invalid message, "
                "(CmdID=0x%X,Remote=%s:%"APL_PRIdINT",SessionID=%"APL_PRIu64")",
                liCommandID,
                aoSession->GetRemoteAddress().GetIpAddr(),
                (apl_int_t)aoSession->GetRemoteAddress().GetPort(),
                (apl_uint64_t)aoSession->GetSessionID() );

            aoSession->Close(aoSession);
        }
    };

    ACL_DELETE(lpoMessage);
}

void CClientHandler::MessageFinished( anf::SessionPtrType& aoSession, anf::CWriteFuture::PointerType& aoWriteFuture )
{
    if (aoWriteFuture->IsException() )
    {
        AAF_LOG_ERROR("ClientHandler message flush exception, "
            "(State=%"APL_PRIdINT",Errno=%"APL_PRIdINT",SessionID=%"APL_PRIu64")",
            (apl_int_t)aoWriteFuture->GetState(),
            (apl_int_t)aoWriteFuture->GetErrno(),
            (apl_uint64_t)aoSession->GetSessionID() );
    }
    else
    {
        AAF_LOG_TRACE("ClientHandler message flush finished, "
            "(MsgID=%"APL_PRIuINT",SessionID=%"APL_PRIu64")",
            (apl_size_t)aoWriteFuture->GetAct(),
            (apl_uint64_t)aoSession->GetSessionID() );
    }
}

//Send request method
void CClientHandler::SendRegisterRequest( anf::SessionPtrType& aoSession )
{
    acl::CIndexDict loRequest;

    loRequest.Set(GFQ_TAG_NODE_NAME, this->moNodeName.c_str() );
    loRequest.Set(GFQ_TAG_GROUP_NAME, this->moGroupName.c_str() );
    loRequest.Set(GFQ_TAG_NODE_STORAGE_TYPE, this->moStorageType.c_str() );
    loRequest.Set(GFQ_TAG_NODE_SERVER_IP, this->moServerIp.c_str() );
    loRequest.Set(GFQ_TAG_NODE_SERVER_PORT, this->mu16ServerPort );
    loRequest.Set(GFQ_TAG_NODE_PRIORITY, this->miNodePriority);
    
    AAF_LOG_DEBUG("%s", "ClientHandler send register request to master");

    this->WriteMessage(aoSession, 0, GFQ_REQUEST_REGISTER, loRequest);
}

void CClientHandler::SendAddSegmentRequest( anf::SessionPtrType& aoSession )
{
    std::vector<CSegmentInfo> loSegmentInfos;
    this->mpoSegmentQueue->GetAllSegmentInfo(loSegmentInfos);
    
    AAF_LOG_DEBUG("%s", "ClientHandler send add segment request to master");

    for (std::vector<CSegmentInfo>::iterator loIter = loSegmentInfos.begin();
         loIter != loSegmentInfos.end(); ++loIter)
    {
        acl::CIndexDict loRequest;

        loRequest.Set(GFQ_TAG_NODE_NAME, this->moNodeName.c_str() );
        loRequest.Set(GFQ_TAG_SEGMENT_ID, loIter->GetSegmentID() );
        loRequest.Set(GFQ_TAG_TIMESTAMP, loIter->GetTimestamp() );
        loRequest.Set(GFQ_TAG_QUEUE_NAME, loIter->GetQueueName() );
        loRequest.Set(GFQ_TAG_SEGMENT_SIZE, loIter->GetSize() );
        loRequest.Set(GFQ_TAG_SEGMENT_CAPACITY, loIter->GetCapacity() );
    
        this->WriteMessage(aoSession, 0, GFQ_REQUEST_ADD_SEGMENT, loRequest);
    }

    {//Request Online
        acl::CIndexDict loRequest;

        loRequest.Set(GFQ_TAG_NODE_NAME, this->moNodeName.c_str() );

        this->WriteMessage(aoSession, 0, GFQ_REQUEST_ONLINE, loRequest);
    }
}

void CClientHandler::SendUnregisterRequest( anf::SessionPtrType& aoSession )
{
    acl::CIndexDict loRequest;

    loRequest.Set(GFQ_TAG_NODE_NAME, this->moNodeName.c_str() );
    
    AAF_LOG_DEBUG("%s", "ClientHandler send unregister request to master");
        
    this->WriteMessage(aoSession, 0, GFQ_REQUEST_UNREGISTER, loRequest);
}

void CClientHandler::SendHeartbeatRequest( anf::SessionPtrType& aoSession )
{
    acl::CIndexDict loRequest;

    loRequest.Set(GFQ_TAG_NODE_NAME, this->moNodeName.c_str() );
    
    AAF_LOG_DEBUG("%s", "ClientHandler send heartbeat request to master");

    this->WriteMessage(aoSession, 0, GFQ_REQUEST_HEARTBEAT, loRequest);
}

void CClientHandler::SendUpdateSegmentRequest( anf::SessionPtrType& aoSession )
{
    std::vector<CSegmentInfo> loSegmentInfos;
    this->mpoSegmentQueue->GetAllSegmentInfo(loSegmentInfos);

    if (this->moPrevSegmentInfos.size() == 0)
    {
        this->mpoSegmentQueue->GetAllSegmentInfo(this->moPrevSegmentInfos);
    }

    ACL_ASSERT(loSegmentInfos.size() == this->moPrevSegmentInfos.size() );

    for (apl_size_t luN = 0; luN < loSegmentInfos.size(); luN++)
    {
        CSegmentInfo& loCurr = loSegmentInfos[luN];
        CSegmentInfo& loPrev = this->moPrevSegmentInfos[luN];

        if (/*loCurr.GetTimestamp() <= 0 
            ||*/ loCurr.GetSize() == loPrev.GetSize() )
        {
            continue;
        }
    
        AAF_LOG_DEBUG("%s", "ClientHandler send update segment request to master");

        acl::CIndexDict loRequest;
        loRequest.Set(GFQ_TAG_NODE_NAME, this->moNodeName.c_str() );
        loRequest.Set(GFQ_TAG_SEGMENT_ID, loCurr.GetSegmentID() );
        loRequest.Set(GFQ_TAG_TIMESTAMP, loCurr.GetTimestamp() );
        loRequest.Set(GFQ_TAG_QUEUE_NAME, loCurr.GetQueueName() );
        loRequest.Set(GFQ_TAG_SEGMENT_SIZE, loCurr.GetSize() );

        this->WriteMessage(aoSession, 0, GFQ_REQUEST_UPDATE_SEGMENT, loRequest);
    }

    this->moPrevSegmentInfos.swap(loSegmentInfos);
}

void CClientHandler::SendUpdateStatInfoRequest( anf::SessionPtrType& aoSession, apl_int_t aiInterval )
{
    AAF_LOG_DEBUG("%s", "ClientHandler send update load info request to master");
    
    acl::CIndexDict loRequest;

    this->mpoSegmentQueue->Dump(loRequest);

    apl_size_t luConnections = acl::Instance<anf::CIoSockAcceptor>()->GetManagedSessionCount();

    loRequest.Set(GFQ_TAG_NODE_NAME, this->moNodeName.c_str() );
    loRequest.Set(GFQ_TAG_NODE_CONNECTION_COUNT, luConnections);
    loRequest.Set(GFQ_TAG_TIME_INTERVAL, aiInterval);

    this->WriteMessage(aoSession, 0, GFQ_REQUEST_UPDATE_STAT_INFO, loRequest);
}


//Handle method
void CClientHandler::HandleAllocateRequest(anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage )
{
    acl::CIndexDict loResponse;
    
    apl_size_t  luSegmentID = apoMessage->GetInt(GFQ_TAG_SEGMENT_ID, 0);
    apl_int64_t li64Timestamp = apoMessage->GetInt(GFQ_TAG_TIMESTAMP, 0);
    char const* lpcQueueName = apoMessage->Get(GFQ_TAG_QUEUE_NAME, "");

    apl_int_t liRetCode = this->mpoSegmentQueue->Allocate(luSegmentID, lpcQueueName, li64Timestamp);
    if (liRetCode != 0)
    {
        AAF_LOG_ERROR("ClientHandler handle allocate request fail, "
            "(SegmentID=%"APL_PRIuINT",QueueName=%s,Status=%"APL_PRIdINT")",
            luSegmentID,
            lpcQueueName,
            liRetCode );
    }
    else
    {
        AAF_LOG_DEBUG("ClientHandler handle allocate request success, "
            "(SegmentID=%"APL_PRIuINT",QueueName=%s)",
            luSegmentID,
            lpcQueueName );
    }
    
    loResponse.Set(anf::BENC_TAG_ERROR_CODE, liRetCode);
    loResponse.Set(GFQ_TAG_SEGMENT_ID, luSegmentID);
    loResponse.Set(GFQ_TAG_NODE_NAME, this->moNodeName.c_str() );
    loResponse.Set(GFQ_TAG_QUEUE_NAME, lpcQueueName);
    
    this->WriteMessage(aoSession, 0, GFQ_RESPONSE_ALLOCATE, loResponse);
}

void CClientHandler::HandleDeallocateRequest(anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage )
{
    acl::CIndexDict loResponse;
    
    apl_size_t  luSegmentID = apoMessage->GetInt(GFQ_TAG_SEGMENT_ID, 0);
    char const* lpcQueueName = apoMessage->Get(GFQ_TAG_QUEUE_NAME, 0);
    
    apl_int_t liRetCode = this->mpoSegmentQueue->Deallocate(luSegmentID);
    if (liRetCode != 0)
    {
        AAF_LOG_ERROR("ClientHandler handle deallocate request fail, "
            "(SegmentID=%"APL_PRIuINT",Status=%"APL_PRIdINT")",
            luSegmentID,
            liRetCode );
    }
    else
    {
        AAF_LOG_DEBUG("ClientHandler handle deallocate request success, "
            "(SegmentID=%"APL_PRIuINT")",
            luSegmentID );
    }
    
    loResponse.Set(anf::BENC_TAG_ERROR_CODE, liRetCode);
    loResponse.Set(GFQ_TAG_SEGMENT_ID, luSegmentID);
    loResponse.Set(GFQ_TAG_NODE_NAME, this->moNodeName.c_str() );
    loResponse.Set(GFQ_TAG_QUEUE_NAME, lpcQueueName);
    
    this->WriteMessage(aoSession, 0, GFQ_RESPONSE_DEALLOCATE, loResponse);
}

void CClientHandler::HandleRegisterResponse( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage )
{
    apl_int_t liStatus = apoMessage->GetInt(anf::BENC_TAG_ERROR_CODE, 0);
    
    if (liStatus != 0)
    {
        AAF_LOG_ERROR("ClientHandler register to master fail, (Status=%"APL_PRIdINT")", liStatus);

        aoSession->Close(aoSession);
    }
    else
    {
        AAF_LOG_INFO("%s", "ClientHandler register to master success");
        this->SendAddSegmentRequest(aoSession);
    }
}

void CClientHandler::HandleAddSegmentResponse( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage )
{
    apl_size_t luSegmentID = apoMessage->GetInt(GFQ_TAG_SEGMENT_ID, 0);
    apl_int_t liStatus = apoMessage->GetInt(anf::BENC_TAG_ERROR_CODE, 0);
    
    if (liStatus != 0)
    {
        AAF_LOG_ERROR("ClientHandler add segment to master fail, "
            "(SegmentID=%"APL_PRIuINT",Status=%"APL_PRIdINT")", luSegmentID, liStatus);

        aoSession->Close(aoSession);
    }
    else
    {
        AAF_LOG_DEBUG("ClientHandler add segment to master success, (SegmentID=%"APL_PRIuINT")", luSegmentID);
    }
}

void CClientHandler::HandleOnlineResponse( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage )
{
    apl_int_t liStatus = apoMessage->GetInt(anf::BENC_TAG_ERROR_CODE, 0);
    
    if (liStatus != 0)
    {
        AAF_LOG_ERROR("ClientHandler request server node online fail, (Status=%"APL_PRIdINT")", liStatus);

        aoSession->Close(aoSession);
    }
    else
    {
        this->mbIsOnlined = true;

        acl::TSmartLock<acl::CLock> loGuard(this->moLock);
        
        this->moSession = aoSession;

        AAF_LOG_DEBUG("ClientHandler request server node online success");
    }
}

void CClientHandler::HandleUnregisterResponse( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage )
{
    apl_int_t liStatus = apoMessage->GetInt(anf::BENC_TAG_ERROR_CODE, 0);
    
    if (liStatus != 0)
    {
        AAF_LOG_ERROR("ClientHandler unregister to master fail, (Status=%"APL_PRIdINT")", liStatus);
    }
    else
    {
        AAF_LOG_INFO("%s", "ClientHandler unregister to master success");
    }
    
    aoSession->Close(aoSession);
}

void CClientHandler::HandleUpdateSegmentResponse( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage )
{
    apl_int_t liStatus = apoMessage->GetInt(anf::BENC_TAG_ERROR_CODE, 0);
    
    if (liStatus != 0)
    {
        AAF_LOG_ERROR("ClientHandler update segment to master fail, (Status=%"APL_PRIdINT")", liStatus);
    }
    else
    {
        AAF_LOG_DEBUG("%s", "ClientHandler update segment to master success");
    }
}

void CClientHandler::HandleUpdateStatInfoResponse( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage )
{
    apl_int_t liStatus = apoMessage->GetInt(anf::BENC_TAG_ERROR_CODE, 0);
    
    if (liStatus != 0)
    {
        AAF_LOG_ERROR("ClientHandler update load info to master fail, (Status=%"APL_PRIdINT")", liStatus);
    }
    else
    {
        AAF_LOG_DEBUG("%s", "ClientHandler update load info to master success");
    }
}

void CClientHandler::HandleHeartbeatResponse( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage )
{
    apl_int_t liStatus = apoMessage->GetInt(anf::BENC_TAG_ERROR_CODE, 0);
    
    if (liStatus != 0)
    {
        AAF_LOG_ERROR("ClientHandler heartbeat to master fail, (Status=%"APL_PRIdINT")", liStatus);
    }
    else
    {
        AAF_LOG_DEBUG("%s", "ClientHandler heartbeat to master success");
    }
}

void CClientHandler::WriteMessage(
    anf::SessionPtrType& aoSession, 
    apl_size_t auMessageID, 
    apl_size_t auCommandID,
    acl::CIndexDict& aoMessage )
{
    aoMessage.Set(anf::BENC_TAG_SEQ_NO, auMessageID);
    aoMessage.Set(anf::BENC_TAG_CMD_ID, auCommandID);

    anf::CWriteFuture::PointerType loFuture = aoSession->Write(aoSession, &aoMessage);
    if (loFuture->IsException() )
    {
        AAF_LOG_ERROR("ClientHandler write message fail, (State=%"APL_PRIdINT",Errno=%"APL_PRIdINT")", 
            loFuture->GetState(), loFuture->GetErrno() );
    }
}

AIBC_GFQ_NAMESPACE_END

