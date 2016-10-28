#ifndef __AILIBEX__AIINDEXLINK_H__
#define __AILIBEX__AIINDEXLINK_H__

#include "AIDualLink.h"
#include "AILib.h"

///start namespace
AIBC_NAMESPACE_START

////////////////////////////////////////////////////////////////////
//
#define  AIL_STATUS_NONE     (0)
#define  AIL_STATUS_EXIST    (1)
#define  AIL_STATUS_NEWADD   (2)
#define  AIL_STATUS_MODIFY   (3)
#define  AIL_STATUS_DELETE   (4)

////////////////////////////////////////////////////////////////////
//
typedef struct IXL_HEAD
{
    int         ciStatus;
    IXL_HEAD    *cpoCHILD[2];
} IXL_HEAD;

////////////////////////////////////////////////////////////////////
//
typedef int (*indexlinkCompare)(void*, void*);

////////////////////////////////////////////////////////////////////
//
template< class INDXNODE >
class clsIndexLink
{
    private:
        int                 ciCount;
        LISTHEAD            coLinkHead;
        IXL_HEAD            *cpoRootNode;
        indexlinkCompare    cpfCompareFunc;

    public:
        clsIndexLink();
        ~clsIndexLink(void) { ClearAll(); }

    private:        
        IXL_HEAD *AccessNode(INDXNODE *apoKeyNode);
        IXL_HEAD *AddNewNode(IXL_HEAD *apoNewNode);
        
    public:
        int GetCount(void);
        int MergeSort(void);
        int LoadLink(LISTHEAD *apoNodeLink, const int aiRemoved = 0);

    public:
        void ClearAll(void);
        void ClearStatus(void);
        void SetCompare(indexlinkCompare apfCompare);

    public:
        void SetCompareEnd(void);
        void SetCompareBegin(void);
        void ModifyStatus(INDXNODE *apoOldNode);
        INDXNODE *GetStatusHead(int &aiStatus);
        INDXNODE *GetStatusItem(INDXNODE *apoPrevNode, int &aiStatus);
        
    public:
        INDXNODE *GetNextHead(void);
        INDXNODE *AddNewNode(INDXNODE *apoNewNode);
        INDXNODE *RemoveNode(INDXNODE *apoKeyNode);
        INDXNODE *SearchNode(INDXNODE *apoKeyNode);
        INDXNODE *GetNextItem(INDXNODE *apoPrevNode);
};

////////////////////////////////////////////////////////////////////
//
template< class INDXNODE >
void clsIndexLink<INDXNODE>::ClearAll(void)
{
    LISTHEAD        *lpoTemp = NULL;
    IXL_HEAD        *lpoHeadNode = NULL;
    INDXNODE        *lpoIndxNode = NULL;
    
    ciCount = 0;
    cpoRootNode = NULL;
    while(!ai_list_is_empty(&coLinkHead))
    {
        ai_list_del_head(lpoTemp, &coLinkHead);
        lpoIndxNode = AI_GET_STRUCT_PTR(lpoTemp, INDXNODE, coChild);

        lpoHeadNode = (IXL_HEAD *)((char *)lpoIndxNode - sizeof(IXL_HEAD));
        AI_FREE(lpoHeadNode);
    }
}

template< class INDXNODE >
void clsIndexLink<INDXNODE>::SetCompareBegin(void)
{
    LISTHEAD        *lpoHead = NULL;
    LISTHEAD        *lpoTemp = NULL;
    IXL_HEAD        *lpoHeadNode = NULL;
    INDXNODE        *lpoIndxNode = NULL;

    lpoHead = &coLinkHead;  
    for(lpoTemp = lpoHead->cpNext; lpoTemp != lpoHead; lpoTemp = lpoTemp->cpNext)
    {
        lpoIndxNode = AI_GET_STRUCT_PTR(lpoTemp, INDXNODE, coChild);

        lpoHeadNode = (IXL_HEAD *)((char *)lpoIndxNode - sizeof(IXL_HEAD));
        lpoHeadNode->ciStatus = AIL_STATUS_NONE;
    }
}

