/*!
 * \file      CacheClient.cpp
 * \brief
 * Client API for CacheServer
 *
 * Copyright (c) 2005 Asiainfo Technologies(China),Inc.
 * RCS: $Id: CacheClient.cpp,v 1.1 2010/12/20 07:55:30 daizh Exp $
 *
 * History
 * 2005.03.08 first release by huanghh
 * 
 */
#include <sys/timeb.h>
 
#include "cacheserver/CacheUtility.h"
#include "cacheserver/CacheClient.h"
#include "AILib.h"
#include "AIConfig.h"
#include "AILogSys.h"
#include "AIString.h"

#define  TRETTYPE       void*
#define  TRETURN        return(NULL)

#define CACHE_HASH_MAX      100

#define REQ_INSERT          0x0001
#define RESP_INSERT         0x8001
#define REQ_FETCH           0x0002
#define RESP_FETCH          0x8002
#define REQ_DELETE          0x0003
#define RESP_DELETE         0x8003
#define REQ_DELTIMEOUT      0x0004
#define RESP_DELTIMEOUT     0x8004
#define REQ_GET_COUNT       0x0005
#define RESP_GET_COUNT      0x8005

using namespace cacheserver;

int CreateBTThread( TRETTYPE (*apfFunction)(void*), void *apvParam )
{
    int             liRetCode = 0;

    pthread_t       pid;
    if( pthread_create( &pid, NULL, apfFunction, apvParam ) != 0 )
    {
        liRetCode = 0;
    }
    else
    {
        liRetCode = (int)pid;
        pthread_detach( pid );
    }

    return  (liRetCode);
}


void aiSleep( int aiSecond, int &aiShutDown )
{
    for( int liIt = 0; liIt < aiSecond && !aiShutDown; liIt++ )
    {
        sleep( 1 );
    }
}


void usSleep( int aiMilliSecond )
{
        int                             nSleepTime;
        struct timeb    tbBegin, tbEnd;

        if( (nSleepTime = aiMilliSecond) <= 0 )
                return;

BEGIN:
        ftime( &tbBegin );

        struct timeval          time_val;
        time_val.tv_sec = (nSleepTime >= 1000)?1:0;
        time_val.tv_usec = nSleepTime % 1000 * 1000;
        select( 0, NULL, NULL, NULL, &time_val );

        ftime( &tbEnd );
        nSleepTime -= ((tbEnd.time - tbBegin.time) * 1000 + (tbEnd.millitm - tbBegin.millitm));

        if( nSleepTime > 1000 )
        {
                goto BEGIN;
        }
}

CacheClient::CacheClient()
{
	memset(this, 0, sizeof(CacheClient));
}

CacheClient::~CacheClient()
{
	for (int i=0; i<ciServerCnt; i++)
	{
		AI_DELETE(cpCacheServer[i].cpoAsyncClient);
	}
	AI_DELETE_N(cpCacheServer);
	if (cpBackupServer != NULL)
	{
		AI_DELETE(cpBackupServer->cpoAsyncClient);
	}
	AI_DELETE(cpBackupServer);
	AI_DELETE(cpoRetryQueue);
}

void CacheClient::Shutdown()
{
	ciShutdown = true;
	for (int i=0; i<ciServerCnt; i++)
	{
		cpCacheServer[i].cpoAsyncClient->ShutDown();
	}
	if (cpBackupServer != NULL)
	{
		cpBackupServer->cpoAsyncClient->ShutDown();
	}
	return;
}

