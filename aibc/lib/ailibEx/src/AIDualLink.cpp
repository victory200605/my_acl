#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AIDualLink.h"

///start namespace
AIBC_NAMESPACE_START

/** 
    \brief Init pHead.
    \return NULL
*/
void ai_init_list_head( LISTHEAD *pHead )
{
    if( pHead )
    {
        pHead->cpNext = pHead;
        pHead->cpPrev = pHead;
    }
}

/** 
    \brief Insert pNew to middle of pPrev and pNext.
    \return NULL
*/
void ai_list_add_mid( LISTHEAD *pNew, LISTHEAD *pPrev, LISTHEAD *pNext )
{
    if( pNew && pPrev && pNext )
    {
        pNext->cpPrev = pNew;
        pNew->cpNext = pNext;
        pNew->cpPrev = pPrev;
        pPrev->cpNext = pNew;
    }
}

/** 
    \brief Insert pNew after pHead.
    \return NULL
*/
void ai_list_add_head( LISTHEAD *pNew, LISTHEAD *pHead )
{
    if( pNew && pHead )
    {
        pNew->cpPrev = pNew->cpNext = NULL;
        ai_list_add_mid( pNew, pHead, pHead->cpNext );
    }
}

/** 
    \brief Insert pNew before pHead.
    \return NULL
*/
void ai_list_add_tail( LISTHEAD *pNew, LISTHEAD *pHead )
{
    if( pNew && pHead )
    {
        pNew->cpPrev = pNew->cpNext = NULL;
        ai_list_add_mid( pNew, pHead->cpPrev, pHead );
    }
}

/** 
    \brief Get the firt item of list  into pRetHead.
    \return NULL
*/
void ai_list_peek_head( LISTHEAD *&pRetHead, LISTHEAD *pHead )
{
    if( pRetHead = NULL, pHead )
    {
        pRetHead = (pHead->cpNext==pHead)?NULL:pHead->cpNext;
    }
}

/** 
    \brief Get the last item of list  into pRetHead.
    \return NULL
*/
void ai_list_peek_tail( LISTHEAD *&pRetHead, LISTHEAD *pHead )
{
    if( pRetHead = NULL, pHead )
    {
        pRetHead = (pHead->cpPrev==pHead)?NULL:pHead->cpPrev;
    }
}

/** 
    \brief Delete the middle item of pPrev and pNext.
    \return NULL
*/
void ai_list_del_mid( LISTHEAD *pPrev, LISTHEAD *pNext )
{
    if( pPrev && pNext )
    {
        pNext->cpPrev = pPrev;
        pPrev->cpNext = pNext;
    }
}

/** 
    \brief Delete pEntry from list.
    \return NULL
*/
void ai_list_del_any( LISTHEAD *pEntry )
{
    if( pEntry )
    {
        ai_list_del_mid( pEntry->cpPrev, pEntry->cpNext );
        pEntry->cpPrev = pEntry->cpNext = NULL;
    }
}

/** 
    \brief Delete the item after pHead.
    \param pRetHead[OUT]: save the pointer of the item after pHead into it.
    \return NULL
*/
void ai_list_del_head( LISTHEAD *&pRetHead, LISTHEAD *pHead )
{
    if( pRetHead = NULL, pHead )
    {
        pRetHead = (pHead->cpNext==pHead)?NULL:pHead->cpNext;
        ai_list_del_any( pRetHead );
    }
}

/** 
    \brief Delete the item before pHead.
    \param pRetHead[OUT]: save the pointer of the item before pHead into it.
    \return NULL
*/
void ai_list_del_tail( LISTHEAD *&pRetHead, LISTHEAD *pHead )
{
    if( pRetHead = NULL, pHead )
    {
        pRetHead = (pHead->cpPrev==pHead)?NULL:pHead->cpPrev;
        ai_list_del_any( pRetHead );
    }
}