template< class INDXNODE >
void clsIndexLink<INDXNODE>::ModifyStatus(INDXNODE *apoOldNode)
{
    IXL_HEAD        *lpoHeadNode = NULL;

    assert(apoOldNode);
    lpoHeadNode = (IXL_HEAD *)((char *)apoOldNode - sizeof(IXL_HEAD));

    if(lpoHeadNode->ciStatus == AIL_STATUS_NONE || \
        lpoHeadNode->ciStatus == AIL_STATUS_EXIST || \
        lpoHeadNode->ciStatus == AIL_STATUS_MODIFY)
    {
        lpoHeadNode->ciStatus = AIL_STATUS_MODIFY;
    }
}

template< class INDXNODE >
void clsIndexLink<INDXNODE>::SetCompareEnd(void)
{
    LISTHEAD        *lpoHead = NULL;
    LISTHEAD        *lpoTemp = NULL;
    IXL_HEAD        *lpoHeadNode = NULL;
    INDXNODE        *lpoIndxNode = NULL;

    lpoHead = &coLinkHead;  
    for(lpoTemp = lpoHead->cpNext; lpoTemp != lpoHead; lpoTemp = lpoTemp->cpNext)
    {
        lpoIndxNode = AI_GET_STRUCT_PTR(lpoTemp, INDXNODE, coChild);
        lpoHeadNode = (IXL_HEAD *)((char *)lpoIndxNode - sizeof(IXL_HEAD));
        
        if(lpoHeadNode->ciStatus == AIL_STATUS_NONE)
        {
            lpoHeadNode->ciStatus = AIL_STATUS_DELETE;
        }
    }
}

template< class INDXNODE >
clsIndexLink<INDXNODE>::clsIndexLink()
{
    ciCount = 0;
    cpoRootNode = NULL;
    cpfCompareFunc = NULL;
    ai_init_list_head(&coLinkHead);
}

template< class INDXNODE >
void clsIndexLink<INDXNODE>::SetCompare(indexlinkCompare apfCompare)
{
    cpfCompareFunc = apfCompare;
}

template< class INDXNODE >
IXL_HEAD *clsIndexLink<INDXNODE>::AccessNode(INDXNODE *apoKeyNode)
{
    int             liRetCode;
    IXL_HEAD        stHeadNode;
    IXL_HEAD        *lpoMaxNode = NULL;
    IXL_HEAD        *lpoMinNode = NULL;
    IXL_HEAD        *lpoTempNode = NULL;
    INDXNODE        *lpoIndxNode = NULL;

    stHeadNode.cpoCHILD[0] = stHeadNode.cpoCHILD[1] = NULL;
    lpoMaxNode = lpoMinNode = &stHeadNode;
    
    lpoIndxNode = (INDXNODE *)(cpoRootNode + 1);
    while( (liRetCode = cpfCompareFunc(apoKeyNode, lpoIndxNode) ) )
    {
        if(liRetCode < 0)
        {
            if(cpoRootNode->cpoCHILD[0] == NULL)
                break;

            lpoIndxNode = (INDXNODE *)(cpoRootNode->cpoCHILD[0] + 1);
            if(cpfCompareFunc(apoKeyNode, lpoIndxNode) < 0)
            {
                lpoTempNode = cpoRootNode->cpoCHILD[0];
                cpoRootNode->cpoCHILD[0] = lpoTempNode->cpoCHILD[1];
                lpoTempNode->cpoCHILD[1] = cpoRootNode;
                cpoRootNode = lpoTempNode;

                if(cpoRootNode->cpoCHILD[0] == NULL)
                    break;
            }

            lpoMinNode->cpoCHILD[0] = cpoRootNode, lpoMinNode = cpoRootNode;
            cpoRootNode = cpoRootNode->cpoCHILD[0];
        }
        else if(liRetCode > 0)
        {
            if(cpoRootNode->cpoCHILD[1] == NULL)
                break;

            lpoIndxNode = (INDXNODE *)(cpoRootNode->cpoCHILD[1] + 1);
            if(cpfCompareFunc(apoKeyNode, lpoIndxNode) > 0)
            {
                lpoTempNode = cpoRootNode->cpoCHILD[1];
                cpoRootNode->cpoCHILD[1] = lpoTempNode->cpoCHILD[0];
                lpoTempNode->cpoCHILD[0] = cpoRootNode;
                cpoRootNode = lpoTempNode;

                if(cpoRootNode->cpoCHILD[1] == NULL)
                    break;
            }

            lpoMaxNode->cpoCHILD[1] = cpoRootNode, lpoMaxNode = cpoRootNode;
            cpoRootNode = cpoRootNode->cpoCHILD[1];
        }
        else
        {
            break;
        }
    
        lpoIndxNode = (INDXNODE *)(cpoRootNode + 1);
    }

    lpoMaxNode->cpoCHILD[1] = cpoRootNode->cpoCHILD[0];
    lpoMinNode->cpoCHILD[0] = cpoRootNode->cpoCHILD[1];
    cpoRootNode->cpoCHILD[0] = stHeadNode.cpoCHILD[1];
    cpoRootNode->cpoCHILD[1] = stHeadNode.cpoCHILD[0];

    return  (cpoRootNode);
}

