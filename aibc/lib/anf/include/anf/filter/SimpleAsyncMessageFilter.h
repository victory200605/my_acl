
#ifndef ANF_SIMPLEASYNCMESSAGEFILTER_H
#define ANF_SIMPLEASYNCMESSAGEFILTER_H

#include "anf/IoFilter.h"

ANF_NAMESPACE_START

class CSimpleAsyncMessageFilter : public CIoFilter
{
public:
    CSimpleAsyncMessageFilter(void);

    virtual ~CSimpleAsyncMessageFilter(void);

    virtual void SessionIdle(
        IoNextFilter* apoNextFilter,
        SessionPtrType& aoSession,
        apl_int_t aiStatus );

    /**
     * Filters IoHandler#MessageReceived(...) event.
     */
    virtual void MessageReceived(
        IoNextFilter* apoNextFilter,
        SessionPtrType& aoSession,
        acl::CAny const& aoMessage );

    /**
     * Filters IoSession#Write(...) method invocation.
     */
    virtual void FilterWrite(
        IoNextFilter* apoNextFilter,
        SessionPtrType& aoSession,
        CWriteFuture::PointerType& aoWriteFuture );
    
    virtual CIoFilter* Clone(void);

private:
    CSimpleAsyncMessage* mpoMessage;
    acl::CAny moMessage;
};

ANF_NAMESPACE_END

#endif//ANF_SIMPLEASYNCMESSAGEFILTER_H
