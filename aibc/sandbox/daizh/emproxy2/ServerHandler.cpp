
#include "acl/Singleton.h"
#include "CmdLineMessage.h"
#include "ServerHandler.h"
#include "GetMailTask.h"
#include "TaskProcessor.h"
#include "Utility.h"

CServerHandler::CServerHandler(void)
{
}

CServerHandler::~CServerHandler(void)
{
}
    
void CServerHandler::Exception( anf::CThrowable const& aoCause )
{
    EMP_LOG_ERROR("ServerHandler catch a exception,%s (State=%"APL_PRIdINT",Errno=%"APL_PRIdINT")",
        aoCause.GetMessage(),
        (apl_int_t)aoCause.GetState(),
        (apl_int_t)aoCause.GetErrno() )
}
    
void CServerHandler::SessionException( anf::SessionPtrType& aoSession, anf::CThrowable const& aoCause )
{
    EMP_LOG_INFO("ServerHandler catch session exception,%s "
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
    EMP_LOG_INFO("ServerHandler add connection to segment server, (Remote=%s:%"APL_PRIdINT",SessionID=%"APL_PRIu64")",
        aoSession->GetRemoteAddress().GetIpAddr(),
        (apl_int_t)aoSession->GetRemoteAddress().GetPort(),
        (apl_uint64_t)aoSession->GetSessionID() );

    WRITE_MESSAGE(true, aoSession, "+OK %s\r\n", "Ready");
}

void CServerHandler::SessionClosed( anf::SessionPtrType& aoSession )
{
    EMP_LOG_INFO("ServerHandler closed connection to segment server, (Remote=%s:%"APL_PRIdINT",SessionID=%"APL_PRIu64")",
        aoSession->GetRemoteAddress().GetIpAddr(),
        (apl_int_t)aoSession->GetRemoteAddress().GetPort(),
        (apl_uint64_t)aoSession->GetSessionID() );
}

void CServerHandler::SessionIdle( anf::SessionPtrType& aoSession, apl_int_t aiStatus )
{
}

void CServerHandler::MessageReceived( anf::SessionPtrType& aoSession, acl::CAny const& aoMessage )
{
    apl_int_t liRetCode = 0;
    CCmdLineMessage* lpoMessage = APL_NULL;
    
    //Cast message to scp message type
    aoMessage.CastTo<CCmdLineMessage*>(lpoMessage);
    if (lpoMessage == NULL)//Exception
    {
        //Exception, invalid message
        EMP_LOG_ERROR("ServerHandler session received invalid message, (Remote=%s:%"APL_PRIdINT",SessionID=%"APL_PRIu64")",
            aoSession->GetRemoteAddress().GetIpAddr(),
            (apl_int_t)aoSession->GetRemoteAddress().GetPort(),
            (apl_uint64_t)aoSession->GetSessionID() );
    
        WRITE_MESSAGE(true, aoSession, "-ERR %s\r\n", "invalid message format");

        aoSession->Close(aoSession);
        return;
    }

    if (apl_strcasecmp(lpoMessage->GetArgv(0), "GETMAIL") == 0)
    {
        CGetMailTask* lpoTask = APL_NULL;

        ACL_NEW_ASSERT(lpoTask, CGetMailTask(
            aoSession,
            lpoMessage->GetArgv(GETMAIL_USERNAME_INDEX),
            apl_strtoi32(lpoMessage->GetArgv(GETMAIL_POPID_INDEX), APL_NULL, 10),
            apl_strtoi32(lpoMessage->GetArgv(GETMAIL_TRACELEVEL_INDEX), APL_NULL, 10) ) );
       
        if ( (liRetCode = acl::Instance<CTaskProcessor>()->Run(
            lpoMessage->GetArgv(GETMAIL_POPSERVER_INDEX), lpoTask) ) != 0)
        {
            ACL_DELETE(lpoTask);
        }
    }
    else if (apl_strcasecmp(lpoMessage->GetArgv(0), "TEST") == 0)
    {
    }
    else
    {
    }

    if (liRetCode != 0)
    {
        EMP_LOG_ERROR("ServerHandler dispath message fail, maybe queue is full "
            "(Remote=%s:%"APL_PRIdINT",SessionID=%"APL_PRIu64",Waiting=%"APL_PRIuINT",Errno=%"APL_PRIdINT")",
            aoSession->GetRemoteAddress().GetIpAddr(),
            (apl_int_t)aoSession->GetRemoteAddress().GetPort(),
            (apl_uint64_t)aoSession->GetSessionID(),
            acl::Instance<CTaskProcessor>()->GetDispatchSize(),
            apl_get_errno() );

        WRITE_MESSAGE(true, aoSession, "-BUSY %s\r\n", "task queue is full");
    }
}

void CServerHandler::MessageFinished( anf::SessionPtrType& aoSession, anf::CWriteFuture::PointerType& aoWriteFuture )
{
    if (aoWriteFuture->IsException() )
    {
        EMP_LOG_ERROR("ServerHandler message flush exception, "
            "(State=%"APL_PRIdINT",Errno=%"APL_PRIdINT",SessionID=%"APL_PRIu64")",
            (apl_int_t)aoWriteFuture->GetState(),
            (apl_int_t)aoWriteFuture->GetErrno(),
            (apl_uint64_t)aoSession->GetSessionID() );
    }
    else
    {
        EMP_LOG_TRACE("ServerHandler message flush finished, (MsgID=%"APL_PRIuINT",SessionID=%"APL_PRIu64")",
            (apl_size_t)aoWriteFuture->GetAct(),
            (apl_uint64_t)aoSession->GetSessionID() );
    }
}

