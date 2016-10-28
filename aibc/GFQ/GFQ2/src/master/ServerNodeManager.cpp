
#include "ServerNodeManager.h"
#include "MasterConfig.h"
#include "acl/Singleton.h"

AIBC_GFQ_NAMESPACE_START

CServerNodeManager::CServerNodeManager(void)
    : muServerNodeID(0)
    , muAllocateWeighting(5000)
{
}

CServerNodeManager::~CServerNodeManager(void)
{
    for (GroupMapType::iterator loIter = this->moServerNodeGroups.begin();
        loIter != this->moServerNodeGroups.end(); ++loIter)
    {
        IServerNodeGroup* lpoGroup = loIter->second;

        ACL_DELETE(lpoGroup);
    }

    for (NameMapType::iterator loNIter = this->moNameMap.begin();
        loNIter != this->moNameMap.end(); ++loNIter)
    {
        CServerNode* lpoServer = loNIter->second;
        ACL_DELETE(lpoServer);
    }
}

apl_int_t CServerNodeManager::AddServerNode(
    char const* apcName, 
    char const* apcGroupName,
    char const* apcStorageType,
    char const* apcServerIp, 
    apl_uint16_t ai16ServerPort,
    apl_int_t aiPriority,
    anf::SessionPtrType& aoSession )
{
    CServerNode* lpoServer = APL_NULL;

    acl::TSmartWLock<acl::CRWLock> loGuard(this->moLock);

    //Insert ServerNode into name map
    NameMapType::iterator loIter = this->moNameMap.find(apcName);
    if (loIter != this->moNameMap.end() )
    {
        return ERROR_NODE_EXISTED;
    }

    //Priority must by in [0, 10)
    aiPriority = aiPriority > 10 ? 9 : aiPriority < 0 ? 0 : aiPriority;

    ACL_NEW_ASSERT(lpoServer, 
        CServerNode(
            apcName, 
            apcGroupName,
            apcStorageType,
            this->muServerNodeID, 
            apcServerIp, 
            ai16ServerPort,
            aiPriority,
            this->muAllocateWeighting,
            aoSession) );

    this->moNameMap[apcName] = lpoServer;
    this->moIDMap[this->muServerNodeID++] = lpoServer;

    //Add server node into attacting node group
    IServerNodeGroup*& lpoGroup = this->moServerNodeGroups[apcGroupName];
    if (lpoGroup == APL_NULL)
    {
        lpoGroup = this->CreateServerNodeGroup();
    }

    lpoGroup->Add(lpoServer);

    return 0;
}

apl_int_t CServerNodeManager::DelServerNode( char const* apcName )
{
    acl::TSmartWLock<acl::CRWLock> loGuard(this->moLock);

    NameMapType::iterator loIter = this->moNameMap.find(apcName);
    if (loIter == this->moNameMap.end() )
    {
        return ERROR_NODE_UNEXISTED;
    }

    CServerNode* lpoServer = loIter->second;
    
    //Release the specifed ServerNode all resource
    IServerNodeGroup* lpoGroup = this->FindServerNodeGroup(lpoServer->GetGroupName() );
    if (lpoGroup != APL_NULL)
    {
        lpoGroup->Remove(lpoServer);
    }

    this->moIDMap.erase(lpoServer->GetServerNodeID() );
    this->moNameMap.erase(loIter);

    ACL_DELETE(lpoServer);

    return 0;
}

apl_int_t CServerNodeManager::AddFreeSegment( char const* apcName, apl_size_t auSegmentID, apl_size_t auCapacity )
{
    acl::TSmartRLock<acl::CRWLock> loGuard(this->moLock);

    CServerNode* lpoServerNode = this->FindServerNode(apcName);
    if (lpoServerNode == APL_NULL)
    {
        return ERROR_NODE_UNEXISTED;
    }

    return lpoServerNode->AddFreeSegment(auSegmentID, auCapacity);
}

apl_int_t CServerNodeManager::AddUsedSegment(
    char const* apcName, 
    apl_size_t auSegmentID,
    apl_size_t auCapacity,
    CSegmentHandle* apoHandle )
{
    apl_int_t liRetCode = 0;

    acl::TSmartRLock<acl::CRWLock> loGuard(this->moLock);

    CServerNode* lpoServerNode = this->FindServerNode(apcName);
    if (lpoServerNode == APL_NULL)
    {
        return ERROR_NODE_UNEXISTED;
    }

    if ( (liRetCode = lpoServerNode->AddUsedSegment(auSegmentID, auCapacity) ) != 0)
    {
        return liRetCode;
    }

    if (apoHandle != APL_NULL)
    {
        apoHandle->Set(lpoServerNode->GetServerNodeID(), auSegmentID);
    }

    return 0;
}

