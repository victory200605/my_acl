
#include "ServerNodePriorityList.h"

AIBC_GFQ_NAMESPACE_START

CServerNodePriorityList::CServerNodePriorityList(void)
{
    //priority [0, 10)
    ACL_NEW_N_ASSERT(this->mpoPriList, CListNode, 10);

    for (apl_size_t luN = 0; luN < 10; luN++)
    {
        this->mpoPriList[luN].miActivatedSize = 0;
        this->mpoPriList[luN].miSuspendedSize = 0;
    }
}

CServerNodePriorityList::~CServerNodePriorityList(void)
{
    ACL_DELETE_N(this->mpoPriList);
}

bool CServerNodePriorityList::Insert( CServerNode* apoServerNode )
{
    apl_int_t liPriority = apoServerNode->GetPriority();

    ACL_ASSERT(liPriority >= 0 && liPriority < 10);

    CListNode& loList = this->mpoPriList[liPriority];

    //If current priority list is empty and insert to @moAvailable list
    //Then it was existed in @moAvailable list
    if (loList.miSuspendedSize + loList.miActivatedSize == 0)
    {
        CListNode* lpoNext = APL_NULL;

        //Find next non-empty priority list and current list should be insert before it in @moAvailable
        //Otherwise inserting to the tail in @moAvailable
        for (apl_int_t liN = liPriority + 1; liN < 10; liN++)
        {
            CListNode& loTemp = this->mpoPriList[liN];
            if (loTemp.miSuspendedSize + loTemp.miActivatedSize > 0)
            {
                lpoNext = &loTemp;
                break;
            }
        }

        if (lpoNext == APL_NULL)
        {
            this->moAvailable.InsertTail(&loList);
        }
        else
        {
            this->moAvailable.InsertBefore(lpoNext, &loList);
        }
    }

    loList.miSuspendedSize++;
    loList.moSuspended.InsertTail(apoServerNode);

    return true;
}

bool CServerNodePriorityList::Remove( CServerNode* apoServerNode )
{
    apl_int_t liPriority = apoServerNode->GetPriority();

    ACL_ASSERT(liPriority >= 0 && liPriority < 10);
    
    CListNode& loList = this->mpoPriList[liPriority];
    
    if (loList.moActivated.IsExisted(apoServerNode) )
    {
        loList.miActivatedSize--;
        loList.moActivated.Remove(apoServerNode);
    }
    else if (loList.moSuspended.IsExisted(apoServerNode) )
    {
        loList.miSuspendedSize--;
        loList.moSuspended.Remove(apoServerNode);
    }

    if (loList.miActivatedSize + loList.miSuspendedSize == 0)
    {
        this->moAvailable.Remove(&loList);
    }

    return true;
}

bool CServerNodePriorityList::MoveToTail( CServerNode* apoServerNode )
{
    apl_int_t liPriority = apoServerNode->GetPriority();

    ACL_ASSERT(liPriority >= 0 && liPriority < 10);
    
    CListNode& loList = this->mpoPriList[liPriority];

    if (loList.moActivated.IsExisted(apoServerNode) )
    {
        loList.moActivated.Remove(apoServerNode);
        loList.moActivated.InsertTail(apoServerNode);
    }

    return true;
}

bool CServerNodePriorityList::Suspend( CServerNode* apoServerNode )
{
    apl_int_t liPriority = apoServerNode->GetPriority();

    ACL_ASSERT(liPriority >= 0 && liPriority < 10);
    
    CListNode& loList = this->mpoPriList[liPriority];
    
    if (loList.moActivated.IsExisted(apoServerNode) )
    {
        loList.miActivatedSize--;
        loList.miSuspendedSize++;

        loList.moActivated.Remove(apoServerNode);
        loList.moSuspended.InsertTail(apoServerNode);
    }

    return true;
}

bool CServerNodePriorityList::Resume( CServerNode* apoServerNode )
{
    apl_int_t liPriority = apoServerNode->GetPriority();

    ACL_ASSERT(liPriority >= 0 && liPriority < 10);
    
    CListNode& loList = this->mpoPriList[liPriority];
    
    if (loList.moSuspended.IsExisted(apoServerNode) )
    {
        loList.miActivatedSize++;
        loList.miSuspendedSize--;

        loList.moSuspended.Remove(apoServerNode);
        loList.moActivated.InsertTail(apoServerNode);
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
CServerNodeIterator::CServerNodeIterator( CPriorityListIterator& aoListIterator )
    : TRingIterator<CServerNode, 1, 0>(aoListIterator->moActivated)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
CPriorityListIterator::CPriorityListIterator( CServerNodePriorityList& aoList )
    : TRingIterator<CServerNodePriorityList::CListNode, 1, 0>(aoList.moAvailable)
{
}

AIBC_GFQ_NAMESPACE_END

