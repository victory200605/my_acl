/*!
 * \file     CacheClient.h
 * \brief
 * Client API for CacheServer
 *
 * Copyright (c) 2005 Asiainfo Technologies(China),Inc.
 * RCS: $Id: CacheClient.h,v 1.1 2010/12/20 07:55:30 daizh Exp $
 *
 * History
 * 2005.03.08 first release by huanghh
 * 
 */
 
#ifndef	_CACHECLIENT_H_
#define	_CACHECLIENT_H_

#include "AIMemAlloc.h"
#include "AIQueue.h"
//#include "AIAsyncComm.h"
//#include "cpunifyasync.h"
#include "AIAsyncTcp.h"

#define	CACHECLIENT_CONFIG_FILE	"../../config/CacheClientV1.ini"

#define	ASYNC_SEND_FAIL		-100
#define	ASYNC_RECV_FAIL		-200

#define CACHE_TYPE_SM       0x0001
#define CACHE_TYPE_MMS      0x0002
#define CACHE_TYPE_WAP      0x0004

typedef int (*DelTimeoutCBFun)(const char *apcKey, const void *apvValue, const int aiValueSize, void *apvParam);
typedef void (*RetryDeleteCBFun)(const int aiResult, const char *apcKey, const void *apvValue, const int aiValueSize, void *apvParam);

typedef struct
{
	char				*cpcKey;
	RetryDeleteCBFun	cRetryDeleteCBFun;
	void				*cpvParam;
	int					ciInQueueTime;
} RetryDeleteReq;

typedef struct
{
	char				csIpAddr[20];
	int					ciPort;
	int					ciConnection;
	int					ciMinRange;
	int					ciMaxRange;
	clsAsyncClient		*cpoAsyncClient;
} CacheServerInfo;

class CacheClient
{
	public:
		CacheClient();
		~CacheClient();
		
		int Init(DelTimeoutCBFun aDelTimeoutCBFun, const int aiTypeFlag, void *apvParam=NULL, const int aiParamSize=0, const char *apcConfigFile=CACHECLIENT_CONFIG_FILE);
		void Shutdown();
		
		int SaveData(const char *apcKey, const void *apvValue, const int aiValueLen, const int aiType=CACHE_TYPE_SM);
		int DeleteData(const char *apcKey, AIChunk *apoValueBuf, 
			RetryDeleteCBFun aRetryDeleteCBFun=NULL, void *apvParam=NULL, const int aiParamSize=0);
		int DeleteData(const char *apcKey, void *apvData, const int aiDataSize, 
			RetryDeleteCBFun aRetryDeleteCBFun=NULL, void *apvParam=NULL, const int aiParamSize=0);
			
	private:
		int GetServerPos(const char *apcKey);
		static void *DeleteTimeout(void *apvParam);
		void *DeleteTimeoutThread();
		static void *SecondDelete(void *apvParam);
		void *SecondDeleteThread();
		int AsyncSendPacket(clsAsyncClient *apoAsyncClient, const AIChunk &aoRequest, char *apcResponse, const int aiBufSize);
		
		int				ciShutdown;
		int				ciDelTimeoutInterval;
		int				ciRetryDeleteInterval;;
		char			csLogFile[20];
		int				ciServerCnt;
		int				ciTypeFlag;
		CacheServerInfo	*cpCacheServer;
		CacheServerInfo	*cpBackupServer;
		bool			cbStartBackupServer;
		AIQueue			*cpoRetryQueue;
		DelTimeoutCBFun	cCBFun;
		void			*cpvParam;
};

#endif
