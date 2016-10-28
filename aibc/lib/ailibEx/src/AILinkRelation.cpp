#include "AILinkRelation.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

///start namespace
AIBC_NAMESPACE_START

//////////////////////////////////////////////////////////////////////////
//
clsLinkRelation::clsLinkRelation(size_t aiHashSize)
    :ciCurSize(0)
{
    ciHashSize = ((aiHashSize<16)?16:aiHashSize);
    
    AI_NEW_N_ASSERT(cpoConnHead, LISTHEAD, ciHashSize);
    for(size_t liIt = 0; liIt < ciHashSize; liIt++)
    {
        ai_init_list_head(&(cpoConnHead[liIt]));
    }

    ai_init_list_head(&coListHead);
}

clsLinkRelation::~clsLinkRelation()
{
    clsLinkRelation::RemoveAll();
    AI_DELETE_N(cpoConnHead);
}

//////////////////////////////////////////////////////////////////////////
//
int clsLinkRelation::Put(unsigned int aiSequenceNo, void *apoConnObj, unsigned int aiOld)
{
    int             liIndex = 0;
    LISTHEAD        *lpoHead = NULL;
    SEQCONNOBJ      *lpoSeqConnObj = NULL;
    
    liIndex = aiSequenceNo % ciHashSize;
    AI_NEW_ASSERT(lpoSeqConnObj, SEQCONNOBJ);
    lpoSeqConnObj->ciSequenceNo = aiSequenceNo;
    lpoSeqConnObj->cpoConnObj = apoConnObj;
    lpoSeqConnObj->ctEnterTime = time(NULL);
    lpoSeqConnObj->ciOldSequence = aiOld;
    
    AISmartLock loSmartLock(coMutexLock);
    lpoHead = &(cpoConnHead[liIndex]);
    ai_list_add_tail(&(lpoSeqConnObj->coSeqConnChild), lpoHead);
    ai_list_add_tail(&(lpoSeqConnObj->coListChild), &coListHead);
   
    ++ciCurSize;
     
    return  (0);
}
    
//////////////////////////////////////////////////////////////////////////
//
int clsLinkRelation::Get(unsigned int aiSequenceNo, void *&apoConnObj, unsigned int &aiOld, bool abToRemove)
{
    int             liIndex = 0;
    LISTHEAD        *lpoTemp = NULL;
    LISTHEAD        *lpoHead = NULL;
    SEQCONNOBJ      *lpoSeqConnObj = NULL;
    
    liIndex = aiSequenceNo % ciHashSize;
    
    AISmartLock loSmartLock(coMutexLock);
   
    lpoHead = &(cpoConnHead[liIndex]);
    
    for(lpoTemp = lpoHead->cpNext; lpoTemp != lpoHead; lpoTemp = lpoTemp->cpNext)
    {
        lpoSeqConnObj = AI_GET_STRUCT_PTR(lpoTemp, SEQCONNOBJ, coSeqConnChild);
        if(lpoSeqConnObj->ciSequenceNo == aiSequenceNo)
        {
            aiOld = lpoSeqConnObj->ciOldSequence;
            apoConnObj = lpoSeqConnObj->cpoConnObj;
            
            if(abToRemove) // Remove From Queue
            {
                ai_list_del_any(&(lpoSeqConnObj->coSeqConnChild));
                ai_list_del_any(&(lpoSeqConnObj->coListChild));
                AI_DELETE(lpoSeqConnObj);
            }
    
            --ciCurSize;
                    
            return  (0);
        }
    }
    
    apoConnObj = NULL;

    return  (-1);
}

