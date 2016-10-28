#include "AIHashTable.h"
#include "AIDualLink.h"

///start namespace
AIBC_NAMESPACE_START

////////////////////////////////////////////////////////////////////////////
size_t const AI_HASH_SIZE_MIN = 1;

////////////////////////////////////////////////////////////////////////////
typedef struct stAIHashNode
{
    LISTHEAD    ctBucketNode;
    void*       cpKey;
    size_t      ciKeyLen;
    void*       cpVal;
    size_t      ciValLen;
}AIHashNode_t;


////////////////////////////////////////////////////////////////////////////
struct stAIHashTable
{
    AIHashFunc_t    cpfHashFunc;
    size_t          ciBuckets; 
    size_t          ciKeySize;
    size_t          ciValSize;
    size_t          ciTotalNodes;

    LISTHEAD        ctFreeList;
    LISTHEAD*       cptBuckets;
};

////////////////////////////////////////////////////////////////////////////
#define GET_HASHNODE_PTR(p) AI_GET_STRUCT_PTR(p, AIHashNode_t, ctBucketNode)

////////////////////////////////////////////////////////////////////////////
AIHashKey_t AIHashFuncDef(AIHashTable_t aptHashTable, void const* apKey, size_t aiKeyLen)
{
    AIHashKey_t liKey = 0x55555555;
     
    for (size_t n=0; n < aiKeyLen; n++)
    {
        liKey = (liKey << 4) ^ *(((uint8_t const*)apKey) + n);
    }

    return liKey % AIHashTableGetHashSize(aptHashTable);
}

////////////////////////////////////////////////////////////////////////////
static inline size_t _FreeBucket(LISTHEAD* aptBucket)
{
    size_t	liCount = 0;
    
    while( !ai_list_is_empty(aptBucket) )
    {
        AIHashNode_t*	lptHashNode = GET_HASHNODE_PTR(aptBucket->cpNext);
        
        ai_list_del_any(&(lptHashNode->ctBucketNode));
        
        AI_FREE(lptHashNode->cpKey);
        AI_FREE(lptHashNode->cpVal);
        AI_FREE(lptHashNode);

        ++liCount;
    }
    
    return  liCount;
}

////////////////////////////////////////////////////////////////////////////
static inline AIHashNode_t* _SearchBucket(AIHashTable_t aptHashTable, AIHashKey_t liHashKey, void const* apKey, size_t aiKeyLen)
{
    LISTHEAD*	lptHashHead = &(aptHashTable->cptBuckets[liHashKey]);
 
    for (LISTHEAD* lptLH = lptHashHead->cpNext; lptLH != lptHashHead; lptLH = lptLH->cpNext)
    {
        AIHashNode_t*	lptHashNode = GET_HASHNODE_PTR(lptLH);
        
        if (aiKeyLen == lptHashNode->ciKeyLen
            && 0 == memcmp(apKey, lptHashNode->cpKey, aiKeyLen))
        {
            return lptHashNode;
        }
    }

    return NULL;
}
////////////////////////////////////////////////////////////////////////////
static inline AIHashNode_t* _AcquireHashNode(AIHashTable_t aptHashTable, AIHashKey_t aiHashKey)
{
    if (ai_list_is_empty(&(aptHashTable->ctFreeList)))
    {
        return NULL;
    }

    AIHashNode_t*	lptHashNode = GET_HASHNODE_PTR(aptHashTable->ctFreeList.cpNext);
    
    ai_list_del_any(&(lptHashNode->ctBucketNode));
    ai_list_add_tail(&(lptHashNode->ctBucketNode), &(aptHashTable->cptBuckets[aiHashKey]));

    return lptHashNode;
}

////////////////////////////////////////////////////////////////////////////
static inline void _ReleaseHashNode(AIHashTable_t aptHashTable, AIHashNode_t* aptHashNode)
{
    ai_list_del_any(&(aptHashNode->ctBucketNode));
    ai_list_add_head(&(aptHashNode->ctBucketNode), &(aptHashTable->ctFreeList));
}

