
#ifndef ANF_SESSIONMANAGER_H
#define ANF_SESSIONMANAGER_H

#include "acl/Synch.h"
#include "acl/stl/map.h"
#include "acl/stl/vector.h"
#include "anf/IoSession.h"

ANF_NAMESPACE_START

///////////////////////////////////////////////////////////////////////////////////////////////
class CSessionManager
{
    //begin of all
    struct CBeginPredication
    {
        template<typename T> bool operator () ( T const& t ) const { return true; }
    };

    //begin of specified group
    struct CGroupBeginPredication
    {
        template<typename T>
        bool operator ()( T aoFirst, T aoLast, anf::SessionPtrType& aoSession ) const
        {
            if (aoFirst != aoLast)
            {
                aoSession = aoFirst->GetSession();
                return true;
            }
            else
            {
                return false;
            }
        }
    };

    struct CNode
    {
        SessionPtrType& GetSession(void);
        
        char const* GetGroupName(void);

        apl_int_t GetRemark(void);

        void SetRemark( apl_int_t aiRemark );

        std::string moGroup;

        SessionPtrType moSession;
        
        apl_int_t miRemark;
    };
    
    struct CNodePointer
    {
        CNodePointer( CNode* apoNode = NULL );
        
        SessionPtrType& GetSession(void);
        
        char const* GetGroupName(void);

        apl_int_t GetRemark(void);

        void SetRemark( apl_int_t aiRemark );
        
        CNode* mpoNode;
    };

public:
    typedef std::vector<CNodePointer> GroupType;
    typedef std::map<std::string, GroupType > GroupMapType;
    typedef std::map<apl_uint64_t, CNode> SessionMapType;

public:
    ///predication
    static const CBeginPredication PRED_BEGIN;
    static const CGroupBeginPredication PRED_GROUP_BEGIN;
    
public:
    CSessionManager(void);
    
    bool Insert( std::string const& aoGroup, SessionPtrType& aoSession );
    
    bool Find( apl_uint64_t au64SessionID );
    
    bool Find( apl_uint64_t au64SessionID, SessionPtrType& aoSession );
    
    template<typename PredicationType>
    bool FindIf( PredicationType const& aoPred, SessionPtrType& aoSession );
    
    template<typename PredicationType>
    apl_size_t FindAllIf( PredicationType const& aoPred, std::vector<SessionPtrType>& aoSessions );

    template<typename PollType>
    bool FindIf( std::string const& aoGroup, PollType const& aoPoll, SessionPtrType& aoSession );
    
    void Erase( apl_uint64_t au64SessionID );
    
    void Erase( SessionPtrType& aoSession );

    bool GetGroupName( SessionPtrType& aoSession, std::string& aoGroupName );
    
    apl_size_t GetSize(void);
    
    apl_size_t GetSize( std::string const& aoGroup );
    
    void Clear(void);

protected:
    void EraseFromGroup( std::string const& aoGroup, SessionPtrType& aoSession );
    
private:
    acl::CLock moLock;
        
    GroupMapType moGroups;
    SessionMapType moSessions;
};

////////////////////////////////////////////////////////////////////////////////////////////
//implement
template<typename PredicationType>
bool CSessionManager::FindIf( PredicationType const& aoPred, SessionPtrType& aoSession )
{
    acl::TSmartLock<acl::CLock> loGuard(this->moLock);
    
    for ( SessionMapType::iterator loIter = this->moSessions.begin();
        loIter != this->moSessions.end(); ++loIter )
    {
        if (aoPred(&loIter->second) )
        {
            aoSession = loIter->second.moSession;
            return true;
        }
    }
    
    return false;
}

template<typename PredicationType>
apl_size_t CSessionManager::FindAllIf( PredicationType const& aoPred, std::vector<SessionPtrType>& aoSessions )
{
    acl::TSmartLock<acl::CLock> loGuard(this->moLock);
    
    for ( SessionMapType::iterator loIter = this->moSessions.begin();
        loIter != this->moSessions.end(); ++loIter )
    {
        if (aoPred(&loIter->second) )
        {
            aoSessions.push_back(loIter->second.moSession);
        }
    }
    
    return aoSessions.size();
}

template<typename PollType>
bool CSessionManager::FindIf( std::string const& aoGroup, PollType const& aoPoll, SessionPtrType& aoSession )
{
    acl::TSmartLock<acl::CLock> loGuard(this->moLock);
        
    GroupMapType::iterator loGroupIter = this->moGroups.find(aoGroup);
    if (loGroupIter == this->moGroups.end() )
    {
        return false;
    }

    return aoPoll(loGroupIter->second.begin(), loGroupIter->second.end(), aoSession);
}

ANF_NAMESPACE_END

#endif//ANF_SESSIONMANAGER_H
