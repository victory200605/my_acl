
#ifndef AIBC_GFQ_QUEUEPOLLIMPL_H
#define AIBC_GFQ_QUEUEPOLLIMPL_H

#include "gfq2/Utility.h"
#include "gfq2/GFQueueClient.h"
#include "acl/BitArray.h"
#include "acl/Synch.h"
#include "acl/IndexDict.h"
#include "acl/stl/map.h"
#include "acl/stl/vector.h"

AIBC_GFQ_NAMESPACE_START

/** 
 * @brief Poll result cache to reduce repeat request times
 */
class CPollResultCache
{
public:
    CPollResultCache(void);
    
    ~CPollResultCache(void);
    
    void SetTimedout( acl::CTimeValue const& aoTimedout );
    
    acl::CIndexDict* GetResult(void);
    
    void Update( acl::CIndexDict* apoResult );

private:
    acl::CIndexDict* mpoResult;
        
    acl::CTimeValue moTimedout;
    
    acl::CTimestamp moTimestamp;
};

/** 
 * @brief Queue pollset impl
 */
class CQueuePollImpl
{
public:
    CQueuePollImpl( CGFQueueClientImpl* apoClient );

    ~CQueuePollImpl(void);

    void SetResultCacheTime( acl::CTimeValue const& aoTimedout );

    void Add( char const* apcQueueName );

    void Remove( char const* apcQueueName );

    apl_ssize_t Poll( acl::CTimeValue const& aoTimedout, std::vector<std::string>* apoResult );

protected:
    void GetAllQueue( std::vector<std::string>& aoResult );
    
    void Reset(void);
    
    apl_int_t UpdateQueueIDSet(void);

private:
    CGFQueueClientImpl* mpoClient;

    apl_time_t mi64Timestamp;

    CPollResultCache moResultCache;

    std::vector<std::string> moAddQueue;
    
    std::vector<std::string> moRemoveQueue;

    std::map<std::string, apl_size_t> moNameMap;

    std::map<apl_size_t, std::string> moIDMap;

    acl::CBitArray moQueueIDSet;

    acl::CLock moQueueLock;
    
    acl::CLock moPollLock;
};

AIBC_GFQ_NAMESPACE_END

#endif

