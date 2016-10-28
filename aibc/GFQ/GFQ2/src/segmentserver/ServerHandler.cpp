
#include "aaf/LogService.h"
#include "acl/IndexDict.h"
#include "anf/protocol/BencodingMessage.h"
#include "gfq2/GFQMessage.h"
#include "ServerHandler.h"

AIBC_GFQ_NAMESPACE_START

CServerHandler::CServerHandler( ISegmentQueue* apoSegmentQueue )
    : mpoSegmentQueue(apoSegmentQueue)
{
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
    AAF_LOG_INFO("ServerHandler catch session exception,%s (Remote=%s:%"APL_PRIdINT",SessionID=%"APL_PRIu64",State=%"APL_PRIdINT",Errno=%"APL_PRIdINT")",
        aoCause.GetMessage(),
        aoSession->GetRemoteAddress().GetIpAddr(),
        (apl_int_t)aoSession->GetRemoteAddress().GetPort(),
        (apl_uint64_t)aoSession->GetSessionID(),
        (apl_int_t)aoCause.GetState(),
        (apl_int_t)aoCause.GetErrno() );
}

void CServerHandler::SessionOpened( anf::SessionPtrType& aoSession )
{
    //acl::Instance<CServerStatInfo>()->IncreaseConnections(1);

    AAF_LOG_INFO("ServerHandler add connection to segment server, (Remote=%s:%"APL_PRIdINT",SessionID=%"APL_PRIu64")",
        aoSession->GetRemoteAddress().GetIpAddr(),
        (apl_int_t)aoSession->GetRemoteAddress().GetPort(),
        (apl_uint64_t)aoSession->GetSessionID() );
}

