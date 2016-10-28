#ifndef AIBC_GFQ2_CLIENT_IMPL_H
#define AIBC_GFQ2_CLIENT_IMPL_H

#include "gfq2/GFQMessage.h"
#include "gfq2/GFQueueClient.h"
#include "anf/util/AsyncClient.h"
#include "anf/filter/BencodingMessageFilter.h"
#include "anf/IoSimpleProcessorPool.h"
#include "anf/IoPollingProcessor.h"
#include "acl/Synch.h"
#include "acl/Timestamp.h"
#include "acl/BitArray.h"
#include "acl/Process.h"
#include "acl/SockDgram.h"
#include "acl/SString.h"

AIBC_GFQ_NAMESPACE_START

typedef anf::TAsyncClient<acl::CIndexDict> ClientType;

/** 
 * @brief Asynch client support Mcast for master server
 */
class CMasterClient : public ClientType
{
public:
    CMasterClient(void);

    apl_int_t Connect( acl::CSockAddr const& aoRemoteAddress );

    void GetRemoteAddress( acl::CSockAddr* apoRemoteAddress, apl_int_t aiTryTimes );

    virtual void HandleConnectException(
        char const* apcName,
        acl::CSockAddr const& aoRemoteAddress,
        apl_int_t aiState,
        apl_int_t aiErrno );

protected:
    bool mbIsMcastAddr;
    
    acl::CSockDgram moDgram;
    
    acl::CSockAddr moRemoteAddress;
};

/** 
 * @brief Error cache to reduce repeat request times
 */
class CErrorCache
{
public:
    CErrorCache(void);

    bool IsHitCache( acl::CTimeValue const& aoTimedout );
    
    apl_int_t GetErrno(void);

    void Update( apl_int_t aiErrno );

private:
    apl_int_t  miErrno;

    acl::CTimestamp moTimestamp;
};

/** 
 * @brief Global File Queue client implement
 */
class CGFQueueClientImpl
{
    // read & write segment info cache node, will be saved in hashtable
    struct CSegmentInfo
    {
        CSegmentInfo(void)
            : muSegmentID(INVALID_ID)
            , mi64Timestamp(-1)
            , mpoSegmentServer(NULL)
        {
        }
        
        apl_size_t   muSegmentID;
        char         macServerNodeName[APL_NAME_MAX];
        apl_int64_t  mi64Timestamp;
        char         macQueueName[APL_NAME_MAX];
        ClientType*  mpoSegmentServer;
        CErrorCache  moErrorCache;
    };

    typedef std::map<std::string, ClientType*> ServerMapType;
    typedef anf::TConcurrentMap<std::string, CSegmentInfo, acl::CLock>  SegmentMapType;

public:
    enum { DEAD_CIRCLE_NUM = 1001 }; // 

    
public:
	CGFQueueClientImpl(void);

	~CGFQueueClientImpl(void);

    void SetReadBufferSize( apl_size_t auBufferSize );

    void SetWriteBufferSize( apl_size_t auBufferSize );

    void SetMessageWindSize( apl_size_t auWindSize );
        
    void SetErrorCacheTime( acl::CTimeValue const& aoTimedout );
	
	apl_int_t Initialize(
        char const* apcName,
        acl::CSockAddr const& aoRemoteAddress, 
        apl_size_t auConnNum, 
        acl::CTimeValue const& aoTimedout,
        apl_int_t aiOpt );
    
    void Close(void);

	apl_int_t Get( const char* apcQueueName, acl::CIndexDict& aoRequest, acl::CIndexDict** appoResponse );

	apl_int_t Get( const char* apcQueueName, std::string* apoData );

	apl_int_t Get( const char* apcQueueName, acl::CMemoryBlock* apoData );

    apl_ssize_t Get( const char* apcQueueName, apl_size_t auNumber, std::vector<std::string>* apoDatas );
    
    apl_int_t GetTimedout( const char* apcQueueName, apl_int_t aiTimedout, std::string* apoData );

	apl_int_t GetTimedout( const char* apcQueueName, apl_int_t aiTimedout, acl::CMemoryBlock* apoData );

    apl_ssize_t GetTimedout( 
        const char* apcQueueName, 
        apl_size_t auNumber,
        apl_int_t aiTimedout, 
        std::vector<std::string>* apoDatas );
	
	apl_int_t Put( const char* apcQueueName, acl::CIndexDict& aoRequest, acl::CIndexDict** appoResponse );

	apl_int_t Put( const char* apcQueueName, char const* apoBuffer, apl_size_t auLength );
	
	apl_int_t Put( const char* apcQueueName, std::string const& aoData );
	
	apl_int_t Put( const char* apcQueueName, acl::CMemoryBlock const& aoData );
	
    apl_int_t CreateQueue( char const* apcQueueName, apl_int_t aiOpt = 0, CQueueInfo* apoInfo = APL_NULL );
    
    apl_int_t GetQueueInfo( char const* apcQueueName, CQueueInfo& aoInfo );
    
    apl_int_t GetQueueInfo( apl_size_t auQueueID, CQueueInfo& aoInfo );
    
    apl_int_t GetAllQueueInfo( std::vector<CQueueInfo>& aoInfos );

    apl_int_t GetServerNodeInfo( char const* apcNodeName, CServerNodeInfo& aoInfo );
    
    apl_int_t GetAllServerNodeInfo( std::vector<CServerNodeInfo>& aoInfos );
    
    apl_int_t GetStartupTimestamp( acl::CTimestamp& aoTimestamp );
    
    apl_int_t BindNodeGroup( char const* apcQueueName, char const* apcGroupName );
	
    apl_ssize_t CommonRequest( acl::CIndexDict& aoRequest, acl::CIndexDict** appResponse, acl::CTimeValue const& aoTimedout );

protected:
	apl_int_t RequestAndUpdateReadableSegment(
        char const* apcQueueName, 
        apl_size_t auOldSegmentID,
        char const* apcOldNodeName,
        apl_int_t aiOldSegmentStatus,
        CSegmentInfo& aoSegmentInfo );
	
    apl_int_t RequestAndUpdateWriteableSegment(
        char const* apcQueueName, 
        apl_size_t auOldSegmentID,
        char const* apcOldNodeName,
        apl_int_t aiOldSegmentStatus,
        CSegmentInfo& aoSegmentInfo );

	apl_int_t RequestQueueSegment(
        apl_int_t aiCommandID,
        char const* apcQueueName, 
        apl_size_t auOldSegmentID,
        char const* apcOldNodeName,
        apl_int_t aiOldSegmentStatus,
        CSegmentInfo& aoSegmentInfo );

	apl_int_t ConnectToSegmentServer( 
        const char* apcServerIp, 
        apl_int_t aiPort, 
        ClientType** appoSegmentServer );
	
protected:
	apl_size_t muConnNum;

	apl_size_t muWindSize;

    acl::CTimeValue moTimedout;

    char macUserName[APL_NAME_MAX];

	CMasterClient moMaster;
    
    acl::CLock moLock;

    ServerMapType moServers;

    SegmentMapType moReadableSegments;
    
    SegmentMapType moWriteableSegments;

    acl::CTimeValue moErrorCacheTime;
    
    anf::IoProcessor* mpoProcessor;

    anf::CIoConfiguration moConfiguration;
};

AIBC_GFQ_NAMESPACE_END

#endif

