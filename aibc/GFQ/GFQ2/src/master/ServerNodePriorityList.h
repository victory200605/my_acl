
#ifndef AIBC_GFQ_SERVER_NODE_PRIORITY_LIST_H
#define AIBC_GFQ_SERVER_NODE_PRIORITY_LIST_H

#include "gfq2/Utility.h"
#include "ServerNode.h"

AIBC_GFQ_NAMESPACE_START

class CPriorityListIterator;

class CServerNodePriorityList
{
public:
    struct CListNode : public TRingEntry<1>
    {
        apl_int_t miActivatedSize;
        
        apl_int_t miSuspendedSize;

        TRing<1, 0> moActivated;
        
        TRing<1, 0> moSuspended;
    };

    friend class CPriorityListIterator;

public:
    CServerNodePriorityList(void);

    ~CServerNodePriorityList(void);
    
    bool Insert( CServerNode* apoServerNode );

    bool Remove( CServerNode* apoServerNode );

    bool MoveToTail( CServerNode* apoServerNode );

    bool Suspend( CServerNode* apoServerNode );
    
    bool Resume( CServerNode* apoServerNode );

private:
    CListNode* mpoPriList;

    TRing<1, 0> moAvailable;
};

class CServerNodeIterator : public TRingIterator<CServerNode, 1, 0>
{
public:
    CServerNodeIterator( CPriorityListIterator& aoListIterator );
};

class CPriorityListIterator : public TRingIterator<CServerNodePriorityList::CListNode, 1, 0>
{
public:
    CPriorityListIterator( CServerNodePriorityList& aoList );
};

AIBC_GFQ_NAMESPACE_END

#endif//GFQ_SERVER_NODE_PRIORITY_LIST_H