////////////////////////////////////////////////////////////////////////////
AIHashTable_t AIHashTableCreate(size_t aiKeySize, size_t aiValSize, size_t aiTotalNodes, size_t aiBuckets, AIHashFunc_t apfHashFunc)
{
    struct stAIHashTable*	lptHashTable;

    AI_MALLOC(lptHashTable, struct stAIHashTable, sizeof(struct stAIHashTable));

    lptHashTable->cpfHashFunc = apfHashFunc ? apfHashFunc : &AIHashFuncDef;
    lptHashTable->ciBuckets = AI_MAX(aiBuckets, AI_HASH_SIZE_MIN);
    lptHashTable->ciKeySize = aiKeySize;
    lptHashTable->ciValSize = aiValSize;
    lptHashTable->ciTotalNodes = aiTotalNodes;
    ai_init_list_head(&(lptHashTable->ctFreeList));

    for (size_t n=0; n < aiTotalNodes; ++n)
    {
        AIHashNode_t*   lptHashNode;

        AI_MALLOC(lptHashNode, AIHashNode_t, sizeof(AIHashNode_t));
        AI_MALLOC(lptHashNode->cpKey, void, aiKeySize);
        AI_MALLOC(lptHashNode->cpVal, void, aiValSize);
        ai_list_add_tail(&(lptHashNode->ctBucketNode), &(lptHashTable->ctFreeList));
    }

    AI_MALLOC(lptHashTable->cptBuckets, LISTHEAD, sizeof(LISTHEAD)*(lptHashTable->ciBuckets));
    for (size_t n = 0; n < lptHashTable->ciBuckets; ++n)
    {
        ai_init_list_head(&(lptHashTable->cptBuckets[n]));
    }
    
    return lptHashTable;
}


////////////////////////////////////////////////////////////////////////////
void AIHashTableReset(AIHashTable_t aptHashTable)
{
    // move all nodes to free list
    for (size_t n = 0; n < aptHashTable->ciBuckets; ++n)
    {
        while (!ai_list_is_empty(&(aptHashTable->cptBuckets[n])))
        { 
            _ReleaseHashNode(
                aptHashTable,
                GET_HASHNODE_PTR(aptHashTable->cptBuckets[n].cpNext) );
        }
    }
}

////////////////////////////////////////////////////////////////////////////
void AIHashTableDestroy(AIHashTable_t aptHashTable)
{
    size_t	liCount = _FreeBucket(&(aptHashTable->ctFreeList));

    for (size_t n = 0; n < aptHashTable->ciBuckets; ++n)
    {
        liCount += _FreeBucket(&(aptHashTable->cptBuckets[n])); 
    }

    assert(liCount == aptHashTable->ciTotalNodes);

    AI_FREE(aptHashTable->cptBuckets);
    AI_FREE(aptHashTable); 
}


////////////////////////////////////////////////////////////////////////////
size_t AIHashTableGetKeySize(AIHashTable_t aptHashTable)
{
    return aptHashTable->ciKeySize;
}


////////////////////////////////////////////////////////////////////////////
size_t AIHashTableGetValSize(AIHashTable_t aptHashTable)
{
    return aptHashTable->ciValSize;
}


////////////////////////////////////////////////////////////////////////////
size_t AIHashTableGetHashSize(AIHashTable_t aptHashTable)  
{
    return aptHashTable->ciValSize;
}

////////////////////////////////////////////////////////////////////////////
bool AIHashTableHasKey(AIHashTable_t aptHashTable, void const* apKey, size_t aiKeyLen)
{
    if (aiKeyLen > aptHashTable->ciKeySize)
    {
        return AI_HASH_ERROR_KEYLEN;
    }
  
    AIHashKey_t	liHashKey = aptHashTable->cpfHashFunc(aptHashTable, apKey, aiKeyLen);
    return (NULL != _SearchBucket(aptHashTable, liHashKey, apKey, aiKeyLen));
}

////////////////////////////////////////////////////////////////////////////
int AIHashTablePut(AIHashTable_t aptHashTable, void const* apKey, size_t aiKeyLen, void const* apVal, size_t aiValLen)
{
    if (aiKeyLen > aptHashTable->ciKeySize)
    {
        return AI_HASH_ERROR_KEYLEN;
    }

    if (aiValLen > aptHashTable->ciValSize)
    {
        return AI_HASH_ERROR_VALLEN;
    }

    AIHashKey_t		liHashKey = aptHashTable->cpfHashFunc(aptHashTable, apKey, aiKeyLen);
    AIHashNode_t*	lptHashNode = _SearchBucket(aptHashTable, liHashKey, apKey, aiKeyLen);

    if (lptHashNode)
    {
        return AI_HASH_ERROR_DUPKEY;
    } 

    lptHashNode = _AcquireHashNode(aptHashTable, liHashKey);

    if (NULL == lptHashNode)
    {
        return AI_HASH_ERROR_FULL;
    }

    memcpy(lptHashNode->cpKey, apKey, aiKeyLen);
    lptHashNode->ciKeyLen = aiKeyLen;

    memcpy(lptHashNode->cpVal, apVal, aiValLen);
    lptHashNode->ciValLen = aiValLen;

    return 0;
}


