
#include "gfq2/GFQMessage.h"
#include "ServerNode.h"
#include "acl/IndexDict.h"

AIBC_GFQ_NAMESPACE_START

///////////////////////////////////////////////////////////////////////////////////////////////////////////
CServerNode::CServerNode(
    char const* apcName,
    char const* apcGroupName,
    char const* apcStorageType,
    apl_size_t auServerID, 
    char const* apcServerIp, 
    apl_uint16_t au16ServerPort,
    apl_int_t aiPriority,
    apl_int_t aiAllocateWeighting,
    anf::SessionPtrType& aoSession )
    : moServerNodeName(apcName)
    , moGroupName(apcGroupName)
    , moStorageType(apcStorageType)
    , muServerNodeID(auServerID)
    , moServerIp(apcServerIp)
    , mu16ServerPort(au16ServerPort)
    , miPriority(aiPriority)
    , miAllocateWeighting(aiAllocateWeighting)
    , moSession(aoSession)
{
    this->moTimestamp.Update();
}

CServerNode::~CServerNode(void)
{
    this->moSession->Close(this->moSession);
}

apl_int_t CServerNode::AddSegment( apl_size_t auSegmentID, apl_size_t auCapacity )
{
    if (auSegmentID == INVALID_ID)
    {
        return ERROR_INVALID_SEGMENT;
    }

    //Insert used segment by segment id
    if (auSegmentID == this->moSegments.size() )
    {
        this->moSegments.push_back(CSegment(auSegmentID, auCapacity) );
    }
    else
    {
        if (auSegmentID > this->moSegments.size() )
        {
            //Pading empty segment, is unnormal, maybe assert here better.
            this->moSegments.resize(auSegmentID + 1);
        }

        this->moSegments[auSegmentID].muSegmentID = auSegmentID;
        this->moSegments[auSegmentID].muCapacity  = auCapacity;
    }

    return 0;
}

apl_int_t CServerNode::AddFreeSegment( apl_size_t auSegmentID, apl_size_t auCapacity )
{
    apl_int_t liRetCode = 0;
    
    acl::TSmartLock<acl::CLock> loGuard(this->moLock);

    if ( (liRetCode = this->AddSegment(auSegmentID, auCapacity) ) != 0)
    {
        return liRetCode;
    }

    this->moFreeSegments.push_back(auSegmentID);

    return 0;
}

apl_int_t CServerNode::AddUsedSegment( apl_size_t auSegmentID, apl_size_t auCapacity )
{
    acl::TSmartLock<acl::CLock> loGuard(this->moLock);
    
    return this->AddSegment(auSegmentID, auCapacity);
}

apl_size_t CServerNode::Allocate(void)
{
    acl::TSmartLock<acl::CLock> loGuard(this->moLock);

    if (this->moFreeSegments.size() > 0)
    {
        apl_size_t luSegmentID = this->moFreeSegments.front();
        this->moFreeSegments.pop_front();

        //Increase allocate weighting value
        this->muTotalAllocateWeighting += this->miAllocateWeighting;
        
        return luSegmentID;
    }
    else
    {
        return INVALID_ID;
    }
}

apl_int_t CServerNode::Deallocate( apl_size_t auSegmentID )
{
    if (auSegmentID == INVALID_ID)
    {
        return ERROR_INVALID_SEGMENT;
    }

    acl::TSmartLock<acl::CLock> loGuard(this->moLock);

    this->moFreeSegments.push_back(auSegmentID);

    return 0;
}

void CServerNode::UpdateStatInfo( CServerNodeStatInfo const& aoStatInfo )
{
    this->moStatInfo = aoStatInfo;
    this->muTotalAllocateWeighting = 0;
}

void CServerNode::GetStatInfo( CServerNodeStatInfo& aoStatInfo )
{
    aoStatInfo = this->moStatInfo;
}

apl_size_t CServerNode::GetAccessedTPS(void)
{
    return this->moStatInfo.GetReadTimes() + this->moStatInfo.GetWriteTimes() + this->muTotalAllocateWeighting;
}

char const* CServerNode::GetServerNodeName(void) const
{
    return this->moServerNodeName.c_str();
}

std::string const& CServerNode::GetGroupName(void) const
{
    return this->moGroupName;
}

char const* CServerNode::GetStorageType(void) const
{
    return this->moStorageType.c_str();
}

apl_size_t CServerNode::GetServerNodeID(void) const
{
    return this->muServerNodeID;
}

char const* CServerNode::GetServerIp(void) const
{
    return this->moServerIp.c_str();
}

apl_uint16_t CServerNode::GetServerPort(void) const
{
    return this->mu16ServerPort;
}

apl_size_t CServerNode::GetSegmentCount(void) const
{
    return this->moSegments.size();
}

apl_size_t CServerNode::GetFreeSegmentCount(void) const
{
    return this->moFreeSegments.size();
}

apl_size_t CServerNode::GetUsedSegmentCount(void) const
{
    return this->moSegments.size() - this->moFreeSegments.size();
}

apl_time_t CServerNode::GetTimestamp(void) const
{
    return this->moTimestamp.Nsec();
}

apl_int_t CServerNode::GetPriority(void) const
{
    return this->miPriority;
}

anf::SessionPtrType& CServerNode::GetSession(void)
{
    return this->moSession;
}

AIBC_GFQ_NAMESPACE_END

