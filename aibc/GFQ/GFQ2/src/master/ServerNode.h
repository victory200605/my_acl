
#ifndef AIBC_GFQ_SERVERNODE_H
#define AIBC_GFQ_SERVERNODE_H

#include "gfq2/Utility.h"
#include "acl/Timestamp.h"
#include "anf/IoSession.h"
#include "Ring.h"
#include "acl/stl/string.h"
#include "acl/stl/deque.h"
#include "acl/stl/vector.h"

AIBC_GFQ_NAMESPACE_START

/** 
 * @brief Storage segment node info
 */
class CSegmentInfo
{
public:
    GFQ_MEMBER_METHOD_U(SegmentID);
    GFQ_MEMBER_METHOD_STR(ServerNodeName);
    GFQ_MEMBER_METHOD_STR(ServerIp);
    GFQ_MEMBER_METHOD_U16(ServerPort);
    GFQ_MEMBER_METHOD_I64(Timestamp);
    GFQ_MEMBER_METHOD_U(Capacity);
    GFQ_MEMBER_METHOD_U(Size);

private:
    apl_size_t   muSegmentID;
    std::string  moServerNodeName;
    std::string  moServerIp;
    apl_uint16_t mu16ServerPort;
    apl_int64_t  mi64Timestamp;
    apl_size_t   muCapacity;
    apl_size_t   muSize;
};

/** 
 * @brief Server node load info
 */
class CServerNodeStatInfo
{
public:
    CServerNodeStatInfo(void) 
        : muConnectionCount(0)
        , mu64ReadTimes(0)
        , mu64WriteTimes(0) 
        , muFreePageCount(0)
        , muUsedPageCount(0)
        , muCacheFreePageCount(0)
        , muCacheUsedPageCount(0)
        , mu64CacheReadTimes(0)
        , mu64CacheReadMissedTimes(0)
        , mu64CacheWriteTimes(0)
        , mu64CacheWriteMissedTimes(0)
        , mu64CacheSyncTimes(0)
    {
    }

    GFQ_MEMBER_METHOD_U(ConnectionCount);
    GFQ_MEMBER_METHOD_U64(ReadTimes);
    GFQ_MEMBER_METHOD_U64(WriteTimes);
    GFQ_MEMBER_METHOD_U(FreePageCount);
    GFQ_MEMBER_METHOD_U(UsedPageCount);
    GFQ_MEMBER_METHOD_U(CacheFreePageCount);
    GFQ_MEMBER_METHOD_U(CacheUsedPageCount);
    GFQ_MEMBER_METHOD_U64(CacheReadTimes);
    GFQ_MEMBER_METHOD_U64(CacheReadMissedTimes);
    GFQ_MEMBER_METHOD_U64(CacheWriteTimes);
    GFQ_MEMBER_METHOD_U64(CacheWriteMissedTimes);
    GFQ_MEMBER_METHOD_U64(CacheSyncTimes);

private:
    apl_size_t   muConnectionCount;
    apl_uint64_t mu64ReadTimes;
    apl_uint64_t mu64WriteTimes;
    apl_size_t   muFreePageCount;
    apl_size_t   muUsedPageCount;
    apl_size_t   muCacheFreePageCount;
    apl_size_t   muCacheUsedPageCount;
    apl_uint64_t mu64CacheReadTimes;
    apl_uint64_t mu64CacheReadMissedTimes;
    apl_uint64_t mu64CacheWriteTimes;
    apl_uint64_t mu64CacheWriteMissedTimes;
    apl_uint64_t mu64CacheSyncTimes;
};

/** 
 * @brief Segment server node info 
 */