////////////////////////////////////////////////////////////////////////////
int AIHashTableSet(AIHashTable_t aptHashTable, void const* apKey, size_t aiKeyLen, void const* apVal, size_t aiValLen)
{
    if (aiKeyLen > aptHashTable->ciKeySize)
    {
        return AI_HASH_ERROR_KEYLEN;
    }

    if (aiValLen > aptHashTable->ciValSize)
    {
        return AI_HASH_ERROR_VALLEN;
    }

    AIHashKey_t		liHashKey = aptHashTable->cpfHashFunc(aptHashTable, apKey, aiKeyLen);
    AIHashNode_t*	lptHashNode = _SearchBucket(aptHashTable, liHashKey, apKey, aiKeyLen);

    if (NULL == lptHashNode)
    {
        lptHashNode = _AcquireHashNode(aptHashTable, liHashKey);

        if (NULL == lptHashNode)
        {
            return AI_HASH_ERROR_FULL;
        }
    } 

    memcpy(lptHashNode->cpKey, apKey, aiKeyLen);
    lptHashNode->ciKeyLen = aiKeyLen;

    memcpy(lptHashNode->cpVal, apVal, aiValLen);
    lptHashNode->ciValLen = aiValLen;

    return 0;
}

////////////////////////////////////////////////////////////////////////////
int AIHashTableGet(AIHashTable_t aptHashTable, void const* apKey, size_t aiKeyLen, void* apVal, size_t* apiValLen)
{
    if (aiKeyLen > aptHashTable->ciKeySize)
    {
        return AI_HASH_ERROR_KEYLEN;
    }

    AIHashKey_t		liHashKey = aptHashTable->cpfHashFunc(aptHashTable, apKey, aiKeyLen);
    AIHashNode_t*	lptHashNode = _SearchBucket(aptHashTable, liHashKey, apKey, aiKeyLen);

    if (NULL == lptHashNode)
    {
        return AI_HASH_ERROR_NOTFOUND;
    }

    memcpy(apVal, lptHashNode->cpVal, AI_MIN(*apiValLen, lptHashNode->ciValLen));
    *apiValLen = AI_MIN(lptHashNode->ciValLen, *apiValLen);

    return 0;
}


////////////////////////////////////////////////////////////////////////////
int AIHashTablePop(AIHashTable_t aptHashTable, void const* apKey, size_t aiKeyLen, void* apVal, size_t* apiValLen)
{
    if (aiKeyLen > aptHashTable->ciKeySize)
    {
        return AI_HASH_ERROR_KEYLEN;
    }

    AIHashKey_t		liHashKey = aptHashTable->cpfHashFunc(aptHashTable, apKey, aiKeyLen);
    AIHashNode_t*	lptHashNode = _SearchBucket(aptHashTable, liHashKey, apKey, aiKeyLen);

    if (NULL == lptHashNode)
    {
        return AI_HASH_ERROR_NOTFOUND;
    }

    memcpy(apVal, lptHashNode->cpVal, AI_MIN(*apiValLen, lptHashNode->ciValLen));
    *apiValLen = AI_MIN(lptHashNode->ciValLen, *apiValLen);
  
    _ReleaseHashNode(aptHashTable, lptHashNode);

    return 0;
}


////////////////////////////////////////////////////////////////////////////
void AIHashTableTraversal(AIHashTable_t aptHashTable, AIHashTraversalFunc_t apfTravFunc, void* apArg)
{
    for (size_t n = 0; n < aptHashTable->ciBuckets; ++n)
    {
        LISTHEAD*	lptBucket = &(aptHashTable->cptBuckets[n]);
        
        for (LISTHEAD* lptLH = lptBucket->cpNext; lptLH != lptBucket; lptLH = lptLH->cpNext)
        {
            AIHashNode_t*	lptHashNode = GET_HASHNODE_PTR(lptLH);

            int		liRetCode = apfTravFunc(
                aptHashTable,
                lptHashNode->cpKey,
                lptHashNode->ciKeyLen,
                lptHashNode->cpVal,
                &(lptHashNode->ciValLen),
                apArg);

            switch (liRetCode)
            {
                case AI_HASH_TRAVERSAL_SKIPNODE:
                    continue;

                case AI_HASH_TRAVERSAL_SKIPLIST:
                    goto HASHLIST_END;

                case AI_HASH_TRAVERSAL_DELETE:
                    lptLH = lptLH->cpPrev;
                    _ReleaseHashNode(aptHashTable, lptHashNode);
                    break;

                case AI_HASH_TRAVERSAL_MODIFIED:
                    if (lptHashNode->ciValLen > aptHashTable->ciValSize)
                    { // what the hell!
                        abort(); 
                    } 
                    break;

                case AI_HASH_TRAVERSAL_EXIT:
                    return;

                default:
                    // what the hell too!
                    abort();
            }
        } // for all hash nodes
        
HASHLIST_END:
        do{}while(false);
    } // for all hash lists
}

///end namespace
AIBC_NAMESPACE_END
