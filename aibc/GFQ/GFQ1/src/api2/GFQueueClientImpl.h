
#ifndef AIBC_GFQUEUECLIENTIMPL_H
#define AIBC_GFQUEUECLIENTIMPL_H

#include "anf/util/AsyncClient.h"
#include "anf/protocol/SimpleAsyncMessage.h"
#include "anf/filter/SimpleAsyncMessageFilter.h"
#include "acl/TimeValue.h"
#include "acl/Synch.h"
#include "acl/MemoryBlock.h"
#include "acl/Timestamp.h"
#include "acl/stl/string.h"
#include "acl/stl/map.h"
#include "gfq/GFQUtility.h"
#include "gfq/GFQProtocol.h"

AIBC_GFQ_NAMESPACE_START

typedef struct AsyncClient : public anf::TAsyncClient<anf::CSimpleAsyncMessage> 
{
    acl::CSockAddr moRemoteAddress;
} ClientType;

// Error cache for high frequency and repeat error
struct CErrorCache
{
    CErrorCache(void);
    bool IsHitCache(void);
    
    apl_int_t  miErrNo;
    apl_int_t  miTimeInterval;
    acl::CTimestamp moTimestamp;
};

// segment server cache node, will be saved in hashtable
// read & write segment info cache node, will be saved in hashtable
struct CSegmentInfo
{
    CSegmentInfo();
    
	apl_int_t        miSegmentID;
	apl_int_t        miModuleID;
	apl_int64_t      miQueueGlobalID;
	char             macQueueName[MAX_NAME_LEN];
	ClientType*      mpoSegmentServer;
	CErrorCache      moErrorCache;
};

// Global File Queue Api interface defined
class CGFQueueClientImpl
{
    struct CSegmentInfoMap
    {
        acl::CLock moLock;
        std::map<std::string, CSegmentInfo> moMap;
    };

public:
    enum { DEAD_CIRCLE_NUM = 1001 }; // 
    
public:
	CGFQueueClientImpl(void);

	~CGFQueueClientImpl(void);
	
    // Initialize api
	apl_int_t Initialize(
        char const* apcName,
        acl::CSockAddr const& aoRemoteAddress, 
        apl_size_t auConnNum, 
        acl::CTimeValue const& aoTimeout,
        apl_size_t auWindSize = 1000,
        bool abIsIgnoreConnectFail = false );
    
    void Close(void);

    void SetErrorCacheInterval( apl_int_t aiInterval );

    apl_int_t GetErrorCacheInterval(void);
	
	// Get data
	apl_int_t Get( const char* apcQueueName, acl::CMemoryBlock* apoData, apl_int_t aiWhenSec = 0 );
	
	// Put data
	apl_int_t Put( const char* apcQueueName, acl::CMemoryBlock const& aoData );
	
	// Get queue status
	apl_int_t GetStat( const char* apcQueueName, apl_int_t* apiSize, apl_int_t* apiCapaciy );
	
	// Get module(segmentserver) count
	apl_int_t GetModuleCount( apl_int_t* apiCount );
     
    // Notify Master reload queue info
    apl_int_t ReloadQueueInfo(void);
    
    // Notify Master print all queue
    apl_int_t PrintQueues(void);
    
    // Notify Master print all module
    apl_int_t PrintModules(void);
    
protected:
	apl_int_t GetWriteSegment( const char* apcQueueName, CSegmentInfo& aoSegmentInfo );
	apl_int_t GetReadSegment( const char* apcQueueName, CSegmentInfo& aoSegmentInfo );
	
	apl_int_t GetWriteSegmentSecond( const char* apcQueueName, apl_int_t aiRetcode, CSegmentInfo& aoSegmentInfo );
	apl_int_t GetReadSegmentSecond( const char* apcQueueName, apl_int_t aiRetcode, CSegmentInfo& aoSegmentInfo );
	
	apl_int_t SetSegmentInfo( CSegmentInfoMap& aoSegmentInfoMap, const char* apcQueueName, CSegmentInfo& apoSegmentInfo );
	apl_int_t GetSegmentInfo( CSegmentInfoMap& aoSegmentInfoMap, const char* apcQueueName, CSegmentInfo& aoSegmentInfo );
	apl_int_t CreateSegmentServerConnection( const char* apcServerIp, apl_int_t aiPort, ClientType** appoSegmentServer );
	
protected:
	apl_size_t muConnNum;
	apl_size_t muWindSize;
    acl::CTimeValue moTimeout;

    char macUserName[MAX_NAME_LEN];
	
	ClientType moMaster;
    
    acl::CLock moLock;
    std::map<std::string, ClientType*> moServers;

    CSegmentInfoMap moReadableSegments;

    CSegmentInfoMap moWriteableSegments;

	// Error cache for high frequency and repeat error
	apl_int_t miErrorCacheInterval;
};

AIBC_GFQ_NAMESPACE_END

#endif //AIBC_GFQUEUECLIENT_H
