#ifndef __AILIBEX_AIHASHTABLE_H__
#define __AILIBEX_AIHASHTABLE_H__

#include "AILib.h"
#include "AIDualLink.h"

///start namespace
AIBC_NAMESPACE_START

int const AI_HASH_ERROR_NOTFOUND    = -1;
int const AI_HASH_ERROR_DUPKEY      = -2;
int const AI_HASH_ERROR_FULL        = -3;
int const AI_HASH_ERROR_KEYLEN      = -4;
int const AI_HASH_ERROR_VALLEN      = -5;

int const AI_HASH_TRAVERSAL_SKIPNODE    = -11;
int const AI_HASH_TRAVERSAL_SKIPLIST    = -12;
int const AI_HASH_TRAVERSAL_DELETE      = -13;
int const AI_HASH_TRAVERSAL_MODIFIED    = -14;
int const AI_HASH_TRAVERSAL_EXIT        = -15;

typedef size_t AIHashKey_t;

typedef struct stAIHashTable* AIHashTable_t;


typedef AIHashKey_t (*AIHashFunc_t)(
    AIHashTable_t aptHashTable,
    void const* apKey,
    size_t aiKeyLen
);


typedef int (*AIHashTraversalFunc_t)(
    AIHashTable_t   apHashTable,
    void const*     apKey,
    size_t          aiKeyLen,
    void*           apVal,
    size_t*         aiValLen,
    void*           apArg
);


AIHashTable_t AIHashTableCreate(
    size_t          aiKeySize,
    size_t          aiValSize,
    size_t          aiTotalNodes,
    size_t          aiBuckets = 16,
    AIHashFunc_t    apfHashFunc = NULL
);


void AIHashTableReset(
    AIHashTable_t   aptHashTable
);


void AIHashTableDestroy(
    AIHashTable_t   aptHashTable
);

size_t AIHashTableGetKeySize(
    AIHashTable_t   aptHashTable
);

size_t AIHashTableGetValSize(
    AIHashTable_t   aptHashTable
);    

size_t AIHashTableGetHashSize(
    AIHashTable_t   aptHashTable
);    

bool AIHashTableHasKey(
    AIHashTable_t   aptHashTable,
    void const*     apKey,
    size_t          aiKeyLen
);

int AIHashTablePut(
    AIHashTable_t   aptHashTable,
    void const*     apKey,
    size_t          aiKeyLen,
    void const*     apVal,
    size_t          aiValLen
);

int AIHashTableSet(
    AIHashTable_t   aptHashTable,
    void const*     apKey,
    size_t          aiKeyLen,
    void const*     apVal,
    size_t          aiValLen
);

int AIHashTableGet(
    AIHashTable_t   aptHashTable,
    void const*     apKey,
    size_t          aiKeyLen,
    void*           apVal,
    size_t*         aiValLen
);

int AIHashTablePop(
    AIHashTable_t   aptHashTable,
    void const*     apKey,
    size_t          aiKeyLen,
    void*           apVal,
    size_t*         aiValLen
);

void AIHashTableTraversal(
    AIHashTable_t           aptHashTable,
    AIHashTraversalFunc_t   apfTravFunc,
    void*                   apArg
);

///end namespace
AIBC_NAMESPACE_END

#endif // __AILIBEX__AIHASHTABLE_H__

