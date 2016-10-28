#include "AIThrPool.h"
#include "AILogSys.h"
#include "AIConfig.h"

#define TEST_ARG ((void*)121212)

AIThrPool_t lptThrPool;

void* TestThrFunc(void* apArg)
{
    printf("F: %lX, %u\n", (long)pthread_self(), (unsigned)apArg);

    return NULL;
}

int Test001(size_t aiThrs, size_t aiCalls)
{
    int liRet;
    size_t liSucc = 0;
    size_t liFail = 0;

    lptThrPool = AIThrPoolCreate(aiThrs);
    
    fprintf(stderr, "create -> %p\n", (void*)lptThrPool);

    for (size_t n = 0; n < aiCalls; ++n)
    {
        liRet = AIThrPoolCall(lptThrPool, &TestThrFunc, (void*)n, "aaa", AI_TIME_SEC*1);
        if (liRet < 0)
        {
            ++liFail;
        }
        else
        {
            ++liSucc;
        }
    }

    AIThrPoolDestroy(lptThrPool);

    fprintf(stderr, "end! %u %u\n", liSucc, liFail);
   
    return 0; 
}

int main(int argc, char** argv)
{
    assert(argc > 2);

    AIInitIniHandler();
    AIInitLOGHandler();

    Test001(strtoul(argv[1], NULL, 10), strtoul(argv[2], NULL, 10));

    AICloseLOGHandler();
    AICloseIniHandler();

    return 0;
}

DECLARE_CMDOPT_BEGIN()
DECLARE_CMDOPT_END()

