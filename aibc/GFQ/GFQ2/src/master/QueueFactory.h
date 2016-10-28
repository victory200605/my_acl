
#ifndef AIBC_GFQ_QUEUEFACTORY_H
#define AIBC_GFQ_QUEUEFACTORY_H

#include "gfq2/Utility.h"
#include "Queue.h"

AIBC_GFQ_NAMESPACE_START

class CQueueFactory
{
public:
    virtual ~CQueueFactory(void){}

    virtual IQueue* CreateQueue( char const* apcName, apl_size_t auQueueID );

    virtual void DestroyQueue( IQueue* apoQueue );

private:
    std::string moType;
};

AIBC_GFQ_NAMESPACE_END

#endif//AIBC_GFQ_QUEUEFACTORY_H

