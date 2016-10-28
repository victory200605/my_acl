
#ifndef AIMC_EMPROXY_SERVERHANDLER_H
#define AIMC_EMPROXY_SERVERHANDLER_H

#include "anf/IoHandler.h"

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
};

#endif//AIMC_EMPROXY_SERVERHANDLER_H

