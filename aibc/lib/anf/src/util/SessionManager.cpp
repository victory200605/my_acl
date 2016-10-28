
#include "anf/util/SessionManager.h"

ANF_NAMESPACE_START

const CSessionManager::CBeginPredication CSessionManager::PRED_BEGIN = CSessionManager::CBeginPredication();
const CSessionManager::CGroupBeginPredication CSessionManager::PRED_GROUP_BEGIN 
    = CSessionManager::CGroupBeginPredication();

////////////////////////////////////////////////////////////////////////////////////////////////////////////
SessionPtrType& CSessionManager::CNode::GetSession(void)
{
    return this->moSession;
}

char const* CSessionManager::CNode::GetGroupName(void)
{
    return this->moGroup.c_str();
}

apl_int_t CSessionManager::CNode::GetRemark(void)
{
    return this->miRemark;
}

void CSessionManager::CNode::SetRemark( apl_int_t aiRemark )
{
    this->miRemark = aiRemark;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
CSessionManager::CNodePointer::CNodePointer( CNode* apoNode )
    : mpoNode(apoNode)
{
}

SessionPtrType& CSessionManager::CNodePointer::GetSession(void)
{
    return this->mpoNode->moSession;
}

char const* CSessionManager::CNodePointer::GetGroupName(void)
{
    return this->mpoNode->moGroup.c_str();
}

apl_int_t CSessionManager::CNodePointer::GetRemark(void)
{
    return this->mpoNode->miRemark;
}

void CSessionManager::CNodePointer::SetRemark( apl_int_t aiRemark )
{
    this->mpoNode->miRemark = aiRemark;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
CSessionManager::CSessionManager(void)
{
}
    
bool CSessionManager::Insert( std::string const& aoGroup, SessionPtrType& aoSession )
{
    acl::TSmartLock<acl::CLock> loGuard(this->moLock);

    CNode& loNode = this->moSessions[aoSession->GetSessionID()];
    
    if (loNode.moSession != NULL)
    {
        return false;
    }
    
    loNode.moGroup = aoGroup;
    loNode.moSession = aoSession;
    
    //Insert group list
    GroupType& loGroup = this->moGroups[aoGroup];
    
    loGroup.push_back( CNodePointer(&loNode) );
    
    return true;
}

bool CSessionManager::Find( apl_uint64_t au64SessionID )
{
    acl::TSmartLock<acl::CLock> loGuard(this->moLock);
        
    SessionMapType::iterator loIter = this->moSessions.find(au64SessionID);
    if (loIter != this->moSessions.end() )
    {
        return true;
    }
    else
    {
        return false;
    }
}
    
bool CSessionManager::Find( apl_uint64_t au64SessionID, SessionPtrType& aoSession )
{
    acl::TSmartLock<acl::CLock> loGuard(this->moLock);
        
    SessionMapType::iterator loIter = this->moSessions.find(au64SessionID);
    if (loIter != this->moSessions.end() )
    {
        aoSession = loIter->second.moSession;
        return true;
    }
    else
    {
        return false;
    }
}
    
void CSessionManager::Erase( apl_uint64_t au64SessionID )
{
    acl::TSmartLock<acl::CLock> loGuard(this->moLock);
        
    SessionMapType::iterator loIter = this->moSessions.find(au64SessionID);
    if (loIter != this->moSessions.end() )
    {
        this->EraseFromGroup(loIter->second.moGroup, loIter->second.moSession);
        
        this->moSessions.erase(loIter);
    }
}

void CSessionManager::Erase( SessionPtrType& aoSession )
{
    this->Erase(aoSession->GetSessionID() );
}

bool CSessionManager::GetGroupName( SessionPtrType& aoSession, std::string& aoGroupName )
{
    acl::TSmartLock<acl::CLock> loGuard(this->moLock);
        
    SessionMapType::iterator loIter = this->moSessions.find(aoSession->GetSessionID() );
    if (loIter != this->moSessions.end() )
    {
        aoGroupName = loIter->second.moGroup;

        return true;
    }
    
    return false;
}

apl_size_t CSessionManager::GetSize(void)
{
    return this->moSessions.size();
}
    
apl_size_t CSessionManager::GetSize( std::string const& aoGroup )
{
    acl::TSmartLock<acl::CLock> loGuard(this->moLock);
    
    GroupMapType::iterator loGroupIter = this->moGroups.find(aoGroup);

    if (loGroupIter != this->moGroups.end() )
    {
        return loGroupIter->second.size();
    }
    else
    {
        return 0;
    }
}

void CSessionManager::Clear(void)
{
    acl::TSmartLock<acl::CLock> loGuard(this->moLock);
        
    this->moGroups.clear();
    this->moSessions.clear();
}

void CSessionManager::EraseFromGroup( std::string const& aoGroup, SessionPtrType& aoSession )
{
    GroupMapType::iterator loGroupIter = this->moGroups.find(aoGroup);
    
    if (loGroupIter != this->moGroups.end() )
    {
        for (GroupType::iterator loIter = loGroupIter->second.begin();
            loIter != loGroupIter->second.end(); ++loIter)
        {
            if (loIter->GetSession()->GetSessionID() == aoSession->GetSessionID() )
            {
                loGroupIter->second.erase(loIter);
                break;
            }
        }
    }
}

ANF_NAMESPACE_END
