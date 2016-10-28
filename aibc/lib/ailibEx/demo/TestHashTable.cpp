#include "AIHashTable.h"

#define TOTAL_NODES 256
#define BUCKETS     16
#define KEY_SIZE    8
#define VAL_SIZE    16


int Test001()
{
    AIHashTable_t lptHashTable;
    int liRetCode;
    char lsBuf[KEY_SIZE+1];
    char lsOut[VAL_SIZE+1];
    size_t liValLen = VAL_SIZE;
 
    lptHashTable = AIHashTableCreate(KEY_SIZE, VAL_SIZE, TOTAL_NODES, BUCKETS);

    for(size_t i = 0; i < TOTAL_NODES; ++i)
    {
        snprintf(lsBuf, sizeof(lsBuf), "%d_0123456789", i);
        liRetCode = AIHashTablePut(lptHashTable, lsBuf, KEY_SIZE, lsBuf, VAL_SIZE); 
        assert(liRetCode == 0);
    }

    liRetCode = AIHashTablePut(lptHashTable, "", 1, "", 1); 
    assert(liRetCode == AI_HASH_ERROR_FULL);

    for(size_t i = 0; i < TOTAL_NODES; ++i)
    {
        snprintf(lsBuf, sizeof(lsBuf), "%d_0123456789", i);

        liRetCode = AIHashTableGet(lptHashTable, lsBuf, KEY_SIZE, lsOut, &liValLen); 
        assert(liRetCode == 0);
        assert(liValLen == VAL_SIZE);
    }

    liValLen = VAL_SIZE;
    liRetCode = AIHashTablePop(lptHashTable, lsBuf, KEY_SIZE, lsOut, &liValLen); 
    assert(liRetCode == 0);
    assert(liValLen == VAL_SIZE);

    liRetCode = AIHashTableSet(lptHashTable, "", 1, "", 1); 
    assert(liRetCode == 0);

    AIHashTableDestroy(lptHashTable);

    return 0;
}

int main()
{
    Test001();
}