int CacheClient::Init(DelTimeoutCBFun aDelTimeoutCBFun, const int aiTypeFlag, void *apvParam, const int aiParamSize, const char *apcConfigFile)
{
	char		lsField[20];
	char		lsBuf[20];
	AIConfig	*lpIni;
	int			liTimeout;
	if ((lpIni = AIGetIniHandler(apcConfigFile)) == NULL)
	{
		AIWriteLOG(NULL, AILOG_LEVEL_ERROR, "Load cache config file %s fail", apcConfigFile);
		return 1;
	}
	
	ciTypeFlag = aiTypeFlag;
	cpvParam = apvParam;
	
	lpIni->GetIniString("Log", "LogFile", csLogFile, sizeof(csLogFile));
	ciDelTimeoutInterval = lpIni->GetIniInt("CacheClient", "DelTmtInterval", 10);
	ciRetryDeleteInterval = lpIni->GetIniInt("CacheClient", "RetryDelete", 3);
	liTimeout = lpIni->GetIniInt("CacheClient", "ConnTimeout", 5);
	cCBFun = aDelTimeoutCBFun;
	AI_NEW_ASSERT(cpoRetryQueue, AIQueue);
	
	//check server count
	for (int i=0; ; i++)
	{
		sprintf(lsField, "Server%d", i+1);
		if (lpIni->GetIniString(lsField, "ServerIP", lsBuf, sizeof(lsBuf)) == 0)
		{
			break;
		}
		else
		{
			ciServerCnt = i+1;
		}
	}
	
	AI_NEW_N_ASSERT(cpCacheServer, CacheServerInfo, ciServerCnt);
	for (int i=0; i<ciServerCnt; i++)
	{
		sprintf(lsField, "Server%d", i+1);
		lpIni->GetIniString(lsField, "ServerIP", cpCacheServer[i].csIpAddr, sizeof(cpCacheServer[i].csIpAddr));
		cpCacheServer[i].ciPort = lpIni->GetIniInt(lsField, "ServerPort", -1);
		cpCacheServer[i].ciConnection = lpIni->GetIniInt(lsField, "Connection", 1);
		cpCacheServer[i].ciMinRange = lpIni->GetIniInt(lsField, "MinRange", 0);
		cpCacheServer[i].ciMaxRange = lpIni->GetIniInt(lsField, "MaxRange", 99);
		cpCacheServer[i].cpoAsyncClient = new clsAsyncClient(cpCacheServer[i].csIpAddr, cpCacheServer[i].ciPort, cpCacheServer[i].ciConnection, liTimeout);
		if (cpCacheServer[i].cpoAsyncClient == NULL)
		{
			return 1;
		}
		cpCacheServer[i].cpoAsyncClient->StartDaemon(3);
	}
	
	if (lpIni->GetIniString("BackupServer", "ServerIP", lsBuf, sizeof(lsBuf)) != 0)
	{
		AI_NEW_ASSERT(cpBackupServer, CacheServerInfo);
		lpIni->GetIniString("BackupServer", "ServerIP", cpBackupServer->csIpAddr, sizeof(cpBackupServer->csIpAddr));
		cpBackupServer->ciPort = lpIni->GetIniInt("BackupServer", "ServerPort", -1);
		cpBackupServer->ciConnection = lpIni->GetIniInt("BackupServer", "Connection", 1);
		cpBackupServer->cpoAsyncClient = new clsAsyncClient(cpBackupServer->csIpAddr, cpBackupServer->ciPort, cpBackupServer->ciConnection, liTimeout);
		if (cpBackupServer->cpoAsyncClient == NULL)
		{
			return 1;
		}
		cpBackupServer->cpoAsyncClient->StartDaemon(3);
	}
	
	if ((cCBFun != NULL) && (CreateBTThread(&DeleteTimeout, this) == 0))
	{
		AIWriteLOG(csLogFile, AILOG_LEVEL_ERROR, "Fail to create DeleteTimeout thread when init CacheClient");
		return 1;
	}
	
	if (CreateBTThread(&SecondDelete, this) == 0)
	{
		AIWriteLOG(csLogFile, AILOG_LEVEL_ERROR, "Fail to create SecondDelete thread when init CacheClient");
		return 1;
	}
	
	AIWriteLOG(csLogFile, AILOG_LEVEL_INFO, "set delete timeout interval = %d", ciDelTimeoutInterval);
	AIWriteLOG(csLogFile, AILOG_LEVEL_INFO, "set retry delete interval   = %d", ciRetryDeleteInterval);
	for (int i=0; i<ciServerCnt; i++)
	{
		AIWriteLOG(csLogFile, AILOG_LEVEL_INFO, "set server%d(%s:%d:%d) range = %d~%d", i+1, cpCacheServer[i].csIpAddr, 
			cpCacheServer[i].ciPort, cpCacheServer[i].ciConnection, cpCacheServer[i].ciMinRange, cpCacheServer[i].ciMaxRange);
	}
	if (cpBackupServer != NULL)
	{
		AIWriteLOG(csLogFile, AILOG_LEVEL_INFO, "set backup server           = %s:%d:%d", 
			cpBackupServer->csIpAddr, cpBackupServer->ciPort, cpBackupServer->ciConnection);
	}
	
	return 0;
}

