
#ifndef AIBC_GFQUEUECLIENT_H
#define AIBC_GFQUEUECLIENT_H

#include "acl/TimeValue.h"
#include "acl/Timestamp.h"
#include "acl/MemoryBlock.h"
#include "acl/stl/string.h"
#include "acl/stl/map.h"
#include "acl/stl/vector.h"
#include "acl/SockAddr.h"
#include "gfq2/Utility.h"

AIBC_GFQ_NAMESPACE_START

class CGFQueueClientImpl;
class CQueuePollImpl;

/** 
 * @brief Queue info
 */
class CQueueInfo
{
public:
    GFQ_MEMBER_METHOD_CSTR(QueueName);
    GFQ_MEMBER_METHOD_U(QueueID);
    GFQ_MEMBER_METHOD_U(Size);
    GFQ_MEMBER_METHOD_U(Capacity);
    GFQ_MEMBER_METHOD_U(SegmentCount);

private:
    char macQueueName[APL_NAME_MAX];
    apl_size_t  muQueueID;
    apl_size_t  muSize;
    apl_size_t  muCapacity;
    apl_size_t  muSegmentCount;
};

/** 
 * @brief Segment server node info 
 */
class CServerNodeInfo
{
public:
    GFQ_MEMBER_METHOD_STR(ServerNodeName);
    GFQ_MEMBER_METHOD_STR(GroupName);
    GFQ_MEMBER_METHOD_STR(StorageType);
    GFQ_MEMBER_METHOD_U(ServerNodeID);
    GFQ_MEMBER_METHOD_STR(ServerIp);
    GFQ_MEMBER_METHOD_U16(ServerPort);
    GFQ_MEMBER_METHOD_I(ConnectionCount);
    GFQ_MEMBER_METHOD_U(SegmentCount);
    GFQ_MEMBER_METHOD_U(UsedSegmentCount);
    GFQ_MEMBER_METHOD_U(FreeSegmentCount);
    GFQ_MEMBER_METHOD_I64(Timestamp);
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
    std::string  moServerNodeName;
    std::string  moGroupName;
    std::string  moStorageType;
    apl_size_t   muServerNodeID;
    std::string  moServerIp;
    apl_uint16_t mu16ServerPort;
    apl_int_t    miConnectionCount;
    apl_size_t   muSegmentCount;
    apl_size_t   muUsedSegmentCount;
    apl_size_t   muFreeSegmentCount;
    apl_int64_t  mi64Timestamp;
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
 * @brief Global File Queue client api interface
 */
class CGFQueueClient
{
public:
    CGFQueueClient(void);

    ~CGFQueueClient(void);

    /** 
     * @brief Set session read buffer size
     * 
     * @param auBufferSize 
     */
    void SetReadBufferSize( apl_size_t auBufferSize );

    /** 
     * @brief Set session write buffer size
     * 
     * @param auBufferSize 
     */
    void SetWriteBufferSize( apl_size_t auBufferSize );

    /** 
     * @brief Set session message wind size
     * 
     * @param auWindSize 
     */
    void SetMessageWindSize( apl_size_t auWindSize );
    
    /** 
     * @brief Set error cache time, over @aoTimedout cache will disable
     * 
     * @param aoTimedout 
     */
    void SetErrorCacheTime( acl::CTimeValue const& aoTimedout );
        
    /** 
     * @brief Initialize GFQ client and connecting to server
     * 
     * @param apcName client identify name
     * @param aoRemoteAddress romote server address
     * @param auConnNum connection count
     * @param aoTimedout request timedout
     * @param aiOpt option if #IGNORE_CONNECT_FAIL set, then init shall be never fail when network broken
     * 
     * @returns if success 0 shell be return, otherwise < 0 return code shell be return  
     */
	apl_int_t Initialize(
        char const* apcName,
        acl::CSockAddr const& aoRemoteAddress, 
        apl_size_t auConnNum, 
        acl::CTimeValue const& aoTimedout,
        apl_int_t aiOpt = 0 );
    