void CServerHandler::SessionClosed( anf::SessionPtrType& aoSession )
{
    //acl::Instance<CServerStatInfo>()->DecreaseConnections(1);

    AAF_LOG_INFO("ServerHandler closed connection to segment server, (Remote=%s:%"APL_PRIdINT",SessionID=%"APL_PRIu64")",
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
        case GFQ_REQUEST_PUSH:
        {
            //TODO push
            this->HandlePushRequest(aoSession, lpoMessage);
            break;
        }
        case GFQ_REQUEST_POP:
        {
            //TODO pop
            this->HandlePopRequest(aoSession, lpoMessage);
            break;
        }
        case GFQ_REQUEST_POP_TIMEDOUT:
        {
            //TODO pop-timedout
            this->HandlePopTimedoutRequest(aoSession, lpoMessage);
            break;
        }
        default:
        {
            //Exception, invalid message
            AAF_LOG_ERROR("ServerHandler session received invalid message, (CmdID=0x%X,Remote=%s:%"APL_PRIdINT",SessionID=%"APL_PRIu64")",
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

//Send request method
void CServerHandler::HandlePushRequest( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage )
{
    acl::CIndexDict loResponse;
    
    //stat.
    //acl::Instance<CServerStatInfo>()->IncreaseWriteTimes(1);

    apl_size_t  luMessageID = apoMessage->GetInt(anf::BENC_TAG_SEQ_NO, 0);
    apl_size_t  luSegmentID = apoMessage->GetInt(GFQ_TAG_SEGMENT_ID, INVALID_ID);
    char const* lpcQueueName = apoMessage->Get(GFQ_TAG_QUEUE_NAME, "");
    acl::CIndexDict::ValueType loData = apoMessage->GetStr(GFQ_TAG_RECORD_DATA);

    apl_int_t liRetCode = this->mpoSegmentQueue->Push(
        luSegmentID, 
        lpcQueueName, 
        loData.GetStr(), 
        loData.GetLength() );
    if (liRetCode != 0)
    {
        AAF_LOG_DEBUG("ServerHandler handle push request fail, "
            "(SegmentID=%"APL_PRIuINT",RecordLength=%"APL_PRIuINT",Status=%"APL_PRIdINT")",
            luSegmentID,
            loData.GetLength(),
            liRetCode );
    }
    else
    {
        AAF_LOG_DEBUG("ClientHandler handle push request success, "
            "(SegmentID=%"APL_PRIuINT",RecordLength=%"APL_PRIuINT")",
            luSegmentID,
            loData.GetLength() );
    }
    
    this->WriteMessage(aoSession, luMessageID, GFQ_RESPONSE_PUSH, liRetCode, loResponse); 
}

void CServerHandler::HandlePopRequest( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage )
{
    apl_int_t liRetCode = 0;
    acl::CIndexDict loResponse;
    acl::CMemoryBlock loData;
    
    //stat.
    //acl::Instance<CServerStatInfo>()->IncreaseReadTimes(1);

    apl_size_t luMessageID = apoMessage->GetInt(anf::BENC_TAG_SEQ_NO, 0);
    apl_size_t luSegmentID  = apoMessage->GetInt(GFQ_TAG_SEGMENT_ID, INVALID_ID);
    char const* lpcQueueName = apoMessage->Get(GFQ_TAG_QUEUE_NAME, "");
    apl_size_t luCount = apoMessage->GetInt(GFQ_TAG_RECORD_LIMIT, 1);
    apl_size_t luN = 0;

    for (luN = 0; luN < luCount; luN++)
    {
        liRetCode = this->mpoSegmentQueue->Pop(luSegmentID, lpcQueueName, 0, loData);
        if (liRetCode != 0)
        {
            break;
        }
        else
        {
            loResponse.Add(GFQ_TAG_RECORD_DATA, loData.GetReadPtr(), loData.GetLength() );
        }

        loData.Reset();
    }
    
    if (luN == 0 && liRetCode != 0)
    {
        AAF_LOG_DEBUG("ServerHandler handle pop request fail, "
            "(SegmentID=%"APL_PRIuINT",Status=%"APL_PRIdINT")",
            luSegmentID,
            liRetCode );
    }
    else
    {
        AAF_LOG_DEBUG("ClientHandler handle pop request success, "
            "(SegmentID=%"APL_PRIuINT",Count=%"APL_PRIuINT")",
            luSegmentID,
            luN );
    }

    this->WriteMessage(aoSession, luMessageID, GFQ_RESPONSE_POP, liRetCode, loResponse); 
}

void CServerHandler::HandlePopTimedoutRequest( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage )
{
    apl_int_t liRetCode = 0;
    acl::CIndexDict loResponse;
    acl::CMemoryBlock loData;
    
    //stat.
    //acl::Instance<CServerStatInfo>()->IncreaseReadTimes(1);

    apl_size_t luMessageID = apoMessage->GetInt(anf::BENC_TAG_SEQ_NO, 0);
    apl_size_t luSegmentID  = apoMessage->GetInt(GFQ_TAG_SEGMENT_ID, INVALID_ID);
    char const* lpcQueueName = apoMessage->Get(GFQ_TAG_QUEUE_NAME, "");
    apl_int_t liTimedout = apoMessage->GetInt(GFQ_TAG_TIMEDOUT, 0);
    apl_size_t luCount = apoMessage->GetInt(GFQ_TAG_RECORD_LIMIT, 1);
    apl_size_t luN = 0;

    for (luN = 0; luN < luCount; luN++)
    {
        liRetCode = this->mpoSegmentQueue->Pop(luSegmentID, lpcQueueName, liTimedout, loData);
        if (liRetCode != 0)
        {
            break;
        }
        else
        {
            loResponse.Add(GFQ_TAG_RECORD_DATA, loData.GetReadPtr(), loData.GetLength() );
        }

        loData.Reset();
    }
    
    if (luN == 0 && liRetCode != 0)
    {
        AAF_LOG_DEBUG("ServerHandler handle pop timedout request fail, "
            "(SegmentID=%"APL_PRIuINT",Status=%"APL_PRIdINT")",
            luSegmentID,
            liRetCode );
    }
    else
    {
        AAF_LOG_DEBUG("ClientHandler handle pop timedout request success, "
            "(SegmentID=%"APL_PRIuINT",Count=%"APL_PRIuINT")",
            luSegmentID,
            luN );
    }
    
    this->WriteMessage(aoSession, luMessageID, GFQ_RESPONSE_POP_TIMEDOUT, liRetCode, loResponse); 
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
        AAF_LOG_ERROR("ClientHandler write message fail, (State=%"APL_PRIdINT",Errno=%"APL_PRIdINT")", 
            loFuture->GetState(), loFuture->GetErrno() );
    }
}

AIBC_GFQ_NAMESPACE_END

