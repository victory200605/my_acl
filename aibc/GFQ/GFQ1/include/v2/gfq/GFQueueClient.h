
#ifndef AIBC_GFQUEUECLIENT_H
#define AIBC_GFQUEUECLIENT_H

#include "GFQUtility.h"

namespace acl
{
    class CSockAddr;
    class CTimeValue;
    class CMemoryBlock;
};

AIBC_GFQ_NAMESPACE_START

// Global File Queue Api interface defined
class CGFQueueClient
{
public:
	CGFQueueClient(void);

	~CGFQueueClient(void);
	
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
	apl_int_t GetStat( const char* apcQueueName, apl_int_t* apiSize, apl_int_t* apiCapacity );
	
	// Get module(segmentserver) count
	apl_int_t GetModuleCount( apl_int_t* apiCount );
     
    // Notify Master reload queue info
    apl_int_t ReloadQueueInfo(void);
    
    // Notify Master print all queue
    apl_int_t PrintQueues(void);
    
    // Notify Master print all module
    apl_int_t PrintModules(void);
   
private:
    class CGFQueueClientImpl* mpoImpl;
};

AIBC_GFQ_NAMESPACE_END

#endif //AIBC_GFQUEUECLIENT_H
