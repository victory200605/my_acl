
#include "EventObserver.h"
#include "acl/Thread.h"

AIBC_GFQ_NAMESPACE_START

////////////////////////////////////////////EventHandler////////////////////////////////////////////////
template<class T> struct TEventHandler
{
    bool operator () ( T* apoNode, char const* apcProvider, apl_int_t aiEvent )
    {
        if (apoNode->miEvent == 0 || ACL_BIT_ENABLED(apoNode->miEvent, aiEvent) )
        {
            (*apoNode->mfHandler)(apoNode->moSession, apoNode->miClass, apcProvider, aiEvent);
        }

        return true;
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////
CEventObserver::CEventObserver(void)
    : mbIsClosed(false)
    , muEventQueueCapacity(10000)
{
}

CEventObserver::~CEventObserver(void)
{
}

apl_int_t CEventObserver::Initialize(void)
{
    return acl::CThread::Spawn(CEventObserver::Srv, this);
}

void CEventObserver::Close(void)
{
    this->mbIsClosed = true;
}

apl_int_t CEventObserver::Register(
    anf::SessionPtrType& aoSession, 
    apl_int_t aiClass, 
    apl_int_t aiEvent,
    EventHandleFuncType afHandler )
{
    CEventNode* lpoNode = APL_NULL;

    acl::TSmartLock<acl::CLock> loGuard(this->moLock);

    SessionRingType& loSessionRing = this->moSessionMap[aoSession->GetSessionID()];

    for ( SessionRingIterType loIter(loSessionRing); !loIter.IsDone(); loIter.Next() )
    {
        //If existed and return immediately
        if (aiClass == loIter->miClass && aiEvent == loIter->miEvent)
        {
            return 0;
        }
    }

    ACL_NEW_ASSERT(lpoNode, CEventNode(aoSession, aiClass, aiEvent, afHandler) );
    loSessionRing.InsertTail(lpoNode);

    EventRingType& loEventRing = this->moClassMap[aiClass][aiEvent];
    loEventRing.InsertTail(lpoNode);

    return 0;
}

apl_int_t CEventObserver::Remove( anf::SessionPtrType& aoSession )
{
    acl::TSmartLock<acl::CLock> loGuard(this->moLock);

    SessionMapType::iterator loIter = this->moSessionMap.find(aoSession->GetSessionID() );
    if (loIter == this->moSessionMap.end() )
    {
        return 0;
    }

    SessionRingType& loSessionRing = loIter->second;
    while(true)
    {
        SessionRingIterType loFirst(loSessionRing);
        if (loFirst.IsDone() )
        {
            break;
        }

        CEventNode* lpoNode = &(*loFirst);
        ACL_DELETE(lpoNode);
    }
    
    this->moSessionMap.erase(loIter);

    return 0;
}

void CEventObserver::HandleEvent( apl_int_t aiClass, char const* apcProvider, apl_int_t aiEvent )
{
    acl::TSmartLock<acl::CLock> loGuard(this->moLock);

    ClassMapType::iterator loClassIter = this->moClassMap.find(aiClass);
    if (loClassIter == this->moClassMap.end() )
    {
        return;
    }

    EventMapType::iterator loEventIter = loClassIter->second.find(aiEvent);
    if (loEventIter == loClassIter->second.end() )
    {
        return;
    }

    for (EventRingIterType loIter(loEventIter->second); !loIter.IsDone(); loIter.Next() )
    {
        CEventNode* lpoNode = &(*loIter);
        (*lpoNode->mfHandler)(lpoNode->moSession, lpoNode->miClass, apcProvider, aiEvent);
    }
}

apl_int_t CEventObserver::SendEvent( apl_int_t aiClass, char const* apcProvider, apl_int_t aiEvent )
{
    if (this->moEventQueue.GetSize() < this->muEventQueueCapacity)
    {
        return this->moEventQueue.Push(CEventMessage(aiClass, apcProvider, aiEvent) );
    }
    else
    {
        return -1;
    }
}

apl_int_t CEventObserver::SendEvent( apl_int_t aiClass, std::string const& aoProvider, apl_int_t aiEvent )
{
    if (this->moEventQueue.GetSize() < this->muEventQueueCapacity)
    {
        return this->moEventQueue.Push(CEventMessage(aiClass, aoProvider.c_str(), aiEvent) );
    }
    else
    {
        return -1;
    }
}

void CEventObserver::SetEventQueueCapacity( apl_size_t auCapacity )
{
    this->muEventQueueCapacity = auCapacity;
}

void* CEventObserver::Srv( void* apvParam )
{
    CEventObserver* lpoObserver = static_cast<CEventObserver*>(apvParam);
    CEventMessage   loMessage;
    acl::CTimeValue loTimeout(1);

    while(!lpoObserver->mbIsClosed)
    {
        if (lpoObserver->moEventQueue.Pop(loMessage, loTimeout) != 0)
        {
            continue;
        }

        //handle event
        lpoObserver->HandleEvent(loMessage.GetClass(), loMessage.GetProvider(), loMessage.GetEvent() );
    }

    return APL_NULL;
}

AIBC_GFQ_NAMESPACE_END