apl_int_t CServerNodeManager::ActivateServerNode( char const* apcName )
{
    acl::TSmartRLock<acl::CRWLock> loGuard(this->moLock);

    CServerNode* lpoServerNode = this->FindServerNode(apcName);
    if (lpoServerNode == APL_NULL)
    {
        return ERROR_NODE_UNEXISTED;
    }

    IServerNodeGroup* lpoGroup = this->FindServerNodeGroup(lpoServerNode->GetGroupName() );
    if (lpoGroup == APL_NULL)
    {
        return ERROR_GROUP_UNEXISTED;
    }

    lpoGroup->Activate(lpoServerNode);

    return 0;
}

CSegmentHandle CServerNodeManager::Allocate( std::string const& aoGroupName )
{
    acl::TSmartRLock<acl::CRWLock> loGuard(this->moLock);

    IServerNodeGroup* lpoGroup = this->FindServerNodeGroup(aoGroupName);
    if (lpoGroup != APL_NULL)
    {
        return lpoGroup->Allocate();
    }

    return CSegmentHandle();
}

void CServerNodeManager::Deallocate( CSegmentHandle aoHandle )
{
    acl::TSmartRLock<acl::CRWLock> loGuard(this->moLock);

    if (!aoHandle.IsInvalid() )
    {
        CServerNode* lpoServerNode = this->FindServerNode(aoHandle.GetServerNodeID() );
        if (lpoServerNode != APL_NULL)
        {
            IServerNodeGroup* lpoGroup = this->FindServerNodeGroup(lpoServerNode->GetGroupName() );
            ACL_ASSERT(lpoGroup != APL_NULL);
            
            return lpoGroup->Deallocate(lpoServerNode, aoHandle);
        }
    }
}

apl_int_t CServerNodeManager::GetSession( CSegmentHandle aoHandle, anf::SessionPtrType& aoSession )
{
    acl::TSmartRLock<acl::CRWLock> loGuard(this->moLock);

    if (!aoHandle.IsInvalid() )
    {
        CServerNode* lpoServerNode = this->FindServerNode(aoHandle.GetServerNodeID() );
        if (lpoServerNode != APL_NULL)
        {
            aoSession = lpoServerNode->GetSession();

            return 0;
        }
    }
    
    return ERROR_NODE_UNEXISTED;
}

void CServerNodeManager::SetAllocateWeighting( apl_size_t auWeighting )
{
    this->muAllocateWeighting = auWeighting;
}

apl_size_t CServerNodeManager::GetAllocateWeighting(void)
{
    return this->muAllocateWeighting;
}
    
void CServerNodeManager::GetServerNodeList( std::vector<std::string>& aoResult )
{
    acl::TSmartRLock<acl::CRWLock> loGuard(this->moLock);

    for (NameMapType::iterator loIter = this->moNameMap.begin();
         loIter != this->moNameMap.end(); ++loIter)
    {
        aoResult.push_back(loIter->first);
    }
}

apl_int_t CServerNodeManager::UpdateServerNodeStatInfo( char const* apcName, CServerNodeStatInfo const& aoStatInfo )
{
    CServerNode* lpoServerNode = APL_NULL;
    
    acl::TSmartRLock<acl::CRWLock> loGuard(this->moLock);

    if ( (lpoServerNode = this->FindServerNode(apcName) ) == APL_NULL)
    {
        return ERROR_NODE_UNEXISTED;
    }

    lpoServerNode->UpdateStatInfo(aoStatInfo);

    return 0;
}

apl_int_t CServerNodeManager::GetServerNodeStatInfo( char const* apcName, CServerNodeStatInfo& aoStatInfo )
{
    CServerNode* lpoServerNode = APL_NULL;
    
    acl::TSmartRLock<acl::CRWLock> loGuard(this->moLock);

    if ( (lpoServerNode = this->FindServerNode(apcName) ) == APL_NULL)
    {
        return ERROR_NODE_UNEXISTED;
    }
    
    lpoServerNode->GetStatInfo(aoStatInfo);

    return 0;
}

apl_size_t CServerNodeManager::GetServerNodeID( char const* apcName )
{
    CServerNode* lpoServerNode = APL_NULL;

    acl::TSmartRLock<acl::CRWLock> loGuard(this->moLock);

    if ( (lpoServerNode = this->FindServerNode(apcName) ) == APL_NULL)
    {
        return INVALID_ID;
    }
    
    return lpoServerNode->GetServerNodeID();
}