template< class INDXNODE >
IXL_HEAD *clsIndexLink<INDXNODE>::AddNewNode(IXL_HEAD *apoNewNode)
{
    int             liRetCode = 0;
    INDXNODE        *lpoNewIndxNode;
    INDXNODE        *lpoRootIndxNode;

    if(apoNewNode == (IXL_HEAD *)NULL)
        return  (NULL);

    lpoNewIndxNode = (INDXNODE *)(apoNewNode + 1);
    if(cpfCompareFunc == NULL)
    {
        ai_list_add_tail(&(lpoNewIndxNode->coChild), &coLinkHead);
        apoNewNode->ciStatus = AIL_STATUS_NEWADD;
        ciCount += 1;
        
        return  (apoNewNode);
    }
    
    if(cpoRootNode == NULL)
    {
        apoNewNode->cpoCHILD[0] = apoNewNode->cpoCHILD[1] = NULL;
        cpoRootNode = apoNewNode;

        ai_list_add_tail(&(lpoNewIndxNode->coChild), &coLinkHead);
        apoNewNode->ciStatus = AIL_STATUS_NEWADD;
        ciCount += 1;
        
        return  (cpoRootNode);
    }

    cpoRootNode = AccessNode(lpoNewIndxNode);
    lpoRootIndxNode = (INDXNODE *)(cpoRootNode + 1);
    
    if((liRetCode = cpfCompareFunc(lpoNewIndxNode, lpoRootIndxNode)) < 0)
    {
        apoNewNode->cpoCHILD[0] = cpoRootNode->cpoCHILD[0];
        apoNewNode->cpoCHILD[1] = cpoRootNode;
        cpoRootNode->cpoCHILD[0] = NULL;
        cpoRootNode = apoNewNode;

        ai_list_add_tail(&(lpoNewIndxNode->coChild), &coLinkHead);
        apoNewNode->ciStatus = AIL_STATUS_NEWADD;
        ciCount += 1;
        
        return  (cpoRootNode);
    }

    if(liRetCode > 0)
    {
        apoNewNode->cpoCHILD[1] = cpoRootNode->cpoCHILD[1];
        apoNewNode->cpoCHILD[0] = cpoRootNode;
        cpoRootNode->cpoCHILD[1] = NULL;
        cpoRootNode = apoNewNode;

        ai_list_add_tail(&(lpoNewIndxNode->coChild), &coLinkHead);
        apoNewNode->ciStatus = AIL_STATUS_NEWADD;
        ciCount += 1;
        
        return  (cpoRootNode);
    }

    return  (NULL);
}

