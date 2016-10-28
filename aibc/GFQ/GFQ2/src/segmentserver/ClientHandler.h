
#ifndef AIBC_GFQ_CLIENTHANDLER_H
#define AIBC_GFQ_CLIENTHANDLER_H

#include "anf/IoHandler.h"
#include "gfq2/Utility.h"
#include "gfq2/GFQMessage.h"
#include "acl/IndexDict.h"
#include "SegmentQueue.h"

AIBC_GFQ_NAMESPACE_START

class CClientHandler : public anf::IoHandler
{
public:
    CClientHandler( 
        char const* apcNodeName,
        char const* apcGroupName,
        char const* apcStorageType,
        char const* apcServerIp,
        apl_uint16_t au16ServerPort,
        apl_int_t aiPriority,
        apl_int_t aiSegmentInfoSyncInterval,
        apl_int_t aiStatInfoSyncInterval,
        ISegmentQueue* apoSegmentQueue );

    virtual ~CClientHandler(void);

    anf::SessionPtrType GetSession(void);

//IoHandler hook
protected:
    virtual void Exception( anf::CThrowable const& aoCause );
    
    virtual void SessionException( anf::SessionPtrType& aoSession, anf::CThrowable const& aoCause );

    virtual void SessionOpened( anf::SessionPtrType& aoSession );

    virtual void SessionClosed( anf::SessionPtrType& aoSession );

    virtual void SessionIdle( anf::SessionPtrType& aoSession, apl_int_t aiStatus );

    virtual void MessageReceived( anf::SessionPtrType& aoSession, acl::CAny const& aoMessage );

    virtual void MessageFinished( anf::SessionPtrType& aoSession, anf::CWriteFuture::PointerType& aoWriteFuture );

//Send request method
public:
    void SendRegisterRequest( anf::SessionPtrType& aoSession );

    void SendUnregisterRequest( anf::SessionPtrType& aoSession );
    
    void SendAddSegmentRequest( anf::SessionPtrType& aoSession );
    
    void SendUpdateSegmentRequest( anf::SessionPtrType& aoSession );
    
    void SendUpdateStatInfoRequest( anf::SessionPtrType& aoSession, apl_int_t aiInterval );

    void SendHeartbeatRequest( anf::SessionPtrType& aoSession );

protected:
//Handle method
    void HandleAllocateRequest( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage );

    void HandleDeallocateRequest( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage );

    void HandleRegisterResponse( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage );

    void HandleUnregisterResponse( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage );

    void HandleAddSegmentResponse( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage );
    
    void HandleOnlineResponse( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage );

    void HandleUpdateSegmentResponse( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage );
    
    void HandleUpdateStatInfoResponse( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage );

    void HandleHeartbeatResponse( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage );

    void WriteMessage(
        anf::SessionPtrType& aoSession, 
        apl_size_t auMessageID, 
        apl_size_t auCommandID,
        acl::CIndexDict& aoMessage );

private:
    //Server Node Attribute
    std::string  moNodeName;

    std::string  moGroupName;
    
    std::string  moStorageType;
    
    std::string  moServerIp;
    
    apl_uint16_t mu16ServerPort;

    apl_int_t    miNodePriority;

    //Segment Queue
    ISegmentQueue* mpoSegmentQueue;
 
    //Connection
    acl::CLock moLock;
        
    anf::SessionPtrType moSession;

    bool mbIsOnlined;

    //Event
    acl::IEventHandler* mpoSegmentSyncHandler;
    
    acl::IEventHandler* mpoStatInfoSyncHandler;
    
    std::vector<CSegmentInfo> moPrevSegmentInfos;
};

AIBC_GFQ_NAMESPACE_END

#endif//GFQ_CLIENTHANDLER_H

