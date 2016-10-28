#ifndef SMMCMESSAGEFILTER_H
#define SMMCMESSAGEFILTER_H

#include "anf/IoSession.h"
#include "anf/Utility.h"
#include "anf/protocol/SMMCMessage.h"
#include "anf/IoFilter.h"

ANF_NAMESPACE_START

class CSMMCMessageFilter : public CIoFilter
{
public:
    virtual void MessageReceived(
        IoNextFilter* apoNextFilter,
        SessionPtrType& aoSession,
        acl::CAny const& aoMessage );

    virtual void FilterWrite(
        IoNextFilter* apoNextFilter,
        SessionPtrType& aoSession,
        CWriteFuture::PointerType& aoWriteFuture );

    virtual CIoFilter* Clone(void);
};

ANF_NAMESPACE_END

#endif
