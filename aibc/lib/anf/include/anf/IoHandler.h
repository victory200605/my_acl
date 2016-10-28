
#ifndef ANF_IOHANDLE_H
#define ANF_IOHANDLE_H

#include "anf/Utility.h"
#include "anf/Exception.h"
#include "anf/IoSession.h"
#include "acl/Any.h"

ANF_NAMESPACE_START

/**
 * Handles all I/O events fired by ANF.
 */
class IoHandler
{
public:
    virtual ~IoHandler(void) {};

    /**
     * Invoked when a exception caught.
     */
    virtual void Exception( CThrowable const& aoCause ){};
    
    /**
     * Invoked when a session exception caught.
     */
    virtual void SessionException( SessionPtrType& aoSession, CThrowable const& aoCause ){};

    /**
     * Invoked when a connection has been opened.
     */
    virtual void SessionOpened( SessionPtrType& aoSession ) {};

    /**
     * Invoked when a connection is closed.
     */
    virtual void SessionClosed( SessionPtrType& aoSession ) {};

    /**
     * Invoked when a connection becomes idle.
     */
    virtual void SessionIdle( SessionPtrType& aoSession, apl_int_t aiStatus ) {};

    /**
     * Invoked when a message is received.
     */
    virtual void MessageReceived( SessionPtrType& aoSession, acl::CAny const& aoMessage ) {};

    /**
     * Invoked when a message written by session is sent out.
     */
    virtual void MessageFinished( SessionPtrType& aoSession, CWriteFuture::PointerType& aoWriteFuture ) {};
};

ANF_NAMESPACE_END

#endif //ANF_IOHANDLE_H
