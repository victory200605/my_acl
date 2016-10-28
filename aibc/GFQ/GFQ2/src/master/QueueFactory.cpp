
#include "MasterConfig.h"
#include "QueueFactory.h"
#include "FIFOQueue.h"

AIBC_GFQ_NAMESPACE_START

IQueue* CQueueFactory::CreateQueue( char const* apcName, apl_size_t auQueueID )
{
    CMasterConfig::CQueueInfo loQueueInfo;
    IQueue* lpoQueue = APL_NULL;

    if (!acl::Instance<CMasterConfig>()->FindQueueInfo(apcName, loQueueInfo) )
    {
        return APL_NULL;
    }
    
    if (apl_strcasecmp(loQueueInfo.moType.c_str(), "FIFO") == 0)
    {
        ACL_NEW_ASSERT(lpoQueue,
            CFIFOQueue(
                apcName, 
                auQueueID, 
                loQueueInfo.muCapacity,
                loQueueInfo.moBindNodeGroup.c_str() ) );
    }
    else
    {
        ACL_NEW_ASSERT(lpoQueue,
            CFIFOQueue(
                apcName, 
                auQueueID, 
                loQueueInfo.muCapacity,
                loQueueInfo.moBindNodeGroup.c_str() ) );
    }

    return lpoQueue;
}

void CQueueFactory::DestroyQueue( IQueue* apoQueue )
{
    ACL_DELETE(apoQueue);
}

AIBC_GFQ_NAMESPACE_END

