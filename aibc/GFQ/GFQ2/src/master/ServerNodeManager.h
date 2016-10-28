
#ifndef AIBC_GFQ_SERVERNODEMANAGER_H
#define AIBC_GFQ_SERVERNODEMANAGER_H

#include "gfq2/Utility.h"
#include "ServerNode.h"
#include "SegmentHandle.h"
#include "ServerNodeGroup.h"

AIBC_GFQ_NAMESPACE_START

/** 
 * @brief Manage all storage segment server node, include node register and unregister,
 * and storage segments allocated and deallocated
 */
class CServerNodeManager
{
public:
    typedef std::map<std::string, CServerNode*> NameMapType;
    typedef std::map<std::string, IServerNodeGroup*> GroupMapType;
    typedef std::map<apl_size_t, CServerNode*> IDMapType;

public:
    CServerNodeManager(void);

    virtual ~CServerNodeManager(void);

    apl_int_t AddServerNode(
        char const* apcName,
        char const* apcGroupName,
        char const* apcStorageType,
        char const* apcServerIp,
        apl_uint16_t ai16ServerPort,
        apl_int_t aiPriority,
        anf::SessionPtrType& aoSession );

    apl_int_t DelServerNode( char const* apcName );

    apl_int_t AddFreeSegment(
        char const* apcName, 
        apl_size_t auSegmentID, 
        apl_size_t auCapacity );

    apl_int_t AddUsedSegment(
        char const* apcName,
        apl_size_t auSegmentID,
        apl_size_t auCapacity,
        CSegmentHandle* apoHandle );

    apl_int_t ActivateServerNode( char const* apcName );

    CSegmentHandle Allocate( std::string const& aoGroupName );
    
    void Deallocate( CSegmentHandle aoHandle );

    apl_int_t GetSession( CSegmentHandle aoHandle, anf::SessionPtrType& aoSession );

    apl_int_t UpdateServerNodeStatInfo(
        char const* apcName,
        CServerNodeStatInfo const& aoStatInfo );
    
    CSegmentHandle Find( char const* apcName, apl_size_t auSegmentID );

    bool IsInvalidServerNode( apl_size_t auServerNodeID );
    
    void SetAllocateWeighting( apl_size_t auWeighting );

    apl_size_t GetAllocateWeighting(void);

// Attribute method
    apl_size_t GetServerNodeID( char const* apcName );
    
    apl_int_t GetServerNodeStatInfo( char const* apcName, CServerNodeStatInfo& aoStatInfo );

    void GetServerNodeList( std::vector<std::string>& aoResult );

    apl_int_t GetServerNodeInfo( char const* apcName, CServerNodeInfo& aoNodeInfo );
    
    apl_int_t GetServerNodeInfo( apl_size_t auServerNodeID, CServerNodeInfo& aoNodeInfo );

protected:
    void GetServerNodeInfo( CServerNode* apoServerNode, CServerNodeInfo& aoNodeInfo );

    CServerNode* FindServerNode( apl_size_t auServerNodeID );
    
    CServerNode* FindServerNode( std::string const& aoName );
    
    IServerNodeGroup* FindServerNodeGroup( std::string const& aoName );
    
    IServerNodeGroup* CreateServerNodeGroup(void);

private:
    apl_size_t muServerNodeID;

    apl_size_t muAllocateWeighting;

    NameMapType moNameMap;

    IDMapType   moIDMap;

    GroupMapType moServerNodeGroups;

    acl::CRWLock moLock;
};

//singleton server node manager
CServerNodeManager* GetServerNodeManager(void);

AIBC_GFQ_NAMESPACE_END

#endif//GFQ_SERVERNODEMANAGER_H

