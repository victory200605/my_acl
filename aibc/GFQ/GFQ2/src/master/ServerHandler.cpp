
#include "gfq2/GFQMessage.h"
#include "aaf/LogService.h"
#include "acl/IndexDict.h"
#include "anf/protocol/BencodingMessage.h"
#include "EventObserver.h"
#include "QueuePoll.h"
#include "ServerHandler.h"
#include "QueueManager.h"
#include "ServerNodeManager.h"

AIBC_GFQ_NAMESPACE_START

void NodeEventHandler( anf::SessionPtrType& aoSession, apl_int_t aiClass, char const* apcProvider, apl_int_t aiEvent )
{
}

void QueueEventHandler( anf::SessionPtrType& aoSession, apl_int_t aiClass, char const* apcProvider, apl_int_t aiEvent )
{
    AAF_LOG_DEBUG("Send event message to client, (Remote=%s:%"APL_PRIdINT")",
        aoSession->GetRemoteAddress().GetIpAddr(),
        (apl_int_t)aoSession->GetRemoteAddress().GetPort() );

    acl::CIndexDict loMessage;

    loMessage.Set(anf::BENC_TAG_SEQ_NO, (apl_intmax_t)0);
    loMessage.Set(anf::BENC_TAG_CMD_ID, GFQ_REQUEST_EVENT);
    loMessage.Set(GFQ_TAG_EVENT_CLASS, aiClass);
    loMessage.Set(GFQ_TAG_EVENT_PROVIDER, apcProvider);
    loMessage.Set(GFQ_TAG_EVENT, aiEvent);
    
    aoSession->Write(aoSession, &loMessage);
}

