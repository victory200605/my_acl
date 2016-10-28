/**
 * @file CMPP20MessageFilter.h
 */

#ifndef CMPP20_MESSAGE_FILTER_H
#define CMPP20_MESSAGE_FILTER_H 

#include "anf/IoSession.h"
#include "anf/Utility.h"
#include "anf/IoFilter.h"
#include "anf/protocol/CMPPMessage.h"

ANF_NAMESPACE_START

class CCMPP20MessageFilter : public CIoFilter
{
public:
    CCMPP20MessageFilter(void);

    virtual ~CCMPP20MessageFilter(void);

    virtual void MessageReceived(
        IoNextFilter* apoNextFilter,
        SessionPtrType& aoSession,
        acl::CAny const& aoMessage );

    virtual void FilterWrite(
        IoNextFilter* apoNextFilter,
        SessionPtrType& aoSession,
        CWriteFuture::PointerType& aoWriteFuture );

    virtual CIoFilter* Clone(void);

private:
    cmpp::CCMPP20Message* mpoMessage;
    acl::CAny             moMessage;
};

ANF_NAMESPACE_END

#endif //CMPP20_MESSAGE_FILTER_H
