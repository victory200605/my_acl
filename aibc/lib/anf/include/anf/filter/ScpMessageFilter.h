
#ifndef ANF_SCP_MESSAGEFILTER_H
#define ANF_SCP_MESSAGEFILTER_H

#include "anf/IoFilter.h"

ANF_NAMESPACE_START

class CScpMessageFilter : public CIoFilter
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

#endif//ANF_SCP_MESSAGEFILTER_H
