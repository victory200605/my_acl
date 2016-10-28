#ifndef __AILIBEX__AIMEMPOOL_H__
#define __AILIBEX__AIMEMPOOL_H__

#include "AILib.h"

///start namespace
AIBC_NAMESPACE_START

typedef struct stAIMemPool* AIMemPool_t;


AIMemPool_t AIMemPoolCreate(
    size_t aiPoolSize,
    size_t aiBlockSize
);


void AIMemPoolReset(
    AIMemPool_t aptMemPool
);


void AIMemPoolDestroy(
    AIMemPool_t aptMemPool
);



void* AIMemPoolAlloc(
    AIMemPool_t aptMemPool
);


void AIMemPoolFree(
    AIMemPool_t aptMemPool,
    void* apMemNode
);



size_t AIMemPoolGetPoolSize(
    AIMemPool_t aptMemPool
);

size_t AIMemPoolGetBlockSize(
    AIMemPool_t aptMemPool
);

size_t AIMemPoolGetPeakSize(
    AIMemPool_t aptMemPool
);

///end namespace
AIBC_NAMESPACE_END

#endif //__AILIBEX__AIMEMPOOL_H__