    /** 
     * @brief Close GFQ client and release all resource
     */
    void Close(void);

    /** 
     * @brief Get/Pop record from Global File Queue by specified queue name
     * 
     * @param[in] apcQueueName
     * @param[out] apoData return data by std::string
     * 
     * @returns if success 0 shell be return, otherwise < 0 return code shell be return  
     */
	apl_int_t Get( const char* apcQueueName, std::string* apoData );
	
    /** 
     * @brief Get/Pop record from Global File Queue by specified queue name
     * 
     * @param[in] apcQueueName 
     * @param[out] apoData return data by acl::CMemoryBlock
     * 
     * @returns if success 0 shell be return, otherwise < 0 return code shell be return 
     */
    apl_int_t Get( const char* apcQueueName, acl::CMemoryBlock* apoData );

    /** 
     * @brief Get/Pop @auNumber records from Global File Queue by specified queue name
     * 
     * @param[in] apcQueueName
     * @param[in] auNumber max record count
     * @param[out] apoDatas return datas by std::vector<std::string>
     * 
     * @returns if success record count shell be return, otherwise < 0 return code shell be return 
     */
    apl_ssize_t Get( const char* apcQueueName, apl_size_t auNumber, std::vector<std::string>* apoDatas );

    /** 
     * @brief Get/Pop timedout record from Global File Queue by specified queue name and timedout value
     * 
     * @param apcQueueName 
     * @param aiTimedout record timedout value second
     * @param apoData return data by std::string
     * 
     * @returns if success 0 shell be return, otherwise < 0 return code shell be return
     */
    apl_int_t GetTimedout( const char* apcQueueName, apl_int_t aiTimedout, std::string* apoData );
    
    /** 
     * @brief Get/Pop timedout record from Global File Queue by specified queue name and timedout value
     * 
     * @param apcQueueName 
     * @param aiTimedout record timedout value second
     * @param apoData return data by acl::CMemoryBlock
     * 
     * @returns if success 0 shell be return, otherwise < 0 return code shell be return
     */
    apl_int_t GetTimedout( const char* apcQueueName, apl_int_t aiTimedout, acl::CMemoryBlock* apoData );
    
    /** 
     * @brief Get/Pop @auNumber timedout records from Global File Queue by specified queue name and timedout value
     * 
     * @param apcQueueName 
     * @param auNumber max record number
     * @param aiTimedout record timedout value second
     * @param apoData return datas by std::vector<std::string>
     * 
     * @returns if success record count shell be return, otherwise < 0 return code shell be return
     */
    apl_ssize_t GetTimedout( const char* apcQueueName, apl_size_t auNumber, apl_int_t aiTimedout, std::vector<std::string>* apoData );
	
    /** 
     * @brief Put/Push record into Global File Queue by specified queue name
     * 
     * @param apcQueueName 
     * @param apoBuffer data buffer pointer
     * @param auLength data buffer length
     * 
     * @returns if success 0 shell be return, otherwise < 0 return code shell be return 
     */
	apl_int_t Put( const char* apcQueueName, char const* apoBuffer, apl_size_t auLenght );
	
    /** 
     * @brief Put/Push record into Global File Queue by specified queue name
     * 
     * @param apcQueueName 
     * @param aoData std::string
     * 
     * @returns if success 0 shell be return, otherwise < 0 return code shell be return 
     */
	apl_int_t Put( const char* apcQueueName, std::string const& aoData );
	
    /** 
     * @brief Put/Push record into Global File Queue by specified queue name
     * 
     * @param apcQueueName 
     * @param aoData acl::CMemoryBlock
     * 
     * @returns if success 0 shell be return, otherwise < 0 return code shell be return 
     */
	apl_int_t Put( const char* apcQueueName, acl::CMemoryBlock const& aoData );
	
