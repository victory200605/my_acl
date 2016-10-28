
#ifndef AIBC_GFQ_EVENTMESSAGE_H
#define AIBC_GFQ_EVENTMESSAGE_H

#include "gfq2/Utility.h"
#include "acl/MsgQueue.h"
#include "acl/Singleton.h"

AIBC_GFQ_NAMESPACE_START

enum EQueueEvent
{
    EVENT_QUEUE_CREATE    = 0x01,
    EVENT_QUEUE_DESTROY   = 0x02,
    EVENT_QUEUE_EMPTY     = 0x03,
    EVENT_QUEUE_NONEMPTY  = 0x04,
    EVENT_QUEUE_FULL      = 0x05,
};

enum ENodeEvent
{
    EVENT_NODE_REGISTER   = 0x01,
    EVENT_NODE_UNREGISTER = 0x02
};

enum EGFQEventClass
{
    CLASS_QUEUE_EVENT = 1,
    CLASS_NODE_EVENT
};

/** 
 * @brief Global File Queue read or write event message
 */
class CEventMessage
{
public:
    CEventMessage(void)
        : miClass(0)
        , miEvent(0)
    {
        this->macProvider[0] = '\0';
    }

    CEventMessage( apl_int_t aiClass, char const* apcName, apl_int_t aiEvent )
        : miClass(aiClass)
        , miEvent(aiEvent)
    {
        apl_strncpy(this->macProvider, apcName, sizeof(this->macProvider) );
    }

    apl_int_t GetClass(void)
    {
        return this->miClass;
    }

    char const* GetProvider(void)
    {
        return this->macProvider;
    }

    apl_int_t GetEvent(void)
    {
        return this->miEvent;
    }

private:
    apl_int_t miClass;
    char macProvider[APL_NAME_MAX];
    apl_int_t miEvent;
};

AIBC_GFQ_NAMESPACE_END

#endif//AIBC_GFQ_EVENTMESSAGE_H