int CacheClient::SaveData(const char *apcKey, const void *apvValue, const int aiValueSize, const int aiType)
{
	int			liServerPos = 0;
	int			liRet;
	AIChunk		lclsSendBuf;
	int			liTotalLen;
	int			liCmdId;
	int			liType = aiType;
	int			liKeySize = strlen(apcKey)+1;
	int			liValueSize = aiValueSize;
	char		lsResponse[12];//total_len:command_id:status
	
	if (ciServerCnt > 1)
	{
		liServerPos = GetServerPos(apcKey);
		if (liServerPos < 0)
		{
			AIWriteLOG(csLogFile, AILOG_LEVEL_ERROR, "Fail to find server pos for key %s", apcKey);
			return AI_ERROR_INVALID_PARAM;
		}
	}
	
	lclsSendBuf.Resize(20+liKeySize+liValueSize);
	liTotalLen = htonl(20+liKeySize+liValueSize);
	liCmdId = htonl(REQ_INSERT);	
	liType = htonl(liType);
	liKeySize = htonl(liKeySize);
	liValueSize = htonl(liValueSize);
	memcpy(lclsSendBuf.GetPointer(), &liTotalLen, 4);
	memcpy(lclsSendBuf.GetPointer()+4, &liCmdId, 4);
	memcpy(lclsSendBuf.GetPointer()+8, &liType, 4);
	memcpy(lclsSendBuf.GetPointer()+12, &liKeySize, 4);
	memcpy(lclsSendBuf.GetPointer()+16, apcKey, strlen(apcKey)+1);
	memcpy(lclsSendBuf.GetPointer()+17+strlen(apcKey), &liValueSize, 4);
	memcpy(lclsSendBuf.GetPointer()+21+strlen(apcKey), apvValue, aiValueSize);
	
	if ((liRet = AsyncSendPacket(cpCacheServer[liServerPos].cpoAsyncClient, lclsSendBuf, lsResponse, sizeof(lsResponse))) == 0)
	{
		memcpy(&liRet, lsResponse+8, 4);
		liRet = ntohl(liRet);
	}
	
	//如果网络连接失败，则启用备用的CacheServer，并设置标识位表示备用CacheServer已被启用
	if ((liRet == ASYNC_SEND_FAIL) && (cpBackupServer != NULL))
	{
		AIWriteLOG(csLogFile, AILOG_LEVEL_WARN, "Connect CacheServer(%s:%d) fail, use backup CacheServer", 
					cpCacheServer[liServerPos].csIpAddr, cpCacheServer[liServerPos].ciPort);
		if ((liRet = AsyncSendPacket(cpBackupServer->cpoAsyncClient, lclsSendBuf, lsResponse, sizeof(lsResponse))) == 0)
		{
			memcpy(&liRet, lsResponse+8, 4);
			liRet = ntohl(liRet);
			cbStartBackupServer = true;
		}
	}
	
	if (liRet != 0)
	{
		AIWriteLOG(csLogFile, AILOG_LEVEL_ERROR, "Fail to save data, err=%d, key=%s", liRet, apcKey);
	}
	else
	{
		AIWriteLOG(csLogFile, AILOG_LEVEL_DEBUG, "Save data to CacheServer, key=%s", apcKey);
	}
	return liRet;
}

