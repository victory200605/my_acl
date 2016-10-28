
#include "TaskProcessor.h"

CTaskProcessor::CTaskProcessor(void)
    : mpoDefault(APL_NULL)
{
}

CTaskProcessor::~CTaskProcessor(void)
{
    this->mpoDefault->Close();
    ACL_DELETE(this->mpoDefault);

    for (std::map<std::string, acl::CThreadTask*>::iterator loIter = this->moProcessores.begin();
        loIter != this->moProcessores.end(); ++loIter)
    {
        acl::CThreadTask* lpoThreadTask = loIter->second;

        lpoThreadTask->Close();
        ACL_DELETE(lpoThreadTask);
    }
}
    
apl_int_t CTaskProcessor::AddProcessor( char const* apcHostName, apl_size_t auQueueSize, apl_size_t auThreadCount )
{
    acl::CThreadTask*& lpoThreadTask = this->moProcessores[apcHostName];
    if (lpoThreadTask == APL_NULL)
    {
        ACL_NEW_ASSERT(lpoThreadTask, acl::CThreadTask);

        return lpoThreadTask->Initialize(auQueueSize, auThreadCount);
    }
    else
    {
        ACL_ASSERT(false);//dup processor
        return -1;
    }
}

apl_int_t CTaskProcessor::SetDefaultProcessor( apl_size_t auQueueSize, apl_size_t auThreadCount )
{
    if (this->mpoDefault == APL_NULL)
    {
        ACL_NEW_ASSERT(this->mpoDefault, acl::CThreadTask);

        return this->mpoDefault->Initialize(auQueueSize, auThreadCount);
    }
    else
    {
        ACL_ASSERT(false);//dup processor
        return -1;
    }
}

apl_size_t CTaskProcessor::GetWaitingSize( char const* apcHostName )
{
    std::map<std::string, acl::CThreadTask*>::iterator loIter = this->moProcessores.find(apcHostName);
    if (loIter == this->moProcessores.end() )
    {
        return this->mpoDefault->GetWaitCount();
    }
    else
    {
        acl::CThreadTask* lpoThreadTask = loIter->second;

        return lpoThreadTask->GetWaitCount();
    }
}

