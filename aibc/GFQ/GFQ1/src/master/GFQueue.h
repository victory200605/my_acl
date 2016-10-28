//=============================================================================
/**
 * \file    GFQueue.h
 * \brief 
 *
 * Copyright (c) 2007 Asiainfo Technologies(China),Inc.
 * RCS: $Id: GFQueue.h,v 1.2 2010/12/20 08:19:04 daizh Exp $
 *
 * History
 * 2008.04.04 first release by DZH
 */
//=============================================================================
#ifndef AI_GFQUEUE_H_2008
#define AI_GFQUEUE_H_2008

#include "gfq/GFQUtility.h"
#include "ModuleMgr.h"
#include "QueueMgr.h"

// start namespace
AI_GFQ_NAMESPACE_START

// permision function define
typedef int32_t (*TPermisionFunc)( const char* apsQueueName, int* apiCapacity );
typedef int32_t (*TLoadQueueInfoFunc)( void );

// Segment info for request
struct stSegmentServerInfo
{
	int32_t ciSegmentID;
	int32_t ciQueueGlobalID;
	size_t  ciSegmentSize;
	size_t  ciSegmentCapacity;
	char ccQueueName[AI_MAX_NAME_LEN];
	char ccModuleName[AI_MAX_NAME_LEN];
};

// Segment info for client request
struct stSegmentClientInfo
{
	int32_t ciSegmentID;
	int32_t ciQueueGlobalID;
	int32_t ciQueueID;
	int32_t ciModuleID;
	char ccIpAddr[AI_IP_ADDR_LEN];
	int32_t ciPort;
};

// Base Queue info, queue info query will be return
struct stQueueInfo
{
    char    ccQueueName[AI_MAX_NAME_LEN];
    int32_t ciSegmentCnt;
	size_t  ciQueueCapacity;
	size_t  ciQueueSize;
};

// Base Module info, module info query will be return
struct stModuleInfo
{
    char    ccModuleName[AI_MAX_NAME_LEN];
    int32_t ciSegmentCnt;
	int32_t ciUsingSegmentCnt;
	int32_t ciFreeSegmentCnt;
};

class clsGFQueue
{
public:
	clsGFQueue();
	
	void SetErrorLimit( int32_t aiNum );

	int32_t LoadPermisionFunc( const char* apsLibName );
	void SetQueueDefaultCapacity( int32_t aiCapacity );

	int32_t LoadQueueInfo();
	bool IsPermision( const char* apsQueueName, int32_t& aiQueueCapacity );
	void RefreshQueue();
	
	// for store module
	int32_t AddModule( const char* apsModuleName, const char* apsIpAddr, int32_t aiPort );
	int32_t AddSegment( const stSegmentServerInfo& aoSegmentInfo );
	int32_t AddSegmentTry( const stSegmentServerInfo& aoSegmentInfo );
	int32_t DelModule( const char* apsModuleName );
	int32_t HeartBeat( const char* apsModuleName );
	void    CheckModule( int32_t aiTimeout );
	
	// for queue
	int32_t GetWriteSegment( const char* apsQueueName, stSegmentClientInfo& aoSegmentInfo );
	int32_t GetReadSegment( const char* apsQueueName, stSegmentClientInfo& aoSegmentInfo );
	int32_t WriteSegmentFail( 
	    const char* apsQueueName, int32_t aiSegmentID, int32_t aiModuleID, int32_t aiRetcode, int32_t* apiBehave );
	int32_t ReadSegmentFail( 
	    const char* apsQueueName, int32_t aiSegmentID, int32_t aiModuleID, int32_t aiRetcode, int32_t* apiBehave );
	
	// get queue size
	int32_t GetQueueStat( const char* apsQueueName, size_t* apiSize, size_t* apiCapacity );
	size_t GetQueueCount();
	size_t GetModuleCount();
	size_t GetSegmentCount();
	size_t GetUsingSegmentCount();
	size_t GetFreeSegmentCount();
	
	void GetAllQueueInfo( AI_STD::vector<stQueueInfo>& aoQueueInfos );
	void GetAllModuleInfo( AI_STD::vector<stModuleInfo>& aoModuleInfos );
	
protected:
	// for queue
	int32_t CreateQueue( const char* apsQueueName, TQueueHandle& atQueueHandle );
	int32_t DelQueue( const char* apsQueueName );
	int32_t GetWriteSegment( const char* apsQueueName, stSegmentNode** apoSegmentNode );
	int32_t GetWriteSegment( TQueueHandle atQueueHandle, stSegmentNode** apoSegmentNode );
		
protected:
	clsQueueMgr coQueueMgr;
	
	AIMutexLock coLock;
	
	// Protected Queue info reload
	AIMutexLock coQueueInfoLock;
	
	AIShlib coShlib;
	
	TPermisionFunc cfIsPermisionFunc;
	TLoadQueueInfoFunc cfLoadFunc;
	
	size_t ciQueueDefaultCapacity;
};

//end namespace
AI_GFQ_NAMESPACE_END

#endif //AI_GFQUEUE_H_2008
