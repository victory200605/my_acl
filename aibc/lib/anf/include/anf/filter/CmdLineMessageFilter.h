#ifndef ANF_CMDLINEMESSAGEFILTER_H
#define ANF_CMDLINEMESSAGEFILTER_H

#include "anf/IoFilter.h"
#include "anf/protocol/CmdLineMessage.h"

ANF_NAMESPACE_START

class CCmdLineMessageFilter : public anf::CIoFilter
{
public:
    CCmdLineMessageFilter(void);

    virtual ~CCmdLineMessageFilter(void);

    virtual void MessageReceived(
        anf::IoNextFilter* apoNextFilter,
        anf::SessionPtrType& aoSession,
        acl::CAny const& aoMessage );

    virtual void FilterWrite(
        anf::IoNextFilter* apoNextFilter,
        anf::SessionPtrType& aoSession,
        anf::CWriteFuture::PointerType& aoWriteFuture );
    
    virtual CIoFilter* Clone(void);

private:
    CCmdLineMessage* mpoMessage;
    acl::CAny moMessage;
};

ANF_NAMESPACE_END

#endif//ANF_CMDLINEMESSAGEFILTER_H