template< class INDXNODE >
INDXNODE *clsIndexLink<INDXNODE>::AddNewNode(INDXNODE *apoNewNode)
{
    int             liSize = 0;
    IXL_HEAD        *lpoNewHeadNode;
    IXL_HEAD        *lpoRootHeadNode;

    if(apoNewNode == (INDXNODE *)NULL)
        return  (NULL);

    liSize = sizeof(IXL_HEAD) + sizeof(INDXNODE);
    AI_MALLOC(lpoNewHeadNode, IXL_HEAD, liSize);
    memcpy((char *)lpoNewHeadNode+sizeof(IXL_HEAD), apoNewNode, sizeof(INDXNODE));

    if((lpoRootHeadNode = AddNewNode(lpoNewHeadNode)) == NULL)
    {
        AI_FREE(lpoNewHeadNode);
        return  (NULL);
    }
    
    return  (INDXNODE *)(lpoRootHeadNode+1);
}

template< class INDXNODE >
INDXNODE *clsIndexLink<INDXNODE>::RemoveNode(INDXNODE *apoKeyNode)
{
    INDXNODE        *lpoReturn = NULL;
    IXL_HEAD        *lpoTempNode = NULL;
    INDXNODE        *lpoRootNode = NULL;

    if(cpfCompareFunc == NULL)
        return  (NULL);

    if(cpoRootNode == (IXL_HEAD *)NULL)
        return  (NULL);

    cpoRootNode = AccessNode(apoKeyNode);
    lpoRootNode = (INDXNODE *)(cpoRootNode + 1);
    
    if(cpfCompareFunc(apoKeyNode, lpoRootNode))
        return  (NULL);

    if(cpoRootNode->cpoCHILD[0] == NULL)
    {
        IXL_HEAD        *pRetNode = NULL;

        lpoTempNode = cpoRootNode->cpoCHILD[1];
        pRetNode = cpoRootNode;
        cpoRootNode = lpoTempNode;

        lpoRootNode = (INDXNODE *)(pRetNode + 1);
        ai_list_del_any(&(lpoRootNode->coChild));
        ciCount -= 1;
        
        AI_NEW_ASSERT(lpoReturn, INDXNODE);
        memcpy(lpoReturn, lpoRootNode, sizeof(INDXNODE));
        
        AI_FREE(pRetNode);
        return  (pRetNode);
    }

    lpoTempNode = cpoRootNode;

    cpoRootNode = cpoRootNode->cpoCHILD[0];
    cpoRootNode = AccessNode(apoKeyNode);
    cpoRootNode->cpoCHILD[1] = lpoTempNode->cpoCHILD[1];

    lpoRootNode = (INDXNODE *)(lpoTempNode + 1);
    ai_list_del_any(&(lpoRootNode->coChild));
    ciCount -= 1;
    
    AI_NEW_ASSERT(lpoReturn, INDXNODE);
    memcpy(lpoReturn, lpoRootNode, sizeof(INDXNODE));
        
    AI_FREE(lpoTempNode);
    return  (lpoTempNode);
}

template< class INDXNODE >
INDXNODE *clsIndexLink<INDXNODE>::SearchNode(INDXNODE *apoKeyNode)
{
    IXL_HEAD        *lpoHeadNode = NULL;
    INDXNODE        *lpoIndxNode = NULL;

    if(cpfCompareFunc == NULL)
        return  (NULL);

    if(cpoRootNode == (IXL_HEAD *)NULL)
        return  (NULL);

    lpoHeadNode = AccessNode(apoKeyNode);
    lpoIndxNode = (INDXNODE *)(lpoHeadNode + 1);

    if(cpfCompareFunc(apoKeyNode, lpoIndxNode))
        return  (NULL);

    if(lpoHeadNode->ciStatus == AIL_STATUS_NONE || \
        lpoHeadNode->ciStatus == AIL_STATUS_EXIST)
    {
        lpoHeadNode->ciStatus = AIL_STATUS_EXIST;
    }
    
    return  (lpoIndxNode);
}

template< class INDXNODE >
INDXNODE *clsIndexLink<INDXNODE>::GetNextHead(void)
{
    LISTHEAD        *lpoTemp = NULL;
    LISTHEAD        *lpoHead = NULL;
    INDXNODE        *lpoIndxNode = NULL;
    
    lpoHead = &coLinkHead;
    if(ai_list_is_empty(lpoHead))
        return  (NULL);
        
    lpoTemp = lpoHead->cpNext;
    lpoIndxNode = AI_GET_STRUCT_PTR(lpoTemp, INDXNODE, coChild);
    
    return  (lpoIndxNode);
}