void PollQueueHandler( anf::SessionPtrType& aoSession, apl_size_t auSequenceID, char const* apcResult, apl_size_t auLen )
{
    AAF_LOG_DEBUG("Response poll result to client, (Remote=%s:%"APL_PRIdINT")",
        aoSession->GetRemoteAddress().GetIpAddr(),
        (apl_int_t)aoSession->GetRemoteAddress().GetPort() );

    acl::CIndexDict loMessage;

    loMessage.Set(anf::BENC_TAG_SEQ_NO, (apl_intmax_t)auSequenceID);
    loMessage.Set(anf::BENC_TAG_CMD_ID, GFQ_RESPONSE_POLL_QUEUE);
    loMessage.Set(GFQ_TAG_QUEUE_BITVEC, apcResult, auLen);
    
    aoSession->Write(aoSession, &loMessage);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CServerHandler::CServerHandler(void)
{
    this->moStartupTime.Update(acl::CTimestamp::PRC_NSEC);
}

CServerHandler::~CServerHandler(void)
{
}
    
void CServerHandler::Exception( anf::CThrowable const& aoCause )
{
    AAF_LOG_ERROR("ServerHandler catch a exception,%s (State=%"APL_PRIdINT",Errno=%"APL_PRIdINT")",
        aoCause.GetMessage(),
        (apl_int_t)aoCause.GetState(),
        (apl_int_t)aoCause.GetErrno() )
}
    
void CServerHandler::SessionException( anf::SessionPtrType& aoSession, anf::CThrowable const& aoCause )
{
    AAF_LOG_ERROR("ServerHandler catch session exception,%s "
        "(Remote=%s:%"APL_PRIdINT",SessionID=%"APL_PRIu64",State=%"APL_PRIdINT",Errno=%"APL_PRIdINT")",
        aoCause.GetMessage(),
        aoSession->GetRemoteAddress().GetIpAddr(),
        (apl_int_t)aoSession->GetRemoteAddress().GetPort(),
        (apl_uint64_t)aoSession->GetSessionID(),
        (apl_int_t)aoCause.GetState(),
        (apl_int_t)aoCause.GetErrno() );
}

void CServerHandler::SessionOpened( anf::SessionPtrType& aoSession )
{
    AAF_LOG_INFO("ServerHandler add connection to master, (Remote=%s:%"APL_PRIdINT",SessionID=%"APL_PRIu64")",
        aoSession->GetRemoteAddress().GetIpAddr(),
        (apl_int_t)aoSession->GetRemoteAddress().GetPort(),
        (apl_uint64_t)aoSession->GetSessionID() );
}

void CServerHandler::SessionClosed( anf::SessionPtrType& aoSession )
{
    std::string* lpoServerNodeName = anf::Find<std::string>(aoSession->GetAttribute(), (apl_size_t)0);
    if (lpoServerNodeName != APL_NULL)
    {
        acl::CIndexDict loMessage;
        loMessage.Set(GFQ_TAG_NODE_NAME, lpoServerNodeName->c_str() );

        this->HandleUnregisterRequest(aoSession, &loMessage);
    }

    acl::Instance<CEventObserver>()->Remove(aoSession);

    AAF_LOG_INFO("ServerHandler closed connection to master, (Remote=%s:%"APL_PRIdINT",SessionID=%"APL_PRIu64")",
        aoSession->GetRemoteAddress().GetIpAddr(),
        (apl_int_t)aoSession->GetRemoteAddress().GetPort(),
        (apl_uint64_t)aoSession->GetSessionID() );
}

void CServerHandler::SessionIdle( anf::SessionPtrType& aoSession, apl_int_t aiStatus )
{
    switch(aiStatus)
    {
        case anf::SESSION_READ_IDLE:
        {
            AAF_LOG_ERROR("Server handler close session because session read idle timeout, (Remote=%s:%"APL_PRIdINT")",
                aoSession->GetRemoteAddress().GetIpAddr(),
                (apl_int_t)aoSession->GetRemoteAddress().GetPort() );

            aoSession->Close(aoSession);

            break;
        }
    };
}

void CServerHandler::MessageReceived( anf::SessionPtrType& aoSession, acl::CAny const& aoMessage )
{
    acl::CIndexDict* lpoMessage = NULL;
    
    //Cast message to scp message type
    aoMessage.CastTo<acl::CIndexDict*>(lpoMessage);
    if (lpoMessage == NULL)//Exception
    {
        //Exception, invalid message
        AAF_LOG_ERROR("ServerHandler session received invalid message, (Remote=%s:%"APL_PRIdINT",SessionID=%"APL_PRIu64")",
            aoSession->GetRemoteAddress().GetIpAddr(),
            (apl_int_t)aoSession->GetRemoteAddress().GetPort(),
            (apl_uint64_t)aoSession->GetSessionID() );

        aoSession->Close(aoSession);
        return;
    }

    apl_int_t liCommandID = lpoMessage->GetInt(anf::BENC_TAG_CMD_ID, 0);
   
    switch(liCommandID)
    {
        case GFQ_REQUEST_READABLE_SEGMENT:
        {
            //TODO get readable segment
            this->HandleReadableSegmentRequest(aoSession, lpoMessage);
            break;
        }
        case GFQ_REQUEST_WRITEABLE_SEGMENT:
        {
            //TODO get writeable segment
            this->HandleWriteableSegmentRequest(aoSession, lpoMessage);
            break;
        }
        case GFQ_REQUEST_STARTUP_TIMESTAMP:
        {
            this->HandleStartupTimestampRequest(aoSession, lpoMessage);
            break;
        }
        case GFQ_REQUEST_POLL_QUEUE:
        {
            this->HandlePollQueueRequest(aoSession, lpoMessage);
            break;
        }
        case GFQ_REQUEST_EVENT_REGISTER:
        {
            this->HandleEventRegisterRequest(aoSession, lpoMessage);
            break;
        }
        case GFQ_REQUEST_REGISTER:
        {
            this->HandleRegisterRequest(aoSession, lpoMessage);
            break;
        }
        case GFQ_REQUEST_UNREGISTER:
        {
            this->HandleUnregisterRequest(aoSession, lpoMessage);
            break;
        }
        case GFQ_REQUEST_ADD_SEGMENT:
        {
            this->HandleAddSegmentRequest(aoSession, lpoMessage);
            break;
        }
        case GFQ_REQUEST_ONLINE:
        {
            this->HandleOnlineRequest(aoSession, lpoMessage);
            break;
        }
        case GFQ_REQUEST_UPDATE_SEGMENT:
        {
            this->HandleUpdateSegmentRequest(aoSession, lpoMessage);
            break;
        }
        case GFQ_REQUEST_UPDATE_STAT_INFO:
        {
            this->HandleUpdateStatInfoRequest(aoSession, lpoMessage);
            break;
        }
        case GFQ_REQUEST_QUEUE_LIST:
        {
            this->HandleQueueListRequest(aoSession, lpoMessage);
            break;
        }
        case GFQ_REQUEST_QUEUE_INFO:
        {
            this->HandleQueueInfoRequest(aoSession, lpoMessage);
            break;
        }
        case GFQ_REQUEST_NODE_LIST:
        {
            this->HandleNodeListRequest(aoSession, lpoMessage);
            break;
        }
        case GFQ_REQUEST_NODE_INFO:
        {
            this->HandleNodeInfoRequest(aoSession, lpoMessage);
            break;
        }
        case GFQ_REQUEST_BIND_NODE_GROUP:
        {
            this->HandleBindNodeGroupRequest(aoSession, lpoMessage);
            break;
        }
        case GFQ_REQUEST_CREATE_QUEUE:
        {
            this->HandleCreateQueueRequest(aoSession, lpoMessage);
            break;
        }
        case GFQ_REQUEST_HEARTBEAT:
        {
            this->HandleHeartbeatRequest(aoSession, lpoMessage);
            break;
        }
        case GFQ_RESPONSE_ALLOCATE:
        {
            this->HandleAllocateResponse(aoSession, lpoMessage);
            break;
        }
        case GFQ_RESPONSE_DEALLOCATE:
        {
            this->HandleDeallocateResponse(aoSession, lpoMessage);
            break;
        }
        default:
        {
            //Exception, invalid message
            AAF_LOG_ERROR("ServerHandler session received invalid message, "
                "(CmdID=0x%"APL_PRIxINT",Remote=%s:%"APL_PRIdINT",SessionID=%"APL_PRIu64")",
                liCommandID,
                aoSession->GetRemoteAddress().GetIpAddr(),
                (apl_int_t)aoSession->GetRemoteAddress().GetPort(),
                (apl_uint64_t)aoSession->GetSessionID() );

            aoSession->Close(aoSession);
        }
    };

    ACL_DELETE(lpoMessage);
}

void CServerHandler::MessageFinished( anf::SessionPtrType& aoSession, anf::CWriteFuture::PointerType& aoWriteFuture )
{
    if (aoWriteFuture->IsException() )
    {
        AAF_LOG_ERROR("ServerHandler message flush exception, "
            "(State=%"APL_PRIdINT",Errno=%"APL_PRIdINT",SessionID=%"APL_PRIu64")",
            (apl_int_t)aoWriteFuture->GetState(),
            (apl_int_t)aoWriteFuture->GetErrno(),
            (apl_uint64_t)aoSession->GetSessionID() );
    }
    else
    {
        AAF_LOG_TRACE("ServerHandler message flush finished, (MsgID=%"APL_PRIuINT",SessionID=%"APL_PRIu64")",
            (apl_size_t)aoWriteFuture->GetAct(),
            (apl_uint64_t)aoSession->GetSessionID() );
    }
}

//handle method
void CServerHandler::HandleReadableSegmentRequest( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage )
{
    acl::CIndexDict loResponse;
    CSegmentInfo loSegmentInfo;

    char const* lpcUserName = apoMessage->Get(GFQ_TAG_USER_NAME, "");
    char const* lpcQueueName = apoMessage->Get(GFQ_TAG_QUEUE_NAME, "");
    apl_size_t  luSegmentID = apoMessage->GetInt(GFQ_TAG_OLD_SEGMENT_ID, INVALID_ID);
    char const* lpcNodeName = apoMessage->Get(GFQ_TAG_OLD_NODE_NAME, "");
    apl_int_t   liStatus = apoMessage->GetInt(GFQ_TAG_OLD_SEGMENT_STATUS, 0);

    //Find segment handle from server node manager
    CSegmentHandle loOldHandle = GetServerNodeManager()->Find(lpcNodeName, luSegmentID);

    //Get readable segment by old segment from queue manager
    apl_int_t liRetCode = GetQueueManager()->GetReadableSegment(lpcQueueName, loOldHandle, liStatus, loSegmentInfo);
    if (liRetCode != 0)
    {
        switch(liRetCode)
        {
            case ERROR_END_OF_QUEUE:
            case ERROR_QUEUE_UNEXISTED:
                AAF_LOG_DEBUG("Request readable segment fail, (UserName=%s,QueueName=%s,Status=%"APL_PRIdINT")",
                    lpcUserName,
                    lpcQueueName,
                    liRetCode );

                liRetCode = ERROR_END_OF_QUEUE;
                break;
            default:
                AAF_LOG_ERROR("Request readable segment fail, (UserName=%s,QueueName=%s,Status=%"APL_PRIdINT")",
                    lpcUserName,
                    lpcQueueName,
                    liRetCode );
                break;
        };
    }
    else
    {
        AAF_LOG_DEBUG("Request readable segment success, "
            "(UserName=%s,QueueName=%s,SegmentID=%"APL_PRIuINT",ServerNodeName=%s)",
            lpcUserName,
            lpcQueueName,
            loSegmentInfo.GetSegmentID(),
            loSegmentInfo.GetServerNodeName() );

        loResponse.Set(GFQ_TAG_SEGMENT_ID, loSegmentInfo.GetSegmentID() );
        loResponse.Set(GFQ_TAG_NODE_NAME, loSegmentInfo.GetServerNodeName() );
        loResponse.Set(GFQ_TAG_NODE_SERVER_IP, loSegmentInfo.GetServerIp() );
        loResponse.Set(GFQ_TAG_NODE_SERVER_PORT, loSegmentInfo.GetServerPort() );
        loResponse.Set(GFQ_TAG_TIMESTAMP, loSegmentInfo.GetTimestamp() );
    }

    this->WriteMessage(
        aoSession, 
        apoMessage->GetInt(anf::BENC_TAG_SEQ_NO, 0),
        GFQ_RESPONSE_READABLE_SEGMENT, 
        liRetCode, 
        loResponse);
}

void CServerHandler::HandleWriteableSegmentRequest( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage )
{
    acl::CIndexDict loResponse;
    CSegmentInfo loSegmentInfo;
    
    char const* lpcUserName = apoMessage->Get(GFQ_TAG_USER_NAME, "");
    char const* lpcQueueName = apoMessage->Get(GFQ_TAG_QUEUE_NAME, "");
    apl_size_t  luSegmentID = apoMessage->GetInt(GFQ_TAG_OLD_SEGMENT_ID, INVALID_ID);
    char const* lpcNodeName = apoMessage->Get(GFQ_TAG_OLD_NODE_NAME, "");
    apl_int_t   liStatus = apoMessage->GetInt(GFQ_TAG_OLD_SEGMENT_STATUS, 0);

    //Find segment handle from server node manager
    CSegmentHandle loOldHandle = GetServerNodeManager()->Find(lpcNodeName, luSegmentID);

    //Get readable segment by old segment from queue manager
    apl_int_t liRetCode = GetQueueManager()->GetWriteableSegment(lpcQueueName, loOldHandle, liStatus, loSegmentInfo);
    if (liRetCode != 0)
    {
        AAF_LOG_ERROR("Request writeable segment fail, (UserName=%s,QueueName=%s,Status=%"APL_PRIdINT")",
            lpcUserName,
            lpcQueueName,
            liRetCode );
    }
    else
    {
        AAF_LOG_DEBUG("Request writeable segment success, "
            "(UserName=%s,QueueName=%s,SegmentID=%"APL_PRIuINT",ServerNodeName=%s)",
            lpcUserName,
            lpcQueueName,
            loSegmentInfo.GetSegmentID(),
            loSegmentInfo.GetServerNodeName() );

        loResponse.Set(GFQ_TAG_SEGMENT_ID, loSegmentInfo.GetSegmentID() );
        loResponse.Set(GFQ_TAG_NODE_NAME, loSegmentInfo.GetServerNodeName() );
        loResponse.Set(GFQ_TAG_NODE_SERVER_IP, loSegmentInfo.GetServerIp() );
        loResponse.Set(GFQ_TAG_NODE_SERVER_PORT, loSegmentInfo.GetServerPort() );
        loResponse.Set(GFQ_TAG_TIMESTAMP, loSegmentInfo.GetTimestamp() );
    }

    this->WriteMessage(
        aoSession, 
        apoMessage->GetInt(anf::BENC_TAG_SEQ_NO, 0), 
        GFQ_RESPONSE_WRITEABLE_SEGMENT, 
        liRetCode, 
        loResponse);
}

void CServerHandler::HandleStartupTimestampRequest( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage )
{
    acl::CIndexDict loResponse;

    loResponse.Set(GFQ_TAG_TIMESTAMP, this->moStartupTime.Nsec() );
    
    AAF_LOG_DEBUG("Request poll queues schedule success, (UserName=%s)",
        apoMessage->Get(GFQ_TAG_USER_NAME, "") )
    
    this->WriteMessage(
        aoSession, 
        apoMessage->GetInt(anf::BENC_TAG_SEQ_NO, 0), 
        GFQ_RESPONSE_STARTUP_TIMESTAMP, 
        0, 
        loResponse);
}

void CServerHandler::HandlePollQueueRequest( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage )
{
    acl::CIndexDict::ValueType loBitSet = apoMessage->GetStr(GFQ_TAG_QUEUE_BITVEC, "");
    apl_int_t liTimedout = apoMessage->GetInt(GFQ_TAG_TIMEDOUT, 0);
    apl_int64_t li64Timestamp = apoMessage->GetInt(GFQ_TAG_TIMESTAMP, 0);

    if (li64Timestamp != this->moStartupTime.Nsec() )
    {
        acl::CIndexDict loResponse;

        this->WriteMessage(
            aoSession, 
            apoMessage->GetInt(anf::BENC_TAG_SEQ_NO, 0), 
            GFQ_RESPONSE_POLL_QUEUE, 
            ERROR_INVALID_TIMESTAMP, 
            loResponse);

        return;
    }

    apl_int_t liRetCode = acl::Instance<CQueuePoll>()->SchedulePoll(
        aoSession,
        apoMessage->GetInt(anf::BENC_TAG_SEQ_NO, 0),
        loBitSet.GetStr(),
        loBitSet.GetLength(),
        liTimedout,
        PollQueueHandler);
    if (liRetCode != 0)
    {
        AAF_LOG_ERROR("Request poll queues schedule fail, (UserName=%s,Status=%"APL_PRIdINT")",
            apoMessage->Get(GFQ_TAG_USER_NAME, ""),
            liRetCode );
    }
    else
    {
        AAF_LOG_DEBUG("Request poll queues schedule success, (UserName=%s)",
            apoMessage->Get(GFQ_TAG_USER_NAME, "") )
    }
}

void CServerHandler::HandleCreateQueueRequest( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage )
{
    apl_int_t       liRetCode = 0;
    acl::CIndexDict loResponse;
    CQueueInfo      loQueueInfo;
    char const*     lpcQueueName = apoMessage->Get(GFQ_TAG_QUEUE_NAME, "");
    apl_int_t       liOpt = apoMessage->GetInt(GFQ_TAG_OPTION, 0);

    liRetCode = GetQueueManager()->CreateQueue(lpcQueueName, liOpt, &loQueueInfo);
    if (liRetCode != 0)
    {
        AAF_LOG_ERROR("Request create queue fail, (QueueName=%s,opt=%"APL_PRIdINT",Status=%"APL_PRIdINT")", 
            lpcQueueName,
            liOpt,
            liRetCode );
    }
    else
    {
        AAF_LOG_DEBUG("Request create queue success, (QueueName=%s,opt=%"APL_PRIdINT")",
            lpcQueueName,
            liOpt );

        loResponse.Set(GFQ_TAG_QUEUE_ID, loQueueInfo.GetQueueID() );
        loResponse.Set(GFQ_TAG_QUEUE_NAME, loQueueInfo.GetQueueName() );
        loResponse.Set(GFQ_TAG_QUEUE_CAPACITY, loQueueInfo.GetCapacity() );
        loResponse.Set(GFQ_TAG_QUEUE_SIZE, loQueueInfo.GetSize() );
        loResponse.Set(GFQ_TAG_SEGMENT_COUNT, loQueueInfo.GetSegmentCount() );
    }

    this->WriteMessage(
         aoSession, 
         apoMessage->GetInt(anf::BENC_TAG_SEQ_NO, 0), 
         GFQ_RESPONSE_QUEUE_INFO, 
         liRetCode, 
         loResponse);
}

void CServerHandler::HandleEventRegisterRequest( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage )
{
    acl::CIndexDict loResponse;

    apl_int_t   liRetCode = 0;
    apl_int_t   liClass = apoMessage->GetInt(GFQ_TAG_EVENT_CLASS, 0);
    apl_int_t   liEvent = apoMessage->GetInt(GFQ_TAG_EVENT, 0);

    switch(liClass)
    {
        case CLASS_NODE_EVENT:
            liRetCode = acl::Instance<CEventObserver>()->Register(aoSession, liClass, liEvent, NodeEventHandler);
            break;
        case CLASS_QUEUE_EVENT:
            liRetCode = acl::Instance<CEventObserver>()->Register(aoSession, liClass, liEvent, QueueEventHandler);
            break;
        default:
            liRetCode = ERROR_UNIMPLEMENTED;
            break;
    };

    if (liRetCode == 0)
    {
        AAF_LOG_DEBUG("Request register event success, "
            "(SessionID=%"APL_PRIu64",Class=%"APL_PRIx32",Event=%"APL_PRIx32")",
            (apl_uint64_t)aoSession->GetSessionID(),
            (apl_int32_t)liClass,
            (apl_int32_t)liEvent );
    }
    else
    {
        AAF_LOG_DEBUG("Request register event fail, "
            "(SessionID=%"APL_PRIu64",Class=%"APL_PRIx32",Event=%"APL_PRIx32",Status=%"APL_PRIdINT")",
            (apl_uint64_t)aoSession->GetSessionID(),
            (apl_int32_t)liClass,
            (apl_int32_t)liEvent,
            liRetCode);
    }

    this->WriteMessage(
        aoSession, 
        apoMessage->GetInt(anf::BENC_TAG_SEQ_NO, 0), 
        GFQ_RESPONSE_EVENT_REGISTER, 
        liRetCode, 
        loResponse);
}

void CServerHandler::HandleRegisterRequest( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage )
{
    acl::CIndexDict loResponse;

    char const* lpcNodeName = apoMessage->Get(GFQ_TAG_NODE_NAME, "");
    char const* lpcGroupName = apoMessage->Get(GFQ_TAG_GROUP_NAME, "");
    char const* lpcStorageType = apoMessage->Get(GFQ_TAG_NODE_STORAGE_TYPE, "");
    char const* lpcServerIp = apoMessage->Get(GFQ_TAG_NODE_SERVER_IP, "");
    apl_int_t   liServerPort = apoMessage->GetInt(GFQ_TAG_NODE_SERVER_PORT, 0);
    apl_int_t   liPriority = apoMessage->GetInt(GFQ_TAG_NODE_PRIORITY, 0);

    apl_int_t liRetCode = GetServerNodeManager()->AddServerNode(
        lpcNodeName,
        lpcGroupName,
        lpcStorageType,
        lpcServerIp,
        liServerPort,
        liPriority,
        aoSession);
    if (liRetCode != 0)
    {
        AAF_LOG_ERROR("Request register server node fail, "
            "(NodeName=%s, Server=%s:%"APL_PRIdINT",Status=%"APL_PRIdINT")",
            lpcNodeName,
            lpcServerIp,
            liServerPort,
            liRetCode );
    }
    else
    {
        apl_size_t luServerNodeID = GetServerNodeManager()->GetServerNodeID(lpcNodeName);
        ACL_ASSERT(luServerNodeID != INVALID_ID);

        aoSession->GetAttribute().PushBack(std::string(lpcNodeName) );
        aoSession->GetAttribute().PushBack(luServerNodeID);

        AAF_LOG_DEBUG("Request register server node success, (NodeName=%s,Server=%s:%"APL_PRIdINT")",
            lpcNodeName,
            lpcServerIp,
            liServerPort );
    }

    this->WriteMessage(
        aoSession, 
        apoMessage->GetInt(anf::BENC_TAG_SEQ_NO, 0), 
        GFQ_RESPONSE_REGISTER, 
        liRetCode, 
        loResponse);
}

void CServerHandler::HandleUnregisterRequest( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage )
{
    apl_int_t liRetCode = GetServerNodeManager()->DelServerNode(apoMessage->Get(GFQ_TAG_NODE_NAME, "") );
    if (liRetCode != 0)
    {
        AAF_LOG_ERROR("Request unregister server node fail, (NodeName=%s,Status=%"APL_PRIdINT")",
            apoMessage->Get(GFQ_TAG_NODE_NAME, ""),
            liRetCode );
    }
    else
    {
        apl_size_t* lpoServerNodeID = anf::Find<apl_size_t>(aoSession->GetAttribute(), (apl_size_t)1);
        ACL_ASSERT(lpoServerNodeID != APL_NULL);

        GetQueueManager()->ClearQueueInvalidSegment(*lpoServerNodeID);

        AAF_LOG_DEBUG("Request unregister server node success, (NodeName=%s)",
            apoMessage->Get(GFQ_TAG_NODE_NAME, "") );
    }
}

void CServerHandler::HandleAddSegmentRequest( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage )
{
    apl_int_t liRetCode = 0;
    acl::CIndexDict loResponse;

    char const* lpcNodeName = apoMessage->Get(GFQ_TAG_NODE_NAME, "");
    apl_size_t  luSegmentID = apoMessage->GetInt(GFQ_TAG_SEGMENT_ID, INVALID_ID);
    apl_int64_t li64Timestamp = apoMessage->GetInt(GFQ_TAG_TIMESTAMP, -1);
    char const* lpcQueueName = apoMessage->Get(GFQ_TAG_QUEUE_NAME, "");
    apl_size_t  luCapacity = apoMessage->GetInt(GFQ_TAG_SEGMENT_CAPACITY, 0);
    apl_size_t  luSize = apoMessage->GetInt(GFQ_TAG_SEGMENT_SIZE, 0);

    if (li64Timestamp > 0 && apl_strlen(lpcQueueName) > 0)
    {
        apl_int_t liRetCode = 0;
        CSegmentHandle loHandle;

        if ((liRetCode = GetServerNodeManager()->AddUsedSegment(
            lpcNodeName,
            luSegmentID, 
            luCapacity, 
            &loHandle) ) == 0)
        {
            liRetCode = GetQueueManager()->InsertSegment(lpcQueueName, loHandle, luSize, li64Timestamp); 
        }
    }
    else
    {
        liRetCode = GetServerNodeManager()->AddFreeSegment(lpcNodeName, luSegmentID, luCapacity);
    }

    if (liRetCode != 0)
    {
        AAF_LOG_ERROR("Request add segment fail, "
            "(NodeName=%s, SegmentID=%"APL_PRIuINT",QueueName=%s,Status=%"APL_PRIdINT")",
            lpcNodeName,
            luSegmentID,
            lpcQueueName,
            liRetCode );
    }
    else
    {
        AAF_LOG_TRACE("Request add segment success, (NodeName=%s,SegmentID=%"APL_PRIuINT",QueueName=%s)",
            lpcNodeName,
            luSegmentID,
            lpcQueueName )
    }

    loResponse.Set(GFQ_TAG_SEGMENT_ID, luSegmentID);

    this->WriteMessage(
        aoSession, 
        apoMessage->GetInt(anf::BENC_TAG_SEQ_NO, 0),
        GFQ_RESPONSE_ADD_SEGMENT, 
        liRetCode, 
        loResponse);
}

void CServerHandler::HandleOnlineRequest( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage )
{
    apl_int_t liRetCode = 0;
    acl::CIndexDict loResponse;

    char const* lpcNodeName = apoMessage->Get(GFQ_TAG_NODE_NAME, "");

    if ((liRetCode = GetServerNodeManager()->ActivateServerNode(lpcNodeName) ) != 0)
    {
        AAF_LOG_ERROR("Request server node online fail, (NodeName=%s,Status=%"APL_PRIdINT")",
            lpcNodeName,
            liRetCode );
    }
    else
    {
        AAF_LOG_TRACE("Request server node online success, (NodeName=%s)", lpcNodeName);
    }

    this->WriteMessage(
        aoSession, 
        apoMessage->GetInt(anf::BENC_TAG_SEQ_NO, 0),
        GFQ_RESPONSE_ONLINE, 
        liRetCode, 
        loResponse);
}

void CServerHandler::HandleUpdateSegmentRequest( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage )
{
    apl_int_t liRetCode = 0;
    acl::CIndexDict loResponse;

    char const* lpcNodeName = apoMessage->Get(GFQ_TAG_NODE_NAME, "");
    apl_size_t  luCount = apoMessage->Count(GFQ_TAG_SEGMENT_ID);

    for (apl_size_t luN = 0; luN < luCount; luN++)
    {
        char const* lpcQueueName = apoMessage->Get2(GFQ_TAG_QUEUE_NAME, luN, "");
        apl_size_t  luSegmentID = apoMessage->GetInt2(GFQ_TAG_SEGMENT_ID, luN, INVALID_ID);
        apl_int64_t li64Timestamp = apoMessage->GetInt2(GFQ_TAG_TIMESTAMP, luN, -1);
        apl_size_t  luSize = apoMessage->GetInt2(GFQ_TAG_SEGMENT_SIZE, luN, 0);

        //Find segment handle from server node manager
        CSegmentHandle loHandle = GetServerNodeManager()->Find(lpcNodeName, luSegmentID);
        
        liRetCode = GetQueueManager()->UpdateSegment(
            lpcQueueName,
            loHandle,
            luSize,
            li64Timestamp );
        if (liRetCode != 0)
        {
            AAF_LOG_DEBUG("Request update segment fail, "
                "(NodeName=%s, SegmentID=%"APL_PRIuINT",QueueName=%s,Status=%"APL_PRIdINT")",
                lpcNodeName,
                luSegmentID,
                lpcQueueName,
                liRetCode );

            break;
        }
    }
    
    AAF_LOG_TRACE("Request update segment success, (NodeName=%s)", lpcNodeName);
    
    this->WriteMessage(
        aoSession, 
        apoMessage->GetInt(anf::BENC_TAG_SEQ_NO, 0), 
        GFQ_RESPONSE_UPDATE_SEGMENT, 
        liRetCode, 
        loResponse );
}

void CServerHandler::HandleUpdateStatInfoRequest( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage )
{
    acl::CIndexDict loResponse;
    CServerNodeStatInfo loNodeStatInfo;

    char const* lpcNodeName = apoMessage->Get(GFQ_TAG_NODE_NAME, "");
    apl_int_t liInterval = apoMessage->GetInt(GFQ_TAG_TIME_INTERVAL, 1);

    loNodeStatInfo.SetConnectionCount(apoMessage->GetInt(GFQ_TAG_NODE_CONNECTION_COUNT, 0) );
    loNodeStatInfo.SetFreePageCount(apoMessage->GetInt(GFQ_TAG_NODE_FREE_PAGE_COUNT, 0) );
    loNodeStatInfo.SetUsedPageCount(apoMessage->GetInt(GFQ_TAG_NODE_USED_PAGE_COUNT, 0) );
    loNodeStatInfo.SetCacheFreePageCount(apoMessage->GetInt(GFQ_TAG_NODE_CACHE_FREE_PAGE_COUNT, 0) );
    loNodeStatInfo.SetCacheUsedPageCount(apoMessage->GetInt(GFQ_TAG_NODE_CACHE_USED_PAGE_COUNT, 0) );
    loNodeStatInfo.SetReadTimes(apoMessage->GetInt(GFQ_TAG_NODE_READ_TIMES, 0) / liInterval );
    loNodeStatInfo.SetWriteTimes(apoMessage->GetInt(GFQ_TAG_NODE_WRITE_TIMES, 0) / liInterval );
    loNodeStatInfo.SetCacheReadTimes(apoMessage->GetInt(GFQ_TAG_NODE_CACHE_READ_TIMES, 0) / liInterval );
    loNodeStatInfo.SetCacheReadMissedTimes(apoMessage->GetInt(GFQ_TAG_NODE_CACHE_READ_MISSED_TIMES, 0) / liInterval );
    loNodeStatInfo.SetCacheWriteTimes(apoMessage->GetInt(GFQ_TAG_NODE_CACHE_WRITE_TIMES, 0) / liInterval );
    loNodeStatInfo.SetCacheWriteMissedTimes(apoMessage->GetInt(GFQ_TAG_NODE_CACHE_WRITE_MISSED_TIMES, 0) / liInterval );
    loNodeStatInfo.SetCacheSyncTimes(apoMessage->GetInt(GFQ_TAG_NODE_CACHE_SYNC_TIMES, 0) / liInterval );
    
    apl_int_t liRetCode = GetServerNodeManager()->UpdateServerNodeStatInfo(lpcNodeName, loNodeStatInfo);
    if (liRetCode != 0)
    {
        AAF_LOG_DEBUG("Request update load info fail, (NodeName=%s,Status=%"APL_PRIdINT")",
            lpcNodeName,
            liRetCode );
    }
    else
    {
        AAF_LOG_TRACE("Request update load info success, (NodeName=%s)", lpcNodeName);
    }

    this->WriteMessage(
        aoSession, 
        apoMessage->GetInt(anf::BENC_TAG_SEQ_NO, 0), 
        GFQ_RESPONSE_UPDATE_STAT_INFO, 
        liRetCode, 
        loResponse );
}

void CServerHandler::HandleQueueListRequest( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage )
{
    acl::CIndexDict loResponse;
    std::vector<std::string> loResult;
    //acl::CBitArray loResult;
    apl_size_t luOffset = apoMessage->GetInt(GFQ_TAG_RECORD_OFFSET, (apl_intmax_t)0);
    apl_size_t luLimit = apoMessage->GetInt(GFQ_TAG_RECORD_LIMIT, (apl_intmax_t)0);

    apl_size_t luListSize = GetQueueManager()->GetQueueList(loResult, luOffset, luLimit);
    
    //loResponse.Set(GFQ_TAG_QUEUE_BITVEC, loResult.GetCStr(), loResult.GetCLength() );
    
    for (std::vector<std::string>::iterator loIter = loResult.begin();
         loIter != loResult.end(); ++loIter)
    {
        loResponse.Add(GFQ_TAG_QUEUE_NAME, *loIter);
    }

    AAF_LOG_DEBUG(
        "Request get queue list success, "
        "(ListSize=%"APL_PRIuINT",Offset=%"APL_PRIuINT",Limit=%"APL_PRIuINT")",
        luListSize,
        luOffset,
        luLimit );

    this->WriteMessage(
        aoSession, 
        apoMessage->GetInt(anf::BENC_TAG_SEQ_NO, 0), 
        GFQ_RESPONSE_QUEUE_LIST, 
        0, 
        loResponse);
}

void CServerHandler::HandleQueueInfoRequest( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage )
{
    apl_int_t       liRetCode = 0;
    acl::CIndexDict loResponse;
    CQueueInfo      loQueueInfo;
    char const*     lpcQueueName = apoMessage->Get(GFQ_TAG_QUEUE_NAME, "");
    apl_size_t      luQueueID = apoMessage->GetInt(GFQ_TAG_QUEUE_ID, INVALID_ID);

    if (luQueueID == INVALID_ID)
    {
        liRetCode = GetQueueManager()->GetQueueInfo(lpcQueueName, loQueueInfo);
    }
    else
    {
        liRetCode = GetQueueManager()->GetQueueInfo(luQueueID, loQueueInfo);
    }
    
    if (liRetCode != 0)
    {
        AAF_LOG_ERROR("Request get queue info fail, (QueueName=%s,QueueID=%"APL_PRIuINT",Status=%"APL_PRIdINT")", 
            lpcQueueName,
            luQueueID,
            liRetCode );
    }
    else
    {
        AAF_LOG_DEBUG("Request get queue info success, (QueueName=%s,QueueID=%"APL_PRIuINT")",
            lpcQueueName,
            luQueueID );

        loResponse.Set(GFQ_TAG_QUEUE_ID, loQueueInfo.GetQueueID() );
        loResponse.Set(GFQ_TAG_QUEUE_NAME, loQueueInfo.GetQueueName() );
        loResponse.Set(GFQ_TAG_QUEUE_CAPACITY, loQueueInfo.GetCapacity() );
        loResponse.Set(GFQ_TAG_QUEUE_SIZE, loQueueInfo.GetSize() );
        loResponse.Set(GFQ_TAG_SEGMENT_COUNT, loQueueInfo.GetSegmentCount() );
    }

    this->WriteMessage(
         aoSession, 
         apoMessage->GetInt(anf::BENC_TAG_SEQ_NO, 0), 
         GFQ_RESPONSE_QUEUE_INFO, 
         liRetCode, 
         loResponse);
}

void CServerHandler::HandleNodeListRequest( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage )
{
    acl::CIndexDict loResponse;
    std::vector<std::string> loResult;

    GetServerNodeManager()->GetServerNodeList(loResult);
    
    for (std::vector<std::string>::iterator loIter = loResult.begin();
         loIter != loResult.end(); ++loIter)
    {
        loResponse.Add(GFQ_TAG_NODE_NAME, *loIter);
    }

    AAF_LOG_DEBUG("Request get node list success, (ListSize=%"APL_PRIuINT")", (apl_size_t)loResult.size() );

    this->WriteMessage(
        aoSession, 
        apoMessage->GetInt(anf::BENC_TAG_SEQ_NO, 0), 
        GFQ_RESPONSE_NODE_LIST, 
        0, 
        loResponse);
}

void CServerHandler::HandleNodeInfoRequest( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage )
{
    acl::CIndexDict loResponse;
    CServerNodeInfo loServerNodeInfo;

    apl_int_t liRetCode = GetServerNodeManager()->GetServerNodeInfo(apoMessage->Get(GFQ_TAG_NODE_NAME, ""), loServerNodeInfo);
    if (liRetCode != 0)
    {
        AAF_LOG_ERROR("Request get server node info fail, (NodeName=%s,Status=%"APL_PRIdINT")",
            apoMessage->Get(GFQ_TAG_NODE_NAME, ""),
            liRetCode );
    }
    else
    {
        AAF_LOG_DEBUG("Request get server node list success, (NodeName=%s)", apoMessage->Get(GFQ_TAG_NODE_NAME, "") );
        loResponse.Set(GFQ_TAG_GROUP_NAME, loServerNodeInfo.GetGroupName() );
        loResponse.Set(GFQ_TAG_NODE_STORAGE_TYPE, loServerNodeInfo.GetStorageType() );
        loResponse.Set(GFQ_TAG_SEGMENT_COUNT, loServerNodeInfo.GetSegmentCount() );
        loResponse.Set(GFQ_TAG_NODE_FREE_SEGMENT_COUNT, loServerNodeInfo.GetFreeSegmentCount() );
        loResponse.Set(GFQ_TAG_NODE_USED_SEGMENT_COUNT, loServerNodeInfo.GetUsedSegmentCount() );
        loResponse.Set(GFQ_TAG_NODE_SERVER_IP, loServerNodeInfo.GetServerIp() );
        loResponse.Set(GFQ_TAG_NODE_SERVER_PORT, loServerNodeInfo.GetServerPort() );
        loResponse.Set(GFQ_TAG_NODE_CONNECTION_COUNT, loServerNodeInfo.GetStatInfo().GetConnectionCount() );
        loResponse.Set(GFQ_TAG_NODE_READ_TIMES, loServerNodeInfo.GetStatInfo().GetReadTimes() );
        loResponse.Set(GFQ_TAG_NODE_WRITE_TIMES, loServerNodeInfo.GetStatInfo().GetWriteTimes() );
        loResponse.Set(GFQ_TAG_NODE_FREE_PAGE_COUNT, loServerNodeInfo.GetStatInfo().GetFreePageCount() );
        loResponse.Set(GFQ_TAG_NODE_USED_PAGE_COUNT, loServerNodeInfo.GetStatInfo().GetUsedPageCount() );
        loResponse.Set(GFQ_TAG_NODE_CACHE_FREE_PAGE_COUNT, loServerNodeInfo.GetStatInfo().GetCacheFreePageCount() );
        loResponse.Set(GFQ_TAG_NODE_CACHE_USED_PAGE_COUNT, loServerNodeInfo.GetStatInfo().GetCacheUsedPageCount() );
        loResponse.Set(GFQ_TAG_NODE_CACHE_READ_TIMES, loServerNodeInfo.GetStatInfo().GetCacheReadTimes() );
        loResponse.Set(GFQ_TAG_NODE_CACHE_READ_MISSED_TIMES, loServerNodeInfo.GetStatInfo().GetCacheReadMissedTimes() );
        loResponse.Set(GFQ_TAG_NODE_CACHE_WRITE_TIMES, loServerNodeInfo.GetStatInfo().GetCacheWriteTimes() );
        loResponse.Set(GFQ_TAG_NODE_CACHE_WRITE_MISSED_TIMES, loServerNodeInfo.GetStatInfo().GetCacheWriteMissedTimes() );
        loResponse.Set(GFQ_TAG_NODE_CACHE_SYNC_TIMES, loServerNodeInfo.GetStatInfo().GetCacheSyncTimes() );
    }

    this->WriteMessage(
        aoSession, 
        apoMessage->GetInt(anf::BENC_TAG_SEQ_NO, 0),
        GFQ_RESPONSE_NODE_INFO, 
        liRetCode, 
        loResponse);
}

void CServerHandler::HandleBindNodeGroupRequest( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage )
{
    acl::CIndexDict loResponse;

    char const* lpcQueueName = apoMessage->Get(GFQ_TAG_QUEUE_NAME, "");
    char const* lpcGroupName = apoMessage->Get(GFQ_TAG_GROUP_NAME, "");

    apl_int_t liRetCode = GetQueueManager()->BindNodeGroup(lpcQueueName, lpcGroupName); 
    if (liRetCode != 0)
    {
        AAF_LOG_ERROR("Request bind node group fail, (QueueName=%s,GroupName=%s,Status=%"APL_PRIdINT")", 
            lpcQueueName,
            lpcGroupName,
            liRetCode );
    }
    else
    {
        AAF_LOG_ERROR("Request bind node group success, (QueueName=%s,GroupName=%s)", 
            lpcQueueName,
            lpcGroupName );
    }
    
    this->WriteMessage(
        aoSession, 
        apoMessage->GetInt(anf::BENC_TAG_SEQ_NO, 0), 
        GFQ_RESPONSE_BIND_NODE_GROUP, 
        0, 
        loResponse);
}

void CServerHandler::HandleHeartbeatRequest( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage )
{
    acl::CIndexDict loResponse;
    char const*     lpcNodeName = apoMessage->Get(GFQ_TAG_NODE_NAME, "");

    AAF_LOG_DEBUG("Request heartbeat success, (NodeName=%s)", lpcNodeName);
    
    this->WriteMessage(
        aoSession, 
        apoMessage->GetInt(anf::BENC_TAG_SEQ_NO, 0), 
        GFQ_RESPONSE_HEARTBEAT, 
        0, 
        loResponse);
}

void CServerHandler::HandleAllocateResponse( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage )
{
    apl_int_t   liStatus = apoMessage->GetInt(anf::BENC_TAG_ERROR_CODE, 0);
    apl_size_t  luSegmentID = apoMessage->GetInt(GFQ_TAG_SEGMENT_ID, INVALID_ID);
    char const* lpcNodeName = apoMessage->Get(GFQ_TAG_NODE_NAME, "");
    char const* lpcQueueName = apoMessage->Get(GFQ_TAG_QUEUE_NAME, "");
    CSegmentHandle loHandle = GetServerNodeManager()->Find(lpcNodeName, luSegmentID);
    
    if (!loHandle.IsInvalid() )
    {
        GetQueueManager()->AllocateSegmentFinished(lpcQueueName, loHandle, liStatus);
    }
    
    if (liStatus != 0)
    {
        AAF_LOG_ERROR("Request server node allocate segment fail, "
            "(NodeName=%s,SegmentID=%"APL_PRIuINT",Status=%"APL_PRIdINT")", 
            lpcNodeName,
            luSegmentID,
            liStatus );
    }
    else
    {
        AAF_LOG_DEBUG("Request server node allocate segment success, "
            "(NodeName=%s,SegmentID=%"APL_PRIuINT")", 
            lpcNodeName,
            luSegmentID );
    }
}

void CServerHandler::HandleDeallocateResponse( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage )
{
    apl_int_t   liStatus = apoMessage->GetInt(anf::BENC_TAG_ERROR_CODE, 0);
    apl_size_t  luSegmentID = apoMessage->GetInt(GFQ_TAG_SEGMENT_ID, INVALID_ID);
    char const* lpcNodeName = apoMessage->Get(GFQ_TAG_NODE_NAME, "");
    char const* lpcQueueName = apoMessage->Get(GFQ_TAG_QUEUE_NAME, "");
    CSegmentHandle loHandle = GetServerNodeManager()->Find(lpcNodeName, luSegmentID);
    
    if (!loHandle.IsInvalid() )
    {
        GetQueueManager()->DeallocateSegmentFinished(lpcQueueName, loHandle, liStatus);
    }

    if (liStatus != 0)
    {
        AAF_LOG_ERROR("Request server node deallocate segment fail, "
            "(NodeName=%s,SegmentID=%"APL_PRIuINT",QueueuName=%s,Status=%"APL_PRIdINT")", 
            lpcNodeName,
            luSegmentID,
            lpcQueueName,
            liStatus );
    }
    else
    {
        AAF_LOG_DEBUG("Request server node deallocate segment success, "
            "(NodeName=%s,SegmentID=%"APL_PRIuINT")", 
            lpcNodeName,
            luSegmentID );
    }
}

void CServerHandler::WriteMessage(
    anf::SessionPtrType& aoSession, 
    apl_size_t auMessageID, 
    apl_size_t auCommandID,
    apl_int_t aiStatus, 
    acl::CIndexDict& aoMessage )
{
    aoMessage.Set(anf::BENC_TAG_SEQ_NO, auMessageID);
    aoMessage.Set(anf::BENC_TAG_CMD_ID, auCommandID);
    aoMessage.Set(anf::BENC_TAG_ERROR_CODE, aiStatus);

    anf::CWriteFuture::PointerType loFuture = aoSession->Write(aoSession, &aoMessage);
    if (loFuture->IsException() )
    {
        AAF_LOG_ERROR("ServerHandler write message fail, (State=%"APL_PRIdINT",Errno=%"APL_PRIdINT")", 
            loFuture->GetState(), loFuture->GetErrno() );
    }
}

AIBC_GFQ_NAMESPACE_END

