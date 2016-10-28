#ifndef __AILIBEX__AITHRPOOL_H__
#define __AILIBEX__AITHRPOOL_H__

#include "AILib.h"
#include "AITime.h"

///start namespace
AIBC_NAMESPACE_START

enum
{
    AI_THRPOOL_ERROR_FULL     = -1,
    AI_THRPOOL_ERROR_TIMEOUT  = -2
};


typedef struct stAIThrPool* AIThrPool_t;


typedef void* (*AIThrFunc_t)(void* apArg);


#define AI_THR_PARAM_T void**
#define AI_THR_PARAM_NEW(param, n)              AI_NEW_N_ASSERT(param, void*, n);
#define AI_THR_PARAM_PUSH(param, idx, value)    param[idx] = (void*)value;
#define AI_THR_PARAM_POP(param, idx, type)      ((type*)param[idx]); 
#define AI_THR_PARAM_DELETE(param)              AI_DELETE_N(param);



AIThrPool_t AIThrPoolCreate(
    size_t      aiTotalThr
);


void AIThrPoolDestroy(
    AIThrPool_t aptThrPool
);


int AIThrPoolCall(
    AIThrPool_t apThrPool,
    AIThrFunc_t apfFunc,
    void*       apArg,
    char const* apcName = NULL,
    AITime_t    aiTimeout = 0
);


bool AIThrPoolIsRunning(
    AIThrPool_t aptThrPool
);


ssize_t AIThrPoolGetCount(
    AIThrPool_t aptThrPool,
    char const* apcName = NULL
);


ssize_t AIThrPoolGetFreeCount(
    AIThrPool_t aptThrPool
);

///end namespace
AIBC_NAMESPACE_END

#endif // __AILIBEX__AITHRPOOL_H__