template< class INDXNODE >
INDXNODE *clsIndexLink<INDXNODE>::GetNextItem(INDXNODE *apoPrevNode)
{
    LISTHEAD        *lpoTemp = NULL;
    LISTHEAD        *lpoHead = NULL;
    INDXNODE        *lpoIndxNode = NULL;
    
    lpoHead = &coLinkHead;
    if(ai_list_is_empty(lpoHead))
        return  (NULL);
    
    if((lpoTemp = apoPrevNode->coChild.cpNext) == lpoHead)
        return  (NULL);

    lpoIndxNode = AI_GET_STRUCT_PTR(lpoTemp, INDXNODE, coChild);
    return  (lpoIndxNode);
}

template< class INDXNODE >
INDXNODE *clsIndexLink<INDXNODE>::GetStatusHead(int &aiStatus)
{
    LISTHEAD        *lpoTemp = NULL;
    LISTHEAD        *lpoHead = NULL;
    IXL_HEAD        *lpoHeadNode = NULL;
    INDXNODE        *lpoIndxNode = NULL;
    
    lpoHead = &coLinkHead;
    aiStatus = AIL_STATUS_NONE;
    
    if(ai_list_is_empty(lpoHead))
        return  (NULL);
        
    lpoTemp = lpoHead->cpNext;
    lpoIndxNode = AI_GET_STRUCT_PTR(lpoTemp, INDXNODE, coChild);
    
    lpoHeadNode = (IXL_HEAD *)((char *)lpoIndxNode - sizeof(IXL_HEAD));
    aiStatus = lpoHeadNode->ciStatus;
    
    return  (lpoIndxNode);
}

template< class INDXNODE >
INDXNODE *clsIndexLink<INDXNODE>::GetStatusItem(INDXNODE *apoPrevNode, int &aiStatus)
{
    LISTHEAD        *lpoTemp = NULL;
    LISTHEAD        *lpoHead = NULL;
    IXL_HEAD        *lpoHeadNode = NULL;
    INDXNODE        *lpoIndxNode = NULL;
    
    lpoHead = &coLinkHead;
    aiStatus = AIL_STATUS_NONE;
    
    if(ai_list_is_empty(lpoHead))
        return  (NULL);
    
    if((lpoTemp = apoPrevNode->coChild.cpNext) == lpoHead)
        return  (NULL);

    lpoIndxNode = AI_GET_STRUCT_PTR(lpoTemp, INDXNODE, coChild);
    lpoHeadNode = (IXL_HEAD *)((char *)lpoIndxNode - sizeof(IXL_HEAD));

    aiStatus = lpoHeadNode->ciStatus;
    return  (lpoIndxNode);
}

template< class INDXNODE >
int clsIndexLink<INDXNODE>::GetCount(void)
{
    return  (ciCount);
}

template< class INDXNODE >
int clsIndexLink<INDXNODE>::LoadLink(LISTHEAD *apoNodeLink, const int aiRemoved)
{
    int             liSize = 0;
    int             liCount = 0;
    LISTHEAD        *lpoHead = NULL;
    LISTHEAD        *lpoTemp = NULL;
    IXL_HEAD        *lpoNewNode = NULL;
    INDXNODE        *lpoTempNode = NULL;
    
    if((lpoHead = apoNodeLink) == NULL)
        return  (0);
        
    if(ai_list_is_empty(lpoHead))
        return  (0);

    liSize = sizeof(IXL_HEAD) + sizeof(INDXNODE);
    clsIndexLink<INDXNODE>::ClearAll();
    
AGAIN1:     
    for(lpoTemp = lpoHead->cpNext; lpoTemp != lpoHead; lpoTemp = lpoTemp->cpNext)
    {
        liCount += 1;
        lpoTempNode = AI_GET_STRUCT_PTR(lpoTemp, INDXNODE, coChild);

        AI_MALLOC(lpoNewNode, IXL_HEAD, liSize);
        memcpy((char *)lpoNewNode+sizeof(IXL_HEAD), lpoTempNode, sizeof(INDXNODE));
                
        if(AddNewNode(lpoNewNode) == NULL)
        {
            AI_FREE(lpoNewNode);
            lpoNewNode = NULL;
        }
                
        if(aiRemoved)
        {   
            ai_list_del_any(lpoTemp);
            AI_DELETE(lpoTempNode);
            goto  AGAIN1;
        }
    }
    
    clsIndexLink<INDXNODE>::MergeSort();
    return  (liCount);
}

