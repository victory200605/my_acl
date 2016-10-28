//=============================================================================
/**
 * \file    GFQueueApi.h
 * \brief 
 *
 * Copyright (c) 2007 Asiainfo Technologies(China),Inc.
 * RCS: $Id: GFQueueApi.h,v 1.2 2010/12/20 08:19:04 daizh Exp $
 *
 * History
 * 2008.04.04 first release by DZH
 */
//=============================================================================
#ifndef AI_GFQUEUEAPI_H_2008
#define AI_GFQUEUEAPI_H_2008

#include <stdio.h>
#include <unistd.h>
#include "GFQUtility.h"
#include "GFQProtocol.h"

using namespace gfq;

// Error cache for high frequency and repeat error
struct stErrorCache
{
    stErrorCache();
    bool IsHitCache();
    
    int  ciErrNo;
    int  ciTimestamp;
    int  ciTimeInterval;
};

// segment server cache node, will be saved in hashtable
struct stSegmentServer
{
	stSegmentServer();
	~stSegmentServer();
	
	char            ccServerIP[AI_IP_ADDR_LEN];
	int32_t         ciServerPort;
	char            ccServerIPPort[AI_IP_ADDR_LEN];
	clsAsyncClient* cpoAsyncClient;

	// must defined for hash table
	const char* GetKey()
	{
		return ccServerIPPort;
	}
	// must defined for hash table
	int Compare( const char* apsKey )
	{
		return strcmp( ccServerIPPort, apsKey );
	}
};

// read & write segment info cache node, will be saved in hashtable
struct stSegmentInfoNode
{
    stSegmentInfoNode();
    
	int              ciSegmentID;
	int              ciModuleID;
	int              ciQueueGlobalID;
	char             ccQueueName[AI_MAX_NAME_LEN];
	stSegmentServer* cpoSegmentServer;
	stErrorCache     coErrorCache;
	
	// must defined for hash table
	const char* GetKey()
	{
		return ccQueueName;
	}
	// must defined for hash table
	int Compare( const char* apsKey )
	{
		return strcmp( ccQueueName, apsKey );
	}
};

// Global File Queue Api interface defined
class clsGFQueueApi
{
public:
    enum { DEAD_CIRCLE_NUM = 1001 }; // 
    
public:
    template< class T >
    struct stMultiHashTable
    {
        typedef typename clsHashTable<const char*, T>::iterator iterator;
        
        AIMutexLock                  coLock;
        clsHashTable<const char*, T> coTable;
    };
    
    // typedef for hash table
	typedef stMultiHashTable<stSegmentInfoNode>           TSegmentHashTable;
	typedef stMultiHashTable<stSegmentServer>::iterator   TServerIter;
	typedef stMultiHashTable<stSegmentInfoNode>::iterator TSegmentIter;
	
public:
	clsGFQueueApi();
	~clsGFQueueApi();
	// Initialize api
	int32_t Initialize( const char* apsConfigFile );
	void Shutdown();
	
	// Get data
	int32_t Get( const char* apsQueueName, AIChunk& aoData, int32_t aiWhenSec = 0 );
	
	// Put data
	int32_t Put( const char* apsQueueName, const AIChunk& aoData );
	
	// Get queue status
	int32_t GetStat( const char* apsQueueName, int* apiSize, int* apiCapaciy );
	
	// Get module(segmentserver) count
	int32_t GetModuleCount( int* apiCount );
     
    // Notify Master reload queue info
    int32_t ReloadQueueInfo( );
    
    // Notify Master print all queue
    int32_t PrintQueues( );
    
    // Notify Master print all module
    int32_t PrintModules( );
    
protected:
	int32_t GetWriteSegment( const char* apsQueueName, stSegmentInfoNode& apoSegmentInfo );
	int32_t GetReadSegment( const char* apsQueueName, stSegmentInfoNode& apoSegmentInfo );
	
	int32_t GetWriteSegmentSecond( const char* apsQueueName, int32_t aiRetcode, stSegmentInfoNode& apoSegmentInfo );
	int32_t GetReadSegmentSecond( const char* apsQueueName, int32_t aiRetcode, stSegmentInfoNode& apoSegmentInfo );
	
	int32_t SetSegmentInfo( TSegmentHashTable& aoHashList, const char* apsQueueName, stSegmentInfoNode& apoSegmentInfo );
	int32_t GetSegmentInfo( TSegmentHashTable& aoHashList, const char* apsQueueName, stSegmentInfoNode& aoSegmentInfo );
	int32_t CreateSegmentServerConnection( const char* apsServerIp, int32_t aiPort, stSegmentServer** apoSegmentServer );
	
protected:
	int32_t ciConnectCnt;
	int32_t ciTimeOut;
	char    ccUserName[AI_MAX_NAME_LEN];
	char    ccPassWord[AI_MAX_NAME_LEN];
	
	stMultiHashTable<stSegmentServer>   coServers;
	stMultiHashTable<stSegmentInfoNode> coWriteSegments;
	stMultiHashTable<stSegmentInfoNode> coReadSegments;
	
	clsAsyncClient* cpoAsyncClient;
	stSegmentServer* cpoSegmentServer;

	// Error cache for high frequency and repeat error
	int32_t ciErrorCacheInterval;
};

#endif //AI_GFQUEUEAPI_H_2008
