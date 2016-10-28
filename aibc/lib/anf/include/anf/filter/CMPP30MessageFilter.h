/**
 * @file CMPP30MessageFilter.h
 */

#ifndef CMPP30_MESSAGE_FILTER_H
#define CMPP30_MESSAGE_FILTER_H 

#include "anf/IoSession.h"
#include "anf/Utility.h"
#include "anf/IoFilter.h"
#include "anf/protocol/CMPPMessage.h"

ANF_NAMESPACE_START

class CCMPP30MessageFilter : public CIoFilter
{
public:
    CCMPP30MessageFilter(void);

    virtual ~CCMPP30MessageFilter(void);

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
    cmpp::CCMPP30Message* mpoMessage;
    acl::CAny             moMessage;
};

ANF_NAMESPACE_END

#endif //CMPP30_MESSAGE_FILTER_H
