#include <pthread.h>

#include "AILib.h"
#include "AILogSys.h"
#include "AIDualLink.h"
#include "AISynch.h"
#include "AITime.h"

#include "AIThrPool.h"

///start namespace
AIBC_NAMESPACE_START

AITime_t const AI_THRPOOL_WAIT_DELTA = AI_TIME_MSEC * 100;

////////////////////////////////////////////////////////////////////////////
struct stThrTask
{
    LISTHEAD        coListNode;
    AIThrFunc_t     cpfFunc;
    void*           cpArg; 
    char            csName[32];
    AISemaphore     coSema;
};


////////////////////////////////////////////////////////////////////////////
class clsIdleQueue
{
public:
    clsIdleQueue()
        :ciSize(0)
    {
        ai_init_list_head(&coList);
    }

    ~clsIdleQueue()
    {
        assert(ai_list_is_empty(&coList));
    }

    void Put(stThrTask* apoTask)
    {
        AISmartLock loSmartLock(coLock);

        ai_list_add_tail(&apoTask->coListNode, &coList); 
        ++ciSize;
    }

    stThrTask* Get()
    {
        AISmartLock loSmartLock(coLock);

        if (ai_list_is_empty(&coList))
        {
            return NULL;
        }

        stThrTask* lpoTask = AI_GET_STRUCT_PTR(coList.cpNext, stThrTask, coListNode);
        ai_list_del_any(&lpoTask->coListNode);
        --ciSize;

        return lpoTask;
    }

    size_t GetSize()
    {
        return ciSize;
    }
    
private:
    size_t      ciSize;
    LISTHEAD    coList;
    AIMutexLock coLock;
};


////////////////////////////////////////////////////////////////////////////
struct stAIThrPool
{
    size_t          ciTotalThrs;
    clsIdleQueue    coIdleQueue;
    AISemaphore     coSema;
};


////////////////////////////////////////////////////////////////////////////
static void* _ThrPoolInternalFunc(void * apArg)
{
    AIThrPool_t lpoThrPool = (AIThrPool_t)apArg;
    stThrTask*  lpoTask;

    pthread_detach(pthread_self());

    AI_NEW_ASSERT(lpoTask, stThrTask);

    lpoThrPool->coIdleQueue.Put(lpoTask);
    lpoThrPool->coSema.Post();

    while (true)
    {
        lpoTask->coSema.Wait();
         
        if (NULL == lpoTask->cpfFunc)
        {
            break; // oh, I quit!
        }

        lpoTask->cpfFunc(lpoTask->cpArg);

        lpoThrPool->coIdleQueue.Put(lpoTask);
    }

    lpoThrPool->coSema.Post();
    AI_DELETE(lpoTask);
    return NULL;
}


////////////////////////////////////////////////////////////////////////////
AIThrPool_t AIThrPoolCreate(size_t aiTotalThrs)
{
    AIThrPool_t lpoThrPool;
    AI_NEW_ASSERT(lpoThrPool, struct stAIThrPool);

    lpoThrPool->ciTotalThrs = 0;
   
    for (size_t n = 0; n < aiTotalThrs; ++n)
    {
        pthread_t   tid;

        if (0 != pthread_create(&tid, NULL, &_ThrPoolInternalFunc, lpoThrPool))
        {   // cleanup!
            AIThrPoolDestroy(lpoThrPool);
            return NULL;
        }

        lpoThrPool->coSema.Wait();
        lpoThrPool->ciTotalThrs++;
    }

    return lpoThrPool;
}


////////////////////////////////////////////////////////////////////////////
void AIThrPoolDestroy(AIThrPool_t apoThrPool)
{
    stThrTask*  lpoTask;
    assert(NULL != apoThrPool);

    for (size_t i = 0; i < apoThrPool->ciTotalThrs; ++i)
    {
        while ((lpoTask = apoThrPool->coIdleQueue.Get()) == NULL)
        {
            AISleepFor(AI_THRPOOL_WAIT_DELTA);
        }

        lpoTask->cpfFunc = NULL; // notify thread to quit
        lpoTask->coSema.Post();

        apoThrPool->coSema.Wait();
    }

    AI_DELETE(apoThrPool);
}


////////////////////////////////////////////////////////////////////////////
int AIThrPoolCall(AIThrPool_t apoThrPool, AIThrFunc_t apfFunc, void* apArg, char const* apcName, AITime_t aiTimeout)
{
    assert(NULL != apoThrPool);
    assert(NULL != apfFunc);

    stThrTask*  lpoTask = apoThrPool->coIdleQueue.Get();

    if (NULL == lpoTask)
    {
        return AI_THRPOOL_ERROR_FULL;
    }

    lpoTask->cpfFunc = apfFunc;
    lpoTask->cpArg = apArg;
    lpoTask->coSema.Post();

    return 0;
}


////////////////////////////////////////////////////////////////////////////
ssize_t AIThrPoolGetCount(AIThrPool_t aptThrPool, char const* apcName)
{
    assert(aptThrPool);

    return aptThrPool->ciTotalThrs - aptThrPool->coIdleQueue.GetSize();
}


////////////////////////////////////////////////////////////////////////////
ssize_t AIThrPoolGetFreeCount(AIThrPool_t aptThrPool)
{
    assert(aptThrPool);

    return aptThrPool->coIdleQueue.GetSize();
}

///end namespace
AIBC_NAMESPACE_END