    /** 
     * @brief Get queue info by specified queue name
     * 
     * @param apcQueueName 
     * @param aoInfo 
     * 
     * @returns if success 0 shell be return, otherwise < 0 return code shell be return
     */
    apl_int_t GetQueueInfo( char const* apcQueueName, CQueueInfo& aoInfo );
    
    /** 
     * @brief Get queue info by specified queue id
     * 
     * @param auQueueID 
     * @param aoInfo 
     * 
     * @returns if success 0 shell be return, otherwise < 0 return code shell be return
     */
    apl_int_t GetQueueInfo( apl_size_t auQueueID, CQueueInfo& aoInfo );
    
    /** 
     * @brief Get all queue info in Global File Queue
     * 
     * @param aoInfos 
     * 
     * @returns if success 0 shell be return, otherwise < 0 return code shell be return 
     */
    apl_int_t GetAllQueueInfo( std::vector<CQueueInfo>& aoInfos );

    /** 
     * @brief Get server node info by specified server node name
     * 
     * @param apcNodeName 
     * @param aoInfo 
     * 
     * @returns if success 0 shell be return, otherwise < 0 return code shell be return
     */
    apl_int_t GetServerNodeInfo( char const* apcNodeName, CServerNodeInfo& aoInfo );
    
    /** 
     * @brief Get all server node info in Global File Queue
     * 
     * @param aoInfos 
     * 
     * @returns if success 0 shell be return, otherwise < 0 return code shell be return
     */
    apl_int_t GetAllServerNodeInfo( std::vector<CServerNodeInfo>& aoInfos );

    /** 
     * @brief Get master server startup timestamp
     * 
     * @param aoTimestamp timestamp
     * 
     * @returns if success 0 shell be return, otherwise < 0 return code shell be return
     */
    apl_int_t GetStartupTimestamp( acl::CTimestamp& aoTimestamp );
    
    /** 
     * @brief Bind node group and segment allocate will from specified bind group
     * 
     * @param apcQueueName
     * @param apcGroupName 
     * 
     * @returns if success 0 shell be return, otherwise < 0 return code shell be return
     */
    apl_int_t BindNodeGroup( char const* apcQueueName, char const* apcGroupName );

    CGFQueueClientImpl* GetImpl(void);

private:
    CGFQueueClientImpl* mpoImpl;
};

//////////////////////////////////////////////////////////////////////////////////////////
/** 
 * @brief Queue pollset
 */
class CQueuePoll
{
public:
    CQueuePoll( CGFQueueClient& aoClient );

    ~CQueuePoll(void);

    /** 
     * @brief Add queue name @apcQueueName into poll-set
     * 
     * @param apcQueueName
     * 
     * @returns if success 0 shell be return, otherwise < 0 return code shell be return 
     */
    void Add( char const* apcQueueName );

    /** 
     * @brief Remove queue name @apcQueueName from poll-set
     * 
     * @param apcQueueName 
     * 
     * @returns if success 0 shell be return, otherwise < 0 return code shell be return  
     */
    void Remove( char const* apcQueueName );

    /** 
     * @brief Set poll result cache timedout, over @aoTimedout cache will disable
     * 
     * @param aoTimedout 
     */
    void SetResultCacheTime( acl::CTimeValue const& aoTimedout );

    /** 
     * @brief Wait for any pending readable events of all queue in the pollset.
     * 
     * @param apoResult poll result-set
     * @param aoTimedout poll timedout
     * 
     * @returns pengding readable queue size 
     */
    apl_ssize_t Poll( acl::CTimeValue const& aoTimedout, std::vector<std::string>* apoResult );

    CGFQueueClient* GetClient(void);

    CQueuePollImpl* GetImpl(void);

private:
    CQueuePollImpl* mpoImpl;

    CGFQueueClient* mpoClient;
};

AIBC_GFQ_NAMESPACE_END

#endif //AIBC_GFQUEUECLIENT_H