class CServerNodeInfo
{
public:
    GFQ_MEMBER_METHOD_U(ServerNodeID);
    GFQ_MEMBER_METHOD_STR(ServerNodeName);
    GFQ_MEMBER_METHOD_STR(GroupName);
    GFQ_MEMBER_METHOD_STR(StorageType);
    GFQ_MEMBER_METHOD_STR(ServerIp);
    GFQ_MEMBER_METHOD_U16(ServerPort);
    GFQ_MEMBER_METHOD_U(SegmentCount);
    GFQ_MEMBER_METHOD_U(UsedSegmentCount);
    GFQ_MEMBER_METHOD_U(FreeSegmentCount);
    GFQ_MEMBER_METHOD_I64(Timestamp);

    CServerNodeStatInfo& GetStatInfo(void)
    {
        return this->moStatInfo;
    }

private:
    apl_size_t   muServerNodeID;
    std::string  moServerNodeName;
    std::string  moGroupName;
    std::string  moStorageType;
    std::string  moServerIp;
    apl_uint16_t mu16ServerPort;
    apl_size_t   muSegmentCount;
    apl_size_t   muUsedSegmentCount;
    apl_size_t   muFreeSegmentCount;
    apl_int64_t  mi64Timestamp;
    CServerNodeStatInfo moStatInfo;
};

/** 
 * @brief Segment server node is remote storage server, provite storage segment for queue, include segment allocated and deallocated 
 */
class CServerNode : public TRingEntry<1>
{
    struct CSegment
    {
        CSegment( apl_size_t auID = INVALID_ID, apl_size_t auCapacity = 0 )
            : muSegmentID(auID)
            , muCapacity(auCapacity)
        {
        }

        apl_size_t  muSegmentID;
        apl_size_t  muCapacity;
    };

public:
    CServerNode( 
        char const* apcName, 
        char const* apcGroupName, 
        char const* apcStorageType, 
        apl_size_t auServerID, 
        char const* apcServerIp, 
        apl_uint16_t au16ServerPort,
        apl_int_t aiPriority,
        apl_int_t aiAllocateWeighting,
        anf::SessionPtrType& aoSession );

    ~CServerNode(void);

    apl_int_t AddFreeSegment( apl_size_t auSegmentID, apl_size_t auCapacity );

    apl_int_t AddUsedSegment( apl_size_t auSegmentID, apl_size_t auCapacity );

    apl_size_t Allocate(void);

    apl_int_t Deallocate( apl_size_t auSegmentID );

    void UpdateStatInfo( CServerNodeStatInfo const& aoStatInfo );
    
//Attribute method
    void GetStatInfo( CServerNodeStatInfo& aoStatInfo );

    apl_size_t GetAccessedTPS(void);

    char const* GetServerNodeName(void) const;
    
    std::string const& GetGroupName(void) const;

    char const* GetStorageType(void) const;
    
    apl_size_t GetServerNodeID(void) const;

    char const* GetServerIp(void) const;

    apl_uint16_t GetServerPort(void) const;

    apl_size_t GetSegmentCount(void) const;

    apl_size_t GetFreeSegmentCount(void) const;

    apl_size_t GetUsedSegmentCount(void) const;

    apl_time_t GetTimestamp(void) const;

    apl_int_t GetPriority(void) const;
    
    anf::SessionPtrType& GetSession(void);

protected:
    apl_int_t AddSegment( apl_size_t auSegmentID, apl_size_t auCapacity );

private:
    std::string  moServerNodeName;
    std::string  moGroupName;
    std::string  moStorageType;
    apl_size_t   muServerNodeID;
    std::string  moServerIp;
    apl_uint16_t mu16ServerPort;

    apl_int_t miPriority;
    
    apl_int_t miAllocateWeighting;
    
    apl_size_t muTotalAllocateWeighting;
    
    acl::CTimestamp moTimestamp;

    CServerNodeStatInfo moStatInfo;

    std::vector<CSegment>   moSegments;
    std::deque<apl_size_t>  moFreeSegments;
    
    anf::SessionPtrType moSession;
    
    acl::CLock moLock;
};

AIBC_GFQ_NAMESPACE_END

#endif//GFQ_SERVERNODE_H