/** 
    \brief move pEntry to head.
    \param pEntry: the item will be moved to head.
    \return NULL
*/
void ai_list_moveto_head( LISTHEAD *pEntry, LISTHEAD *pHead )
{
    if( pEntry && pHead )
    {
        ai_list_del_mid( pEntry->cpPrev, pEntry->cpNext );
        pEntry->cpPrev = pEntry->cpNext = NULL;
        ai_list_add_head( pEntry, pHead );
    }
}
/** 
    \brief move pEntry to tail.
    \param pEntry: the item will be moved to tail.
    \return NULL
*/
void ai_list_moveto_tail( LISTHEAD *pEntry, LISTHEAD *pHead )
{
    if( pEntry && pHead )
    {
        ai_list_del_mid( pEntry->cpPrev, pEntry->cpNext );
        pEntry->cpPrev = pEntry->cpNext = NULL;
        ai_list_add_tail( pEntry, pHead );
    }
}

/** 
    \brief test the list is empty or not.
    \param pHead: the head of list
    \return 1:empty,0: not empty
*/
bool ai_list_is_empty( LISTHEAD const *pHead )
{
    return  pHead?(pHead->cpNext==pHead):1;
}

/** 
    \brief use pDestHead replace pSrcHead of the list,keep the old first item first  of the src list .
    \param pSrcHead: the old head of the list
    \param pDestHead: the new head of the list
    \return NULL
*/
void ai_list_join_head( LISTHEAD *pSrcHead, LISTHEAD *pDestHead )
{
    if( !ai_list_is_empty( pSrcHead ) && pDestHead )
    {
        LISTHEAD    *pFirst = pSrcHead->cpNext;
        LISTHEAD    *pLast = pSrcHead->cpPrev;
        LISTHEAD    *pAt = pDestHead->cpNext;

        pFirst->cpPrev = pDestHead;
        pDestHead->cpNext = pFirst;
        pLast->cpNext = pAt;
        pAt->cpPrev = pLast;

        pSrcHead->cpPrev = pSrcHead->cpNext = NULL;
        ai_init_list_head( pSrcHead );
    }
}

/** 
    \brief use pDestHead replace pSrcHead of the list,keep the old last item last of the src list 
    \param pSrcHead: the old head of the list
    \param pDestHead: the new head of the list
    \return NULL
*/
void ai_list_join_tail( LISTHEAD *pSrcHead, LISTHEAD *pDestHead )
{
    if( !ai_list_is_empty( pSrcHead ) && pDestHead )
    {
        LISTHEAD    *pFirst = pSrcHead->cpNext;
        LISTHEAD    *pLast = pSrcHead->cpPrev;
        LISTHEAD    *pAt = pDestHead->cpPrev;

        pFirst->cpPrev = pAt;
        pAt->cpNext = pFirst;
        pLast->cpNext = pDestHead;
        pDestHead->cpPrev = pLast;

        pSrcHead->cpPrev = pSrcHead->cpNext = NULL;
        ai_init_list_head( pSrcHead );
    }
}

/** 
    \brief swap two head for the two lists
    \param pHead1: 
    \param pHead2: 
    \return NULL
*/
void ai_list_swap( LISTHEAD* pHead1, LISTHEAD* pHead2 )
{
    bool lbEmpty1 = ai_list_is_empty(pHead1);
    bool lbEmpty2 = ai_list_is_empty(pHead2);
    if ( !lbEmpty1 && !lbEmpty2 )
    {
        LISTHEAD* pTmp1 = pHead1->cpNext;
        LISTHEAD* pTmp2 = pHead2->cpNext;
        ai_list_del_any( pHead1 );
        ai_list_del_any( pHead2 );
        ai_list_add_tail( pHead1, pTmp2 );
        ai_list_add_tail( pHead2, pTmp1 );
    }
    else if ( lbEmpty1 && !lbEmpty2 )
    {
        LISTHEAD* pTmp = pHead2->cpNext;
        ai_list_del_any( pHead2 );
        ai_list_add_tail( pHead1, pTmp );
    }
    else if ( !lbEmpty1 && lbEmpty2 )
    {
        LISTHEAD* pTmp = pHead1->cpNext;
        ai_list_del_any( pHead1 );
        ai_list_add_tail( pHead2, pTmp );
    }
    else
    {
        return;
    }
}

