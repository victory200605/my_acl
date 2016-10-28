#include "AIMemPool.h"
#include "AIDualLink.h"

///start namespace
AIBC_NAMESPACE_START

////////////////////////////////////////////////////////////////////////////
struct stAIMemPool
{
    size_t      ciPoolSize;
    size_t      ciBlockSize;
    size_t      ciPeakSize;
    size_t      ciAllocSize;
    LISTHEAD    ctFreeList;
    LISTHEAD*   cptNodes;
    char*       cpcMemPtr;
};

////////////////////////////////////////////////////////////////////////////
#define MIN_MEM_SIZE ((size_t)8)

////////////////////////////////////////////////////////////////////////////
static inline size_t _AlignMemSize(size_t aiSize)
{
    return (((aiSize / MIN_MEM_SIZE) + ((aiSize % MIN_MEM_SIZE) ? 1 : 0)) * MIN_MEM_SIZE);
}

////////////////////////////////////////////////////////////////////////////
static inline size_t _GetNodeIndex(AIMemPool_t aptMemPool, LISTHEAD* lptNode)
{
    assert(lptNode >= aptMemPool->cptNodes);
    assert(lptNode < (aptMemPool->cptNodes + aptMemPool->ciPoolSize));

    return lptNode - aptMemPool->cptNodes;
}

////////////////////////////////////////////////////////////////////////////
static inline size_t _GetDataIndex(AIMemPool_t aptMemPool, void* lpMem)
{
    ptrdiff_t liLen = ((char*)lpMem) - aptMemPool->cpcMemPtr;

    assert(liLen >= 0);
    assert(liLen < (ptrdiff_t)(aptMemPool->ciPoolSize * aptMemPool->ciBlockSize));
    assert((liLen % aptMemPool->ciBlockSize) == 0);

    return (((char*)lpMem) - aptMemPool->cpcMemPtr) / aptMemPool->ciBlockSize;
}

////////////////////////////////////////////////////////////////////////////
static inline char* _GetDataPtr(AIMemPool_t aptMemPool, size_t aiIndex)
{
    return aptMemPool->cpcMemPtr + aptMemPool->ciBlockSize * aiIndex;
}


////////////////////////////////////////////////////////////////////////////
AIMemPool_t AIMemPoolCreate(size_t aiPoolSize, size_t aiBlockSize)
{
    AIMemPool_t lptMP;

    AI_MALLOC(lptMP, struct stAIMemPool, sizeof(struct stAIMemPool));

    lptMP->ciPoolSize = aiPoolSize;
    lptMP->ciBlockSize = _AlignMemSize(aiBlockSize);

    AI_MALLOC(lptMP->cptNodes, LISTHEAD, sizeof(LISTHEAD) * lptMP->ciPoolSize);
    AI_MALLOC(lptMP->cpcMemPtr, char, lptMP->ciBlockSize * lptMP->ciPoolSize);

    AIMemPoolReset(lptMP);

    return lptMP;
}


////////////////////////////////////////////////////////////////////////////
void AIMemPoolReset(AIMemPool_t aptMemPool)
{
    aptMemPool->ciPeakSize   = 0;
    aptMemPool->ciAllocSize  = 0;

    ai_init_list_head(&(aptMemPool->ctFreeList));
    
    for (size_t n = 0; n < aptMemPool->ciPoolSize; ++n)
    {
        ai_list_add_tail(&(aptMemPool->cptNodes[n]), &(aptMemPool->ctFreeList));
    }
}

////////////////////////////////////////////////////////////////////////////
void AIMemPoolDestroy(AIMemPool_t aptMemPool)
{
    AI_FREE(aptMemPool->cpcMemPtr);
    AI_FREE(aptMemPool->cptNodes);
    AI_FREE(aptMemPool);
}


////////////////////////////////////////////////////////////////////////////
void* AIMemPoolAlloc(AIMemPool_t aptMemPool)
{
    LISTHEAD* lptNode;

    AI_RETURN_IF(NULL, (aptMemPool->ciAllocSize >= aptMemPool->ciPoolSize));

    ai_list_del_head(lptNode, &(aptMemPool->ctFreeList));

    aptMemPool->ciAllocSize++;
    if (aptMemPool->ciPeakSize < aptMemPool->ciAllocSize)
    {
        aptMemPool->ciPeakSize = aptMemPool->ciAllocSize;
    }

    return _GetDataPtr(aptMemPool, _GetNodeIndex(aptMemPool, lptNode));
}

////////////////////////////////////////////////////////////////////////////
void AIMemPoolFree(AIMemPool_t aptMemPool, void* apMem)
{
    LISTHEAD* lptNode = &(aptMemPool->cptNodes[_GetDataIndex(aptMemPool, apMem)]);

    ai_list_add_head(lptNode, &(aptMemPool->ctFreeList));

    aptMemPool->ciAllocSize--;
}

////////////////////////////////////////////////////////////////////////////
size_t AIMemPoolGetPoolSize(AIMemPool_t aptMemPool)
{
    return aptMemPool->ciPoolSize;
}

////////////////////////////////////////////////////////////////////////////
size_t AIMemPoolGetBlockSize(AIMemPool_t aptMemPool)
{
    return aptMemPool->ciBlockSize;
}

////////////////////////////////////////////////////////////////////////////
size_t AIMemPoolGetPeakSize(AIMemPool_t aptMemPool)
{
    return aptMemPool->ciPeakSize;
}

///end namespace
AIBC_NAMESPACE_END
