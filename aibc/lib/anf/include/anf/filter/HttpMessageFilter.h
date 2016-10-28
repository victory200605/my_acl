#ifndef ANF_HTTP_MESSAGEFILTER_H
#define ANF_HTTP_MESSAGEFILTER_H

#include "anf/IoFilter.h"

ANF_NAMESPACE_START

//http body limit size to 10Mb
const apl_size_t ANF_HTTP_LIMIT_BODY_SIZE        = 1073741824;

class CHttpMessageFilter : public CIoFilter
{
public:
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
};

ANF_NAMESPACE_END

#endif//ANF_HTTP_MESSAGEFILTER_H
