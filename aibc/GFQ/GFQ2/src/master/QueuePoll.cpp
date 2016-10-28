
#include "acl/ThreadManager.h"
#include "acl/stl/vector.h"
#include "QueuePoll.h"
#include "EventObserver.h"
#include "QueueManager.h"

AIBC_GFQ_NAMESPACE_START

void EventHandler( anf::SessionPtrType& aoSession, apl_int_t aiClass, char const* apcProvider, apl_int_t aiEvent )
{
    acl::Instance<CQueuePoll>()->Signal(apcProvider);
}

///////////////////////////////////////////////////////////////////////////////////////
CQueuePoll::CQueuePoll(void)
    : mbIsClosed(false)
{
    this->moSession = new anf::CIoSession;
    this->moSession->SetHandle(-1);
}

CQueuePoll::~CQueuePoll(void)
{
}

apl_int_t CQueuePoll::Initialize(void)
{
    if (acl::Instance<CEventObserver>()->Register(
        this->moSession, 
        CLASS_QUEUE_EVENT, 
        EVENT_QUEUE_NONEMPTY, 
        EventHandler) != 0)
    {
        return -1;
    }

    return this->moThreadManager.Spawn(CQueuePoll::Srv, this);
}

void CQueuePoll::Close(void)
{
    this->mbIsClosed = true;

    this->moThreadManager.WaitAll();
}

apl_int_t CQueuePoll::SchedulePoll(
    anf::SessionPtrType& aoSession, 
    apl_size_t auSequenceID,
    char const* apcBitSet,
    apl_size_t auLength,
    apl_int_t aiTimeout,
    PollHandlerFuncType afHandler )
{
    CPollRequest* lpoRequest = APL_NULL;

    ACL_NEW_ASSERT(lpoRequest, CPollRequest(
        aoSession, 
        auSequenceID,
        apcBitSet,
        auLength,
        aiTimeout,
        afHandler) );

    this->moRequests.Push(lpoRequest);

    return 0;
}
    
void CQueuePoll::Signal( char const* apcProvider )
{
    this->moRequests.Push(APL_NULL);
}

bool CQueuePoll::CheckPollSet( acl::CBitArray& aoPollSet, acl::CBitArray& aoNoEmpty, acl::CBitArray& aoResult )
{
    bool lbIsHited = false;

    aoResult.Resize(aoPollSet.GetSize() );
    aoResult.SetAll(false);

    for (apl_size_t luPos = aoPollSet.Find(0, true); 
        luPos != acl::CBitArray::INVALID_POS; luPos = aoPollSet.Find(luPos + 1, true) )
    {
        if (aoNoEmpty.GetSize() > luPos && aoNoEmpty.Get(luPos) )
        {
            aoResult.Set(luPos, true);

            lbIsHited = true;
        }
    }

    return lbIsHited;
}

void CQueuePoll::UpCall( CPollRequest* apoRequest, acl::CBitArray& aoResult )
{
    (*apoRequest->mfHandler)(
        apoRequest->moSession,
        apoRequest->muSequenceID,
        aoResult.GetCStr(),
        aoResult.GetCLength() );
}

void* CQueuePoll::Srv( void* apvParam )
{
    CQueuePoll*     lpoPoll = static_cast<CQueuePoll*>(apvParam);
    CPollRequest*   lpoRequest = APL_NULL;
    acl::CTimeValue loTimeout(1);
    acl::CBitArray  loNonEmpty;
    acl::CBitArray  loResult;
    std::vector<CPollRequest*> loWaitingRequests;
    
    while(!lpoPoll->mbIsClosed)
    {
        lpoRequest = APL_NULL;
        lpoPoll->moRequests.Pop(lpoRequest, loTimeout);

        if (acl::Instance<CQueueManager>()->GetNonEmptyQueueList(loNonEmpty) > 0)
        {
            if (lpoRequest != APL_NULL)
            {
                //New request from message queue
                if (lpoRequest->moPollBitSet.GetSize() > 0) 
                {
                    //Seted pollset and check it
                    if (lpoPoll->CheckPollSet(lpoRequest->moPollBitSet, loNonEmpty, loResult) )
                    {
                        lpoPoll->UpCall(lpoRequest, loResult);
                        ACL_DELETE(lpoRequest);
                    }
                    else
                    {
                        loWaitingRequests.push_back(lpoRequest);
                    }
                }
                else
                {
                    lpoPoll->UpCall(lpoRequest, loNonEmpty);
                    ACL_DELETE(lpoRequest);
                }
            }
            else
            {
                acl::CTimestamp loCurrTime;

                loCurrTime.Update();

                //Check all waiting requests
                for (std::vector<CPollRequest*>::iterator loIter = loWaitingRequests.begin();
                     loIter != loWaitingRequests.end(); )
                {
                    bool lbIsRelease = false;

                    lpoRequest = *loIter;
                    
                    if (loCurrTime.Sec() - lpoRequest->moTimestamp.Sec() >= lpoRequest->moTimeout.Sec() )
                    {
                        //Request timedout
                        lbIsRelease = true;
                    }
                    else
                    {
                        if (lpoRequest->moPollBitSet.GetSize() > 0) 
                        {
                            //Check the specified request's pollset
                            if (lpoPoll->CheckPollSet(lpoRequest->moPollBitSet, loNonEmpty, loResult) )
                            {
                                lpoPoll->UpCall(lpoRequest, loResult);
                                
                                lbIsRelease = true;
                            }
                        }
                        else
                        {
                            lpoPoll->UpCall(lpoRequest, loNonEmpty);

                            lbIsRelease = true;
                        }
                    }
                    
                    if (lbIsRelease)
                    {
                        loIter = loWaitingRequests.erase(loIter);
                        ACL_DELETE(lpoRequest);
                    }
                    else
                    {
                        ++loIter;
                    }
                }
            }
        }
        else
        {
            if (lpoRequest != APL_NULL)
            {
                loWaitingRequests.push_back(lpoRequest);
            }
        }
    }
    
    return APL_NULL;
}

AIBC_GFQ_NAMESPACE_END

