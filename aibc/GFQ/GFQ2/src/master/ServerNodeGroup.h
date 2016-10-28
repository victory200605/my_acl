
#ifndef AIBC_GFQ_SERVERNODEGROUP_H
#define AIBC_GFQ_SERVERNODEGROUP_H

#include "gfq2/Utility.h"
#include "ServerNode.h"
#include "SegmentHandle.h"
#include "ServerNodePriorityList.h"

AIBC_GFQ_NAMESPACE_START

/** 
 * @brief Allocating and deallocating all storage segment server node in the same group
 */
class IServerNodeGroup
{
public:
    virtual ~IServerNodeGroup(void) {};

    virtual void Add( CServerNode* apoServerNode ) = 0;
    
    virtual void Activate( CServerNode* apoServerNode ) = 0;

    virtual void Remove( CServerNode* apoServerNode ) = 0;

    virtual CSegmentHandle Allocate(void) = 0;

    virtual void Deallocate( CServerNode* apoServerNode, CSegmentHandle aoHandle ) = 0;
};

/** 
 * @brief Round-Robin Scheduling server node group manager
 */
class CServerNodeGroupRRS : public IServerNodeGroup
{
public:
    CServerNodeGroupRRS(void);

    ~CServerNodeGroupRRS(void);

    void Add( CServerNode* apoServerNode );
    
    void Activate( CServerNode* apoServerNode );

    void Remove( CServerNode* apoServerNode );
    
    CSegmentHandle Allocate(void);

    void Deallocate( CServerNode* apoServerNode, CSegmentHandle aoHandle );

private:
    CServerNodePriorityList moPriList;

    acl::CLock moLock;
};

/** 
 * @brief Weighted Least Recently Allocated Scheduling server node manager
 */
class CServerNodeGroupWLRA : public IServerNodeGroup
{
public:
    CServerNodeGroupWLRA(void);

    ~CServerNodeGroupWLRA(void);

    void Add( CServerNode* apoServerNode );
    
    void Activate( CServerNode* apoServerNode );

    void Remove( CServerNode* apoServerNode );
    
    CSegmentHandle Allocate(void);
    
    void Deallocate( CServerNode* apoServerNode, CSegmentHandle aoHandle );

private:
    CServerNodePriorityList moPriList;

    acl::CLock moLock;
};

AIBC_GFQ_NAMESPACE_END

#endif//GFQ_SERVERNODEMANAGER_H