void ai_list_mergesort( LISTHEAD *apoListHead, pCOMPFunc CompareFunc )
{
    int             liLeftSize;
    int             liRightSize;
    int             liMergeTimes;
    int             liStepSize = 1;
    LISTHEAD        *lpoLeftHead = NULL;
    LISTHEAD        *lpoRightHead = NULL;
    LISTHEAD        *lpoAppendNode = NULL;
    LISTHEAD        *lpoListCurTail = NULL;
    LISTHEAD        *lpoListOldHead = NULL;
    LISTHEAD        *lpoNewListHead = NULL;
    

    // Emtpy Linked-List //
    if( apoListHead->cpNext == apoListHead )
        return;
    
    // Remove Head Node //
    lpoNewListHead = apoListHead->cpNext;
    lpoNewListHead->cpPrev = apoListHead->cpPrev;
    apoListHead->cpPrev->cpNext = lpoNewListHead;
    apoListHead->cpNext = apoListHead->cpPrev = NULL;

    while( 1 ) 
    {
        liMergeTimes = 0;
        lpoLeftHead = lpoNewListHead;
        lpoListOldHead = lpoNewListHead;
        lpoNewListHead = (LISTHEAD *)NULL;
        lpoListCurTail = (LISTHEAD *)NULL;

        while( lpoLeftHead )
        {
            liLeftSize = 0;
            liMergeTimes++;

            lpoRightHead = lpoLeftHead;
            for( int liIt = 0; (liIt < liStepSize && lpoRightHead); liIt++ ) 
            {
                liLeftSize++;
                lpoRightHead = (lpoRightHead->cpNext == lpoListOldHead ? NULL : lpoRightHead->cpNext);
            }

            // START MERGE //
            liRightSize = liStepSize;
            while( liLeftSize > 0 || (liRightSize > 0 && lpoRightHead) ) 
            {
                if( liLeftSize == 0 ) 
                {
                    liRightSize--;
                    lpoAppendNode = lpoRightHead; 
                    lpoRightHead = lpoRightHead->cpNext; 
                    if(lpoRightHead == lpoListOldHead)  lpoRightHead = NULL;
                } 
                else if( liRightSize == 0 || !lpoRightHead )
                {
                    liLeftSize--;
                    lpoAppendNode = lpoLeftHead; 
                    lpoLeftHead = lpoLeftHead->cpNext; 
                    if(lpoLeftHead == lpoListOldHead)  lpoLeftHead = NULL;
                } 
                else if( CompareFunc( lpoLeftHead, lpoRightHead ) <= 0 ) 
                {
                    liLeftSize--;
                    lpoAppendNode = lpoLeftHead; 
                    lpoLeftHead = lpoLeftHead->cpNext; 
                    if(lpoLeftHead == lpoListOldHead)  lpoLeftHead = NULL;
                } 
                else 
                {
                    liRightSize--;
                    lpoAppendNode = lpoRightHead; 
                    lpoRightHead = lpoRightHead->cpNext; 
                    if(lpoRightHead == lpoListOldHead) lpoRightHead = NULL;
                }

                if( lpoListCurTail )
                {
                    lpoListCurTail->cpNext = lpoAppendNode;
                } 
                else 
                {
                    lpoNewListHead = lpoAppendNode;
                }
                
                lpoAppendNode->cpPrev = lpoListCurTail;
                lpoListCurTail = lpoAppendNode;
            }

            lpoLeftHead = lpoRightHead;
        }

        // Rebuild Circle Linked-list //
        lpoListCurTail->cpNext = lpoNewListHead;
        lpoNewListHead->cpPrev = lpoListCurTail;

        if( liMergeTimes <= 1 )
        {
            // FINISHED MERGESORT //
            apoListHead->cpNext = lpoNewListHead;
            apoListHead->cpPrev = lpoNewListHead->cpPrev;
            
            lpoNewListHead->cpPrev = apoListHead;
            apoListHead->cpPrev->cpNext = apoListHead;

            return;
        }

        liStepSize *= 2;
    }
}

///end namespace
AIBC_NAMESPACE_END
