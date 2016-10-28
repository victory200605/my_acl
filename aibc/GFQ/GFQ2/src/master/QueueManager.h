
#ifndef AIBC_GFQ_QUEUEMANAGER_H
#define AIBC_GFQ_QUEUEMANAGER_H

#include "gfq2/Utility.h"
#include "acl/stl/map.h"
#include "acl/stl/string.h"
#include "acl/BitArray.h"
#include "acl/EventHandler.h"
#include "acl/TimerQueueAdapter.h"
#include "Queue.h"
#include "QueueFactory.h"

AIBC_GFQ_NAMESPACE_START

/** 
 * @brief Manage all queue resource, include creating and destroying queue
 */
class CQueueManager : public acl::IEventHandler
{
    typedef std::map<std::string, IQueue*> QueueMapType;
    typedef std::vector<IQueue*> QueueListType;

public:
    CQueueManager( CQueueFactory* apoQueueFactory = APL_NULL );

    ~CQueueManager(void);

    apl_int_t InsertSegment(
        char const* apcName, 
        CSegmentHandle aoHandle,
        apl_size_t auSegmentSize,
        apl_int64_t ai64Timestamp );
    
    apl_int_t UpdateSegment(
        char const* apcName, 
        CSegmentHandle aoHandle,
        apl_size_t auSegmentSize,
        apl_int64_t ai64Timestamp );

    apl_int_t DeallocateSegmentFinished(
        char const* apcName,
        CSegmentHandle aoHandle,
        apl_int_t aiStatus );
    
    apl_int_t AllocateSegmentFinished(
        char const* apcName,
        CSegmentHandle aoHandle,
        apl_int_t aiStatus );
    
    apl_int_t GetReadableSegment(
        char const* apcName,
        CSegmentHandle aoOldHandle,
        apl_int_t aiOldSegmentStatus,
        CSegmentInfo& aoSegmentInfo );
    
    apl_int_t GetWriteableSegment(
        char const* apcName, 
        CSegmentHandle aoOldHandle,
        apl_int_t aiOldSegmentStatus,
        CSegmentInfo& aoSegmentInfo );

    apl_int_t UpdateConfiguration(void);

    apl_int_t ClearQueueInvalidSegment( apl_size_t auServerNodeID );
    
    apl_int_t ClearQueue( char const* apcName );

    apl_size_t GetQueueList( std::vector<std::string>& aoResult, apl_size_t auOffset, apl_size_t auLimit );
    
    apl_size_t GetQueueList( acl::CBitArray& aoResult );
    
    apl_size_t GetNonEmptyQueueList( acl::CBitArray& aoResult );

    apl_int_t GetQueueInfo( char const* apcName, CQueueInfo& aoQueueInfo );
    
    apl_int_t GetQueueInfo( apl_size_t auQueueID, CQueueInfo& aoQueueInfo );
    
    apl_int_t BindNodeGroup( char const* apcQueueName, char const* apcGroupName );

    apl_int_t CreateQueue( char const* apcQueueName, apl_int_t aiOpt, CQueueInfo* apoQueueInfo );

protected:
    apl_int_t CreateQueue0( char const* apcName, IQueue** appoQueue );

    apl_int_t DestroyQueue0( char const* apcName );
    
    apl_int_t GetQueueInfo( IQueue* apoQueue, CQueueInfo& aoQueueInfo );

    //Timer handle method
    apl_int_t HandleTimeout( acl::CTimestamp const& aoCurrTime, void const* apcAct );

private:
    apl_size_t muQueueID;

    apl_int_t miTimerID;

    QueueMapType  moQueueMap;
    QueueListType moQueueList;

    CQueueFactory* mpoQueueFactory;

    acl::CRWLock moLock;
};

//singleton queue manager
CQueueManager* GetQueueManager(void);

AIBC_GFQ_NAMESPACE_END

#endif//GFQ_QUEUEMANAGER_H

