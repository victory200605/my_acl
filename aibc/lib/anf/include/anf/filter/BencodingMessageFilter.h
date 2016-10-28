
#ifndef ANF_BENCODINGMESSAGEFILTER_H
#define ANF_BENCODINGMESSAGEFILTER_H

#include "acl/IndexDict.h"
#include "anf/IoFilter.h"

ANF_NAMESPACE_START

class CBencodingMessageFilter : public CIoFilter
{
public:
    CBencodingMessageFilter();

    virtual ~CBencodingMessageFilter();

    // 20120109
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
    acl::CIndexDict*    mpoIndexDict;
    acl::CAny           moMessage;
};

ANF_NAMESPACE_END

#endif//ANF_SIMPLEASYNCMESSAGEFILTER_H