int CacheClient::DeleteData(const char *apcKey, AIChunk *apoValueBuf, 
		RetryDeleteCBFun aRetryDeleteCBFun, void *apvParam, const int aiParamSize)
{
	int			liServerPos = 0;
	int			liRet;
	AIChunk		lclsSendBuf;
	int			liTotalLen;
	int			liCmdId;
	int			liKeySize = strlen(apcKey)+1;
	char		lsResponse[12 + 4 + AI_MAX_VALUE_LEN];
	
	if (ciServerCnt > 1)
	{
		liServerPos = GetServerPos(apcKey);
		if (liServerPos < 0)
		{
			AIWriteLOG(csLogFile, AILOG_LEVEL_ERROR, "Fail to find server pos for key %s", apcKey);
			return AI_ERROR_INVALID_PARAM;
		}
	}
	
	lclsSendBuf.Resize(12+liKeySize);
	liTotalLen = htonl(12+liKeySize);
	if (apoValueBuf==NULL)
	{
		liCmdId = htonl(REQ_DELETE);
	}
	else
	{
		liCmdId = htonl(REQ_FETCH);
	}
	liKeySize = htonl(liKeySize);
	memcpy(lclsSendBuf.GetPointer(), &liTotalLen, 4);
	memcpy(lclsSendBuf.GetPointer()+4, &liCmdId, 4);
	memcpy(lclsSendBuf.GetPointer()+8, &liKeySize, 4);
	memcpy(lclsSendBuf.GetPointer()+12, apcKey, strlen(apcKey)+1);
	
	if ((liRet = AsyncSendPacket(cpCacheServer[liServerPos].cpoAsyncClient, lclsSendBuf, lsResponse, sizeof(lsResponse))) == 0)
	{
		memcpy(&liRet, lsResponse+8, 4);
		liRet = ntohl(liRet);
	}
	
//disable backupserver
#if 0
	//如果网络连接失败，或者在目的CacheServer找不到数据，而备用CacheServer已被启用，则尝试在备用CacheServer上查找数据
	if ((cbStartBackupServer) && ((liRet == ASYNC_SEND_FAIL) || (liRet == ERR_DATA_NOFOUND)) && (cpBackupServer != NULL))
	{
		AIWriteLOG(csLogFile, AILOG_LEVEL_INFO, "Try to delete data from backup CacheServer");
		if ((liRet = AsyncSendPacket(cpBackupServer->cpoAsyncClient, lclsSendBuf, lsResponse, sizeof(lsResponse))) == 0)
		{
			memcpy(&liRet, lsResponse+8, 4);
			liRet = ntohl(liRet);
		}
	}
#endif

	//如果删除数据失败，在有连接的情况下，如果需要二次匹配，则存储到重发队列
	if (liRet != 0)
	{
		AIWriteLOG(csLogFile, AILOG_LEVEL_ERROR, "Fail to delete data, err=%d, key=%s", liRet, apcKey);
		if ((aRetryDeleteCBFun != NULL)  && (liRet != ASYNC_SEND_FAIL))
		{
			RetryDeleteReq	lstRetryDeleteReq;
			AI_NEW_N_ASSERT(lstRetryDeleteReq.cpcKey, char, strlen(apcKey)+1);
			strcpy(lstRetryDeleteReq.cpcKey, apcKey);
			lstRetryDeleteReq.cpvParam = apvParam;
			lstRetryDeleteReq.cRetryDeleteCBFun = aRetryDeleteCBFun;
			lstRetryDeleteReq.ciInQueueTime = time(NULL);
			cpoRetryQueue->InQueue(&lstRetryDeleteReq, sizeof(RetryDeleteReq));
			AIWriteLOG(csLogFile, AILOG_LEVEL_INFO, "Save delete request to delete later, key=%s, ret=%d", apcKey, liRet);
		}
	}
	else
	{
		AIWriteLOG(csLogFile, AILOG_LEVEL_DEBUG, "Delete data from CacheServer, key=%s", apcKey);
		if (apoValueBuf != NULL)
		{
			int	liValueSize;
			memcpy(&liValueSize, lsResponse+12, 4);
			liValueSize = ntohl(liValueSize);
			apoValueBuf->Resize(liValueSize);
			memcpy(apoValueBuf->GetPointer(), lsResponse+16, liValueSize);
		}
	}
	
	return liRet;
}