int clsLinkRelation::Get(unsigned int aiSequenceNo, bool abToRemove)
{
    void            *lpvObject;
    unsigned int    liSequenceNo;
    
    return Get(aiSequenceNo, lpvObject, liSequenceNo, abToRemove);
}

    
//////////////////////////////////////////////////////////////////////////
//
size_t clsLinkRelation::Remove(void const* apoConnObj)
{
    int             liRmvCnt = 0;
    LISTHEAD        *lpoTemp = NULL;
    LISTHEAD        *lpoHead = NULL;
    SEQCONNOBJ      *lpoSeqConnObj = NULL;

    for(size_t liIndex = 0; liIndex < ciHashSize; liIndex++)
    {
        AISmartLock loSmartLock(coMutexLock);
        
TRYAGAIN:       
        lpoHead = &(cpoConnHead[liIndex]);
        for(lpoTemp = lpoHead->cpNext; lpoTemp != lpoHead; lpoTemp = lpoTemp->cpNext)
        {
            lpoSeqConnObj = AI_GET_STRUCT_PTR(lpoTemp, SEQCONNOBJ, coSeqConnChild);
            if(lpoSeqConnObj->cpoConnObj == apoConnObj)
            {
                ai_list_del_any(&(lpoSeqConnObj->coSeqConnChild));
                ai_list_del_any(&(lpoSeqConnObj->coListChild));
                AI_DELETE(lpoSeqConnObj);
                
                ++liRmvCnt;
                --ciCurSize;

                goto  TRYAGAIN;
            }
        }
    }
    
    return  (liRmvCnt);
}


size_t clsLinkRelation::RemoveAll(time_t aiTimeout)
{
    int             liRmvCnt = 0;
    time_t          ltCurrentTime;
    LISTHEAD        *lpoTemp = NULL;
    LISTHEAD        *lpoHead = NULL;
    SEQCONNOBJ      *lpoSeqConnObj = NULL;

    ltCurrentTime = time(NULL);
    for(size_t liIndex = 0; liIndex < ciHashSize; liIndex++)
    {
        AISmartLock loSmartLock(coMutexLock);
        
TRYAGAIN:       
        lpoHead = &(cpoConnHead[liIndex]);
        for(lpoTemp = lpoHead->cpNext; lpoTemp != lpoHead; lpoTemp = lpoTemp->cpNext)
        {
            lpoSeqConnObj = AI_GET_STRUCT_PTR(lpoTemp, SEQCONNOBJ, coSeqConnChild);
            
            if((aiTimeout<=0) || (ltCurrentTime-lpoSeqConnObj->ctEnterTime) >= aiTimeout)
            {
                ai_list_del_any(&(lpoSeqConnObj->coSeqConnChild));
                ai_list_del_any(&(lpoSeqConnObj->coListChild));
                AI_DELETE(lpoSeqConnObj);
                
                ++liRmvCnt;
                --ciCurSize;
                goto  TRYAGAIN;
            }
        }
    }
    
    return  (liRmvCnt);
}


int clsLinkRelation::GetTimeout(unsigned int& aiSequenceNo, void*& apoConnObj, unsigned int& aiOld, time_t aiTimeout)
{
    SEQCONNOBJ* lpoSeqConnObj;

    AISmartLock loSmartLock(coMutexLock);

    if (ai_list_is_empty(&coListHead))
    {
        return -1;
    }

    lpoSeqConnObj = AI_GET_STRUCT_PTR(coListHead.cpNext, SEQCONNOBJ, coListChild);

    if((aiTimeout<=0) || (time(NULL) - lpoSeqConnObj->ctEnterTime) >= aiTimeout)
    {
        aiSequenceNo    = lpoSeqConnObj->ciSequenceNo;
        aiOld           = lpoSeqConnObj->ciOldSequence;
        apoConnObj      = lpoSeqConnObj->cpoConnObj;

        ai_list_del_any(&(lpoSeqConnObj->coSeqConnChild));
        ai_list_del_any(&(lpoSeqConnObj->coListChild));

        AI_DELETE(lpoSeqConnObj);

        --ciCurSize;

        return 0;
    }

    return -1;
}


bool clsLinkRelation::IsEmpty()
{
    AISmartLock loSmartLock(coMutexLock);
   
    return (ai_list_is_empty(&coListHead)); 
}

size_t clsLinkRelation::GetSize()
{
    return ciCurSize;
}

///end namespace
AIBC_NAMESPACE_END
