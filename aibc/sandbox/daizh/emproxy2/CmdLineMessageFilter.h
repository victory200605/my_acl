#ifndef AIMC_EMPROXY_CMDLINEMESSAGEFILTER_H
#define AIMC_EMPROXY_CMDLINEMESSAGEFILTER_H

#include "anf/IoFilter.h"
#include "CmdLineMessage.h"

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

#endif//AIMC_EMPROXY_CMDLINEMESSAGEFILTER_H