template< class INDXNODE >
int clsIndexLink<INDXNODE>::MergeSort(void)
{
    int             liLeftSize;
    int             liRightSize;
    int             liMergeTimes;
    int             liStepSize = 1;
    INDXNODE        *lpoLeftNode = NULL;
    LISTHEAD        *lpoLeftHead = NULL;
    INDXNODE        *lpoRightNode = NULL;
    LISTHEAD        *lpoRightHead = NULL;
    LISTHEAD        *lpoAppendNode = NULL;
    LISTHEAD        *lpoListCurTail = NULL;
    LISTHEAD        *lpoListOldHead = NULL;
    LISTHEAD        *lpoNewListHead = NULL;

    if(coLinkHead.cpNext == &coLinkHead)
        return  (1);
        
    if(cpfCompareFunc == NULL)
        return  (2);
    
    // Remove Head Node //
    lpoNewListHead = coLinkHead.cpNext;
    lpoNewListHead->cpPrev = coLinkHead.cpPrev;
    coLinkHead.cpPrev->cpNext = lpoNewListHead;
    coLinkHead.cpNext = coLinkHead.cpPrev = NULL;

    while(1) 
    {
        liMergeTimes = 0;
        lpoLeftHead = lpoNewListHead;
        lpoListOldHead = lpoNewListHead;
        lpoNewListHead = (LISTHEAD *)NULL;
        lpoListCurTail = (LISTHEAD *)NULL;

        while(lpoLeftHead)
        {
            liLeftSize = 0;
            liMergeTimes++;

            lpoRightHead = lpoLeftHead;
            for(int liIt = 0; (liIt < liStepSize && lpoRightHead); liIt++) 
            {
                liLeftSize++;
                lpoRightHead = (lpoRightHead->cpNext == lpoListOldHead ? NULL : lpoRightHead->cpNext);
            }

            // START MERGE //
            liRightSize = liStepSize;
            while(liLeftSize > 0 || (liRightSize > 0 && lpoRightHead)) 
            {
                if(lpoLeftHead)  lpoLeftNode = AI_GET_STRUCT_PTR(lpoLeftHead, INDXNODE, coChild);
                if(lpoRightHead)  lpoRightNode = AI_GET_STRUCT_PTR(lpoRightHead, INDXNODE, coChild);

                if(liLeftSize == 0) 
                {
                    liRightSize--;
                    lpoAppendNode = lpoRightHead; 
                    lpoRightHead = lpoRightHead->cpNext; 
                    if(lpoRightHead == lpoListOldHead)  lpoRightHead = NULL;
                } 
                else if(liRightSize == 0 || !lpoRightHead)
                {
                    liLeftSize--;
                    lpoAppendNode = lpoLeftHead; 
                    lpoLeftHead = lpoLeftHead->cpNext; 
                    if(lpoLeftHead == lpoListOldHead)  lpoLeftHead = NULL;
                } 
                else if(cpfCompareFunc(lpoLeftNode, lpoRightNode) <= 0) 
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

                if(lpoListCurTail)
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

        if(liMergeTimes <= 1)
        {
            // FINISHED MERGESORT //
            coLinkHead.cpNext = lpoNewListHead;
            coLinkHead.cpPrev = lpoNewListHead->cpPrev;
            
            lpoNewListHead->cpPrev = &coLinkHead;
            coLinkHead.cpPrev->cpNext = &coLinkHead;

            return  (0);
        }

        liStepSize *= 2;
    }
    
    return  (0);
}

///end namespace
AIBC_NAMESPACE_END

#endif   // __AILIBEX__AIINDEXLINK_H__

