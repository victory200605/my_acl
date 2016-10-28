
#ifndef AIBC_GFQ_SERVERHANDLER_H
#define AIBC_GFQ_SERVERHANDLER_H

#include "anf/IoHandler.h"
#include "gfq2/Utility.h"
#include "SegmentQueue.h"

AIBC_GFQ_NAMESPACE_START

class CServerHandler : public anf::IoHandler
{
public:
    CServerHandler( ISegmentQueue* apoSegmentQueue );

    virtual ~CServerHandler(void);
    
    virtual void Exception( anf::CThrowable const& aoCause );
    
    virtual void SessionException( anf::SessionPtrType& aoSession, anf::CThrowable const& aoCause );

    virtual void SessionOpened( anf::SessionPtrType& aoSession );

    virtual void SessionClosed( anf::SessionPtrType& aoSession );

    virtual void SessionIdle( anf::SessionPtrType& aoSession, apl_int_t aiStatus );

    virtual void MessageReceived( anf::SessionPtrType& aoSession, acl::CAny const& aoMessage );

    virtual void MessageFinished( anf::SessionPtrType& aoSession, anf::CWriteFuture::PointerType& aoWriteFuture );

    apl_int64_t GetReferences(void);
    
    apl_uint64_t GetReadTimes(void);

    apl_uint64_t GetWriteTimes(void);

protected:
    void HandlePushRequest( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage );
    
    void HandlePopRequest( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage );
    
    void HandlePopTimedoutRequest( anf::SessionPtrType& aoSession, acl::CIndexDict* apoMessage );

    void WriteMessage(
        anf::SessionPtrType& aoSession, 
        apl_size_t auMessageID, 
        apl_size_t auCommandID,
        apl_int_t aiStatus,
        acl::CIndexDict& aoMessage );

private:
    ISegmentQueue* mpoSegmentQueue;

    acl::TNumber<apl_int64_t> moReferences;
    
    acl::TNumber<apl_uint64_t> moReadTimes;

    acl::TNumber<apl_uint64_t> moWriteTimes;
};

AIBC_GFQ_NAMESPACE_END

#endif//GFQ_SERVERHANDLER_H