void CServerNodeManager::GetServerNodeInfo( CServerNode* apoServerNode, CServerNodeInfo& aoNodeInfo )
{
    aoNodeInfo.SetServerNodeID(apoServerNode->GetServerNodeID() );
    aoNodeInfo.SetServerNodeName(apoServerNode->GetServerNodeName() );
    aoNodeInfo.SetGroupName(apoServerNode->GetGroupName() );
    aoNodeInfo.SetStorageType(apoServerNode->GetStorageType() );
    aoNodeInfo.SetServerIp(apoServerNode->GetServerIp() );
    aoNodeInfo.SetServerPort(apoServerNode->GetServerPort() );
    aoNodeInfo.SetSegmentCount(apoServerNode->GetSegmentCount() );
    aoNodeInfo.SetUsedSegmentCount(apoServerNode->GetUsedSegmentCount() );
    aoNodeInfo.SetFreeSegmentCount(apoServerNode->GetFreeSegmentCount() );
    aoNodeInfo.SetTimestamp(apoServerNode->GetTimestamp() );
    apoServerNode->GetStatInfo(aoNodeInfo.GetStatInfo() );
}

apl_int_t CServerNodeManager::GetServerNodeInfo( char const* apcName, CServerNodeInfo& aoNodeInfo )
{
    CServerNode* lpoServerNode = APL_NULL;

    acl::TSmartRLock<acl::CRWLock> loGuard(this->moLock);

    if ( (lpoServerNode = this->FindServerNode(apcName) ) == APL_NULL)
    {
        return ERROR_NODE_UNEXISTED;
    }
    
    this->GetServerNodeInfo(lpoServerNode, aoNodeInfo);

    return 0;
}

apl_int_t CServerNodeManager::GetServerNodeInfo( apl_size_t auServerNodeID, CServerNodeInfo& aoNodeInfo )
{
    CServerNode* lpoServerNode = APL_NULL;

    acl::TSmartRLock<acl::CRWLock> loGuard(this->moLock);

    if ( (lpoServerNode = this->FindServerNode(auServerNodeID) ) == APL_NULL)
    {
        return ERROR_NODE_UNEXISTED;
    }
    
    this->GetServerNodeInfo(lpoServerNode, aoNodeInfo);
    
    return 0;
}

bool CServerNodeManager::IsInvalidServerNode( apl_size_t auServerNodeID )
{
    acl::TSmartRLock<acl::CRWLock> loGuard(this->moLock);

    if (this->FindServerNode(auServerNodeID) == APL_NULL)
    {
        return true;
    }
    else
    {
        return false;
    }
}

CSegmentHandle CServerNodeManager::Find( char const* apcName, apl_size_t auSegmentID )
{
    CServerNode* lpoServerNode = APL_NULL;

    if (*apcName == '\0' || auSegmentID == INVALID_ID)
    {
        //Return invalid id immediately
        return CSegmentHandle();
    }

    acl::TSmartRLock<acl::CRWLock> loGuard(this->moLock);

    if ( (lpoServerNode = this->FindServerNode(apcName) ) != APL_NULL)
    {
        return CSegmentHandle(lpoServerNode->GetServerNodeID(), auSegmentID);
    }
    else
    {
        return CSegmentHandle();
    }
}

CServerNode* CServerNodeManager::FindServerNode( apl_size_t auServerNodeID )
{
    IDMapType::iterator loIter = this->moIDMap.find(auServerNodeID);
    if (loIter != this->moIDMap.end() )
    {
        return loIter->second;
    }
    else
    {
        return APL_NULL;
    }
}

CServerNode* CServerNodeManager::FindServerNode( std::string const& aoName )
{
    NameMapType::iterator loIter = this->moNameMap.find(aoName);
    if (loIter != this->moNameMap.end() )
    {
        return loIter->second;
    }
    else
    {
        return APL_NULL;
    }
}

IServerNodeGroup* CServerNodeManager::FindServerNodeGroup( std::string const& aoName )
{
    GroupMapType::iterator loIter = this->moServerNodeGroups.find(aoName);
    if (loIter != this->moServerNodeGroups.end() )
    {
        return loIter->second;
    }
    else
    {
        return APL_NULL;
    }
}

IServerNodeGroup* CServerNodeManager::CreateServerNodeGroup(void)
{
    IServerNodeGroup* lpoGroup = APL_NULL;

    if (acl::Instance<CMasterConfig>()->moAllocateScheduling == "wlra")
    {
        ACL_NEW_ASSERT(lpoGroup, CServerNodeGroupWLRA);
    }
    else if (acl::Instance<CMasterConfig>()->moAllocateScheduling == "rr")
    {
        ACL_NEW_ASSERT(lpoGroup, CServerNodeGroupRRS);
    }
    else
    {
        bool unimplement_allocate_scheduling = false;

        ACL_ASSERT(unimplement_allocate_scheduling);
    }

    return lpoGroup;
}

CServerNodeManager* GetServerNodeManager(void)
{
    return acl::Instance<CServerNodeManager>();
}

AIBC_GFQ_NAMESPACE_END