int CacheClient::DeleteData(const char *apcKey, void *apvData, const int aiDataSize, 
		RetryDeleteCBFun aRetryDeleteCBFun, void *apvParam, const int aiParamSize)
{
	AIChunk		lclsData;
	int			liRet;
	
	liRet = DeleteData(apcKey, &lclsData, aRetryDeleteCBFun, apvParam, aiParamSize);
	if (liRet == 0)
	{
		memcpy(apvData, lclsData.GetPointer(), AI_MIN(lclsData.GetSize(), (unsigned int)aiDataSize));
	}
	
	return liRet;
}
	

int CacheClient::GetServerPos(const char *apcKey)
{
	int ciHashKey = GetStringHashKey1(apcKey, CACHE_HASH_MAX);
	int ciServerPos = -1;
	for (int i=0; i<ciServerCnt; i++)
	{
		if ((ciHashKey>=cpCacheServer[i].ciMinRange) && (ciHashKey<=cpCacheServer[i].ciMaxRange))
		{
			ciServerPos = i;
			break;
		}
	}
	return ciServerPos;
}

void *CacheClient::DeleteTimeout(void *apvParam)
{
	return ((CacheClient *)apvParam)->DeleteTimeoutThread();
}

void *CacheClient::DeleteTimeoutThread()
{
	AIChunk		lclsSendBuf;
	AIChunk		lclsKeyBuf;
	AIChunk		lclsValueBuf;
	int			liKeySize;
	int			liValueSize;
	int			liTotalLen;
	int			liCmdId;
	int			liResult;
	char		lsResponse[12 + 4 + AI_MAX_KEY_LEN + 4 + AI_MAX_VALUE_LEN];
	int			liFlag;
	int			liLen;
	int			liCmd;
	
	lclsSendBuf.Resize(12);
	liTotalLen = htonl(12);
	liCmdId = htonl(REQ_DELTIMEOUT);
	liFlag = htonl(ciTypeFlag);
	memcpy(lclsSendBuf.GetPointer(), &liTotalLen, 4);
	memcpy(lclsSendBuf.GetPointer()+4, &liCmdId, 4);
	memcpy(lclsSendBuf.GetPointer()+8, &liFlag, 4);
	
	while (!ciShutdown)
	{
		for (int i=0; i<ciServerCnt && !ciShutdown; i++)
		{
			while (!ciShutdown)
			{
				if (AsyncSendPacket(cpCacheServer[i].cpoAsyncClient, lclsSendBuf, lsResponse, sizeof(lsResponse)) != 0)
				{
					break;
				}
				memcpy(&liLen, lsResponse, 4);
				memcpy(&liCmd, lsResponse+4, 4);
				liLen = ntohl(liLen);
				liCmd = ntohl(liCmd);
				memcpy(&liResult, lsResponse+8, 4);
				liResult = ntohl(liResult);
				if (liResult != 0)
				{
					break;
				}
				else
				{
					memcpy(&liKeySize, lsResponse+12, 4);
					liKeySize = ntohl(liKeySize);
					memcpy(&liValueSize, lsResponse+16+liKeySize, 4);
					liValueSize = ntohl(liValueSize);
					lclsKeyBuf.Resize(liKeySize);
					lclsValueBuf.Resize(liValueSize);
					memcpy(lclsKeyBuf.GetPointer(), lsResponse+16, liKeySize);
					memcpy(lclsValueBuf.GetPointer(), lsResponse+20+liKeySize, liValueSize);
					(* cCBFun)(lclsKeyBuf.GetPointer(), lclsValueBuf.GetPointer(), lclsValueBuf.GetSize(), cpvParam);
					AIWriteLOG(csLogFile, AILOG_LEVEL_DEBUG, "Delete timeout data from CacheServer, key=%s", lclsKeyBuf.GetPointer());
					usSleep(1);
				}
			}
		}
		
		if ((cbStartBackupServer) && (cpBackupServer != NULL))
		{
#if 0//disable backupserver
			while (!ciShutdown)
			{
				if (AsyncSendPacket(cpBackupServer->cpoAsyncClient, lclsSendBuf, lsResponse, sizeof(lsResponse)) != 0)
				{
					break;
				}
				memcpy(&liResult,lsResponse+8, 4);
				liResult = ntohl(liResult);
				if (liResult == ERR_DATA_NOFOUND)
				{
					break;
				}
				else if (liResult == ERR_DATABASE_EMPTY)
				{
					//当备用CacheServer上无数据时，设置标识位，停止该CacheServer的使用
					AIWriteLOG(csLogFile, AILOG_LEVEL_INFO, "Stop using backup CacheServer");
					cbStartBackupServer = false;
					break;
				}
				else
				{
					memcpy(&liKeySize, lsResponse+12, 4);
					liKeySize = ntohl(liKeySize);
					memcpy(&liValueSize, lsResponse+16+liKeySize, 4);
					liValueSize = ntohl(liValueSize);
					lclsKeyBuf.Resize(liKeySize);
					lclsValueBuf.Resize(liValueSize);
					memcpy(lclsKeyBuf.GetPointer(),lsResponse+16, liKeySize);
					memcpy(lclsValueBuf.GetPointer(),lsResponse+20+liKeySize, liValueSize);
					(* cCBFun)(lclsKeyBuf.GetPointer(), lclsValueBuf.GetPointer(), lclsValueBuf.GetSize(), cpvParam);
					AIWriteLOG(csLogFile, AILOG_LEVEL_DEBUG, "Delete timeout data from CacheServer, key=%s", lclsKeyBuf.GetPointer());
					usSleep(1);
				}
			}
#endif
		}
		
		aiSleep(ciDelTimeoutInterval, *(int*)&ciShutdown);
	}
	
	return NULL;
}

