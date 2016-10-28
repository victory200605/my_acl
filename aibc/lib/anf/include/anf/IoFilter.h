
#ifndef ANF_IOFILTER_H
#define ANF_IOFILTER_H

#include "acl/Any.h"
#include "anf/Utility.h"
#include "anf/Exception.h"
#include "anf/future/WriteFuture.h"

ANF_NAMESPACE_START

//Service class forward declare
class CIoService;

class IoNextFilter
{
public:
    virtual ~IoNextFilter(void){};

    /**
     * Filters SessionOpened event.
     */
    virtual void SessionOpened( SessionPtrType& aoSession ) = 0;

    /**
     * Filters SessionClosed event.
     */
    virtual void SessionClosed( SessionPtrType& aoSession ) = 0;

    /**
     * Filters SessionIdle event.
     */
    virtual void SessionIdle( SessionPtrType& aoSession, apl_int_t aiStatus ) = 0;

    /**
     * Filters Exception event.
     */
    //virtual void Exception( CIoService* apoService, CThrowable const& aoCause ) = 0;
    
    /**
     * Filters SessionException event.
     */
    virtual void SessionException( SessionPtrType& aoSession, CThrowable const& aoCause ) = 0;

    /**
     * Filters MessageReceived event.
     */
    virtual void MessageReceived( SessionPtrType& aoSession, acl::CAny const& aoMessage ) = 0;

    /**
     * Filters MessageFinished event.
     */
    virtual void MessageFinished( SessionPtrType& aoSession, CWriteFuture::PointerType& aoWriteFuture ) = 0;
    
    /**
     * Filters CIoSession#Close() method invocation.
     */
    virtual void FilterClose( SessionPtrType& aoSession ) = 0;

    /**
     * Filters IoSession#Write(...} method invocation.
     */
    virtual void FilterWrite( SessionPtrType& aoSession, CWriteFuture::PointerType& aoWriteFuture ) = 0;
};

class CIoFilter
{
public:
    virtual ~CIoFilter(void){};

    /**
     * Invoked when this filter is added to a CIoFilterChain at the first time,
     * so you can initialize shared resources.
     */
    virtual void Init(void);

    /**
     * Invoked when this filter is not used by any CIoFilterChain anymore,
     * so you can destroy shared resources.
     */
    virtual void Destroy(void);

    /**
     * Filters IoHandler#SessionOpened(...) event.
     */
    virtual void SessionOpened(
        IoNextFilter* apoNextFilter,
        SessionPtrType& aoSession );

    /**
     * Filters IoHandler#SessionClosed(...) event.
     */
    virtual void SessionClosed(
        IoNextFilter* apoNextFilter,
        SessionPtrType& aoSession );

    /**
     * Filters IoHandler#SessionIdle(...) event.
     */
    virtual void SessionIdle(
        IoNextFilter* apoNextFilter,
        SessionPtrType& aoSession,
        apl_int_t aiStatus );
    
    /**
     * Filters IoHandler#Exception(...) event.
     */
    //virtual void Exception(
    //    IoNextFilter* apoNextFilter,
    //    CIoService* apoService,
    //    CThrowable const& aoCause );
    
    /**
     * Filters IoHandler#SessionException(...) event.
     */
    virtual void SessionException(
        IoNextFilter* apoNextFilter,
        SessionPtrType& aoSession,
        CThrowable const& aoCause );

    /**
     * Filters IoHandler#MessageReceived(...) event.
     */
    virtual void MessageReceived(
        IoNextFilter* apoNextFilter,
        SessionPtrType& aoSession,
        acl::CAny const& aoMessage );

    /**
     * Filters IoHandler#MessageFinished(...) event.
     */
    virtual void MessageFinished(
        IoNextFilter* apoNextFilter,
        SessionPtrType& aoSession,
        CWriteFuture::PointerType& aoWriteFuture );

    /**
     * Filters CIoSession#Close(...) method invocation.
     */
    virtual void FilterClose(
        IoNextFilter* apoNextFilter,
        SessionPtrType& aoSession );

    /**
     * Filters IoSession#Write(...) method invocation.
     */
    virtual void FilterWrite(
        IoNextFilter* apoNextFilter,
        SessionPtrType& aoSession,
        CWriteFuture::PointerType& aoWriteFuture );
    
    /** 
     * @brief Filter clone hook
     */
    virtual CIoFilter* Clone(void) = 0;
};

ANF_NAMESPACE_END

#endif //ANF_IOFILTER_H
