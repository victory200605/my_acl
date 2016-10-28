
#ifndef AIBC_GFQ_EVENTOBSERVER_H
#define AIBC_GFQ_EVENTOBSERVER_H

#include "EventMessage.h"
#include "anf/IoSession.h"
#include "Ring.h"
#include "acl/MsgQueue.h"

AIBC_GFQ_NAMESPACE_START

typedef void (*EventHandleFuncType)(anf::SessionPtrType& aoSession, apl_int_t aiClass, char const* apcProvider, apl_int_t aiEvent );

class CEventObserver
{
    struct CEventNode : public TRingEntry<2>
    {
        CEventNode( 
            anf::SessionPtrType& aoSession,
            apl_int_t aiClass,
            apl_int_t aiEvent,
            EventHandleFuncType afHandler )
            : moSession(aoSession)
            , miClass(aiClass)
            , miEvent(aiEvent)
            , mfHandler(afHandler)
        {
        }

        anf::SessionPtrType moSession;
        apl_int_t miClass;
        apl_int_t miEvent;
        EventHandleFuncType mfHandler;
    };

    typedef TRing<2,0> EventRingType;
    typedef TRingIterator<CEventNode, 2, 0> EventRingIterType;
    typedef TRing<2,1> SessionRingType;
    typedef TRingIterator<CEventNode, 2, 1> SessionRingIterType;
    typedef std::map<apl_int_t, EventRingType> EventMapType;
    typedef std::map<apl_int_t, EventMapType> ClassMapType;
    typedef std::map<apl_uint64_t, SessionRingType> SessionMapType;

public:
    CEventObserver(void);

    ~CEventObserver(void);

    apl_int_t Initialize(void);

    void Close(void);

    apl_int_t Register(
        anf::SessionPtrType& aoSession, 
        apl_int_t aiClass, 
        apl_int_t aiEvent,
        EventHandleFuncType afHandler );

    apl_int_t Remove( anf::SessionPtrType& aoSession );

    apl_int_t SendEvent( apl_int_t aiClass, char const* apcProvider, apl_int_t aiEvent );

    apl_int_t SendEvent( apl_int_t aiClass, std::string const& aoProvider, apl_int_t aiEvent );

    void SetEventQueueCapacity( apl_size_t auCapacity );

protected:
    void HandleEvent( apl_int_t aiClass, char const* apcProvider, apl_int_t aiEvent );

    static void* Srv( void* apvParam );

private:
    bool mbIsClosed;
    
    apl_size_t muEventQueueCapacity;

    apl_time_t miHandleInterval;//calculate by speed

    acl::CLock moLock;

    SessionMapType moSessionMap;

    ClassMapType moClassMap;

    acl::TMsgQueue<CEventMessage> moEventQueue;
};

#define GFQ_SEND_EVENT( class, name, event ) \
    { \
        acl::Instance<CEventObserver>()->SendEvent(class, name, event);\
    }

AIBC_GFQ_NAMESPACE_END

#endif//AIBC_GFQ_EVENTOBSERVER_H