void *CacheClient::SecondDelete(void *apvParam)
{
	return ((CacheClient *)apvParam)->SecondDeleteThread();
}

void *CacheClient::SecondDeleteThread()
{
	int				liTimeDiff;
	RetryDeleteReq	lstRetryDeleteReq;
	AIChunk			lstValue;
	int				liRet;
	
	while (!ciShutdown)
	{
		if (cpoRetryQueue->OutQueue(&lstRetryDeleteReq, sizeof(RetryDeleteReq)) > 0)
		{
			liTimeDiff = time(NULL) - lstRetryDeleteReq.ciInQueueTime;
			if (liTimeDiff < ciRetryDeleteInterval)
			{
				aiSleep(ciRetryDeleteInterval-liTimeDiff, *(int*)&ciShutdown);
			}
			if (ciShutdown)
			{
				AI_DELETE_N(lstRetryDeleteReq.cpcKey);
				break;
			}
			
			if ((liRet=DeleteData(lstRetryDeleteReq.cpcKey, &lstValue)) == 0)
			{
				AIWriteLOG(csLogFile, AILOG_LEVEL_DEBUG, "Delete data second time success, key=%s", lstRetryDeleteReq.cpcKey);
				lstRetryDeleteReq.cRetryDeleteCBFun(0, lstRetryDeleteReq.cpcKey, lstValue.GetPointer(), lstValue.GetSize(), lstRetryDeleteReq.cpvParam);
			}
			else
			{
				AIWriteLOG(csLogFile, AILOG_LEVEL_ERROR, "Fail to delete data second time, key=%s, ret=%d", lstRetryDeleteReq.cpcKey, liRet);
				lstRetryDeleteReq.cRetryDeleteCBFun(liRet, lstRetryDeleteReq.cpcKey, NULL, 0, lstRetryDeleteReq.cpvParam);
			}
			AI_DELETE_N(lstRetryDeleteReq.cpcKey);
		}
		else
		{
			aiSleep(ciRetryDeleteInterval, *(int*)&ciShutdown);
		}
	}
	
	return NULL;
}

int CacheClient::AsyncSendPacket(clsAsyncClient *apoAsyncClient, const AIChunk &aoRequest, char *apcResponse, const int aiBufSize)
{
	int	liRespSize;
	if (apoAsyncClient->SendRequest(aoRequest.GetPointer(), aoRequest.GetSize(), NULL) != 0)
	{
		return ASYNC_SEND_FAIL;
	}
	else if (apoAsyncClient->RecvResponse(apcResponse, aiBufSize, liRespSize) != 0)
	{
		return ASYNC_RECV_FAIL;
	}
	else
	{
		return 0;
	}
}
