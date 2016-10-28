
#ifndef AIBC_GFQ_SERVERHANDLER_H
#define AIBC_GFQ_SERVERHANDLER_H

#include "gfq2/Utility.h"
#include "anf/IoHandler.h"
#include "aaf/ConfigService.h"
#include "ServerNodeManager.h"
#include "QueueManager.h"

AIBC_GFQ_NAMESPACE_START

class CServerHandler : public anf::IoHandler
{
public:
    CServerHandler(void);

    virtual ~CServerHandler(void);
    
    virtual void Exception( anf::CThrowable const& aoCause );
    
    virtual void SessionException( anf::SessionPtrType& aoSession, anf::CThrowable const& aoCause );

    virtual void SessionOpened( anf::SessionPtrType& aoSession );

    virtual void SessionClosed( anf::SessionPtrType& aoSession );

    virtual void SessionIdle( anf::SessionPtrType& aoSession, apl_int_t aiStatus );

    virtual void MessageReceived( anf::SessionPtrType& aoSession, acl::CAny const& aoMessage );

    virtual void MessageFinished( anf::SessionPtrType& aoSession, anf::CWriteFuture::PointerType& aoWriteFuture );

protected:
    void HandleReadableSegmentRequest( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage );

    void HandleWriteableSegmentRequest( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage );
    
    void HandlePollQueueRequest( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage );
    
    void HandleCreateQueueRequest( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage );
    
    void HandleStartupTimestampRequest( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage );

    void HandleEventRegisterRequest( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage );
    
    void HandleRegisterRequest( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage );

    void HandleUnregisterRequest( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage );
    
    void HandleAddSegmentRequest( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage );
    
    void HandleOnlineRequest( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage );
    
    void HandleUpdateSegmentRequest( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage );
    
    void HandleUpdateStatInfoRequest(anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage );

    void HandleQueueListRequest( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage );
    
    void HandleQueueInfoRequest( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage );

    void HandleNodeListRequest( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage );

    void HandleNodeInfoRequest( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage );
    
    void HandleBindNodeGroupRequest( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage );
    
    void HandleHeartbeatRequest( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage );

    void HandleAllocateResponse( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage );

    void HandleDeallocateResponse( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage );

protected:
    void WriteMessage(
        anf::SessionPtrType& aoSession, 
        apl_size_t auMessageID, 
        apl_size_t auCommandID,
        apl_int_t aiStatus, 
        acl::CIndexDict& aoMessage );

private:
    acl::CTimestamp moStartupTime;
};

AIBC_GFQ_NAMESPACE_END

#endif//GFQ_SERVERHANDLER_H

