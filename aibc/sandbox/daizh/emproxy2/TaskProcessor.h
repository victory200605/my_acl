#ifndef AIMC_EMPROXY_POPTASKMANAGER_H
#define AIMC_EMPROXY_POPTASKMANAGER_H

#include "acl/ThreadTask.h"
#include "acl/stl/map.h"
#include "acl/stl/string.h"

class CTaskProcessor
{
public:
    CTaskProcessor(void);

    ~CTaskProcessor(void);
    
    apl_int_t AddProcessor( char const* apcHostName, apl_size_t auQueueSize, apl_size_t auThreadCount );

    apl_int_t SetDefaultProcessor( apl_size_t auQueueSize, apl_size_t auThreadCount );

    template<class T>
    apl_int_t Run( char const* apcHostName, T& atTask )
    {
        std::map<std::string, acl::CThreadTask*>::iterator loIter = this->moProcessores.find(apcHostName);
        if (loIter == this->moProcessores.end() )
        {
            return this->mpoDefault->Run(atTask, acl::CTimeValue::ZERO);
        }
        else
        {
            acl::CThreadTask* lpoThreadTask = loIter->second;

            return lpoThreadTask->Run(atTask, acl::CTimeValue::ZERO);
        }
    }

    apl_size_t GetWaitingSize( char const* apcHostName );

private:
    acl::CThreadTask* mpoDefault;
    std::map<std::string, acl::CThreadTask*> moProcessores;
};

#endif//AIMC_EMPROXY_POPTASKMANAGER_H

