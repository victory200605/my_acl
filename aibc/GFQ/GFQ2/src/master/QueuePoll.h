
#ifndef AIBC_GFQ_QUEUE_POLL_H
#define AIBC_GFQ_QUEUE_POLL_H

#include "gfq2/Utility.h"
#include "acl/stl/string.h"
#include "acl/stl/map.h"
#include "acl/stl/vector.h"
#include "acl/Timestamp.h"
#include "acl/MsgQueue.h"
#include "acl/Synch.h"
#include "acl/ThreadManager.h"
#include "anf/IoSession.h"
#include "acl/BitArray.h"

AIBC_GFQ_NAMESPACE_START

typedef void (*PollHandlerFuncType)(anf::SessionPtrType& aoSession, apl_size_t auSequenceID, char const* apcResult, apl_size_t auSize);

/** 
 * @brief Implement queue event poll mechanism
 */
class CQueuePoll
{
    struct CPollRequest
    {
        CPollRequest(
            anf::SessionPtrType& aoSession,
            apl_size_t auSequenceID,
            char const* apcBitSet,
            apl_size_t auLength,
            apl_int_t aiTimeout,
            PollHandlerFuncType afHandler )
            : moSession(aoSession)
            , muSequenceID(auSequenceID)
            , moPollBitSet(apcBitSet, auLength, acl::CBitArray::DO_COPY)
            , moTimeout(aiTimeout <= 0 ? 5 : aiTimeout)
            , mfHandler(afHandler)
        {
            this->moTimestamp.Update();
        }

        anf::SessionPtrType moSession;
        apl_size_t          muSequenceID;
        acl::CBitArray      moPollBitSet;
        acl::CTimeValue     moTimeout;
        PollHandlerFuncType mfHandler;
        acl::CTimestamp     moTimestamp;
    };

public:
    typedef acl::TMsgQueue<CPollRequest*> RequestListType;

public:
    CQueuePoll(void);

    ~CQueuePoll(void);

    apl_int_t Initialize(void);

    void Close(void);

    apl_int_t SchedulePoll(
        anf::SessionPtrType& aoSession, 
        apl_size_t auSequenceID,
        char const* apcBitSet,
        apl_size_t auLength,
        apl_int_t  aiTimeout,
        PollHandlerFuncType afHandler );

    void Signal( char const* apcProvider );

protected:
    bool CheckPollSet( acl::CBitArray& aoPollSet, acl::CBitArray& aoNoEmpty, acl::CBitArray& aoResult );

    void UpCall( CPollRequest* apoRequest, acl::CBitArray& aoResult );

    static void* Srv( void* apvParam );

private:
    bool mbIsClosed;

    anf::SessionPtrType moSession;

    RequestListType moRequests;

    acl::CThreadManager moThreadManager;
};

AIBC_GFQ_NAMESPACE_END

#endif//AIBC_GFQ_QUEUE_POLL_H

