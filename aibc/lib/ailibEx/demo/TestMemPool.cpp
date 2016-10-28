#include "AILib.h"
#include "AIMemPool.h"


#define ARR_SIZE ((size_t)256)

int Test001()
{
    void* lvpN[ARR_SIZE];
    AIMemPool_t lptMP = AIMemPoolCreate(ARR_SIZE, 1024);
    AIMemPoolDestroy(lptMP);

    lptMP = AIMemPoolCreate(ARR_SIZE, 1024);

    memset(lvpN, 0, sizeof(lvpN));      
    for (size_t n = 0; n < ARR_SIZE; ++n)
    {
        lvpN[n] = AIMemPoolAlloc(lptMP);
        write(STDOUT_FILENO, "1", 1);
    }
    write(STDOUT_FILENO, "\n", 1);

    AIMemPoolReset(lptMP);

    memset(lvpN, 0, sizeof(lvpN));      
    for (size_t n = 0; n < ARR_SIZE; ++n)
    {
        lvpN[n] = AIMemPoolAlloc(lptMP);
        write(STDOUT_FILENO, "2", 1);
    }
    write(STDOUT_FILENO, "\n", 1);
     
    for (size_t n = 0; n < ARR_SIZE; ++n)
    {
        AIMemPoolFree(lptMP, lvpN[n]);
        write(STDOUT_FILENO, "3", 1);
    }
    write(STDOUT_FILENO, "\n", 1);

    memset(lvpN, 0, sizeof(lvpN));      
    for (size_t n = 0; n < ARR_SIZE; ++n)
    {
        lvpN[n] = AIMemPoolAlloc(lptMP);
        write(STDOUT_FILENO, "4", 1);
    }
    write(STDOUT_FILENO, "\n", 1);
     
    printf("the next statement would give you a coredump!\n");
    AIMemPoolAlloc(lptMP);

    return 0;
}

int main()
{
    Test001();
}

