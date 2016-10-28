#include "LogClientAPI.h"
#include "LogServerProtocol.h"
#include "AILib.h"
#include "AISynch.h"
#include "AIThrPool.h"
#include "AISocket.h"
#include "AIConfig.h"
#include <string>
#include <map>
#include <vector>

class CLogClientApiSingle
{
public:
    struct CMsgNode
    {
        size_t muMsgID;

        AICondition moCond;
                                           
        size_t muNext;

        bool mbIsSignal;

        int32_t mi32StatusCode;
    };

typedef std::map<size_t, size_t> MsgMapType;

public:
	CLogClientApiSingle()
		: muMsgID(0)
		, miConnCloseReason(LOG_SERVER_CONNTION_CLOSE_RECV_FAILED)
		, muSendBufLength(1024)
	{
		AI_MALLOC_ASSERT(this->mpcSendBuf, char, this->muSendBufLength);
	}

	~CLogClientApiSingle()
	{
	}

	int Initialize( 
	    const char* apcIpAddr, 
		uint16_t au16Port,
		size_t auTotalNodeNum,
		LogClientConnCloseCallback apfCallback,
		void* apvArg,
		time_t auSendIdlePeriod,
		time_t auReqTimeout,
		time_t auRspTimeout)
	{
		//ConnectTo apcIpAddr:au16Port aiTimeout
		strncpy(this->macIpAddress, apcIpAddr, sizeof(this->macIpAddress));
		this->mu16Port = au16Port;
		this->muReqTimeout = auReqTimeout;
		this->muRspTimeout = auRspTimeout;
		this->muSendIdlePeriod = auSendIdlePeriod;

		this->moLock.Lock();
		AI_MALLOC_ASSERT(this->mpoMsgNodes, CMsgNode, auTotalNodeNum * sizeof(CMsgNode));

		for ( size_t liN = 0; liN < auTotalNodeNum; liN++ )
		{
			this->mpoMsgNodes[liN].muNext = liN + 1;
		}
	        
		this->muFirstAvailableNode = 0;

		//set max pending msg nodes 
		this->muTotalNodesNum = auTotalNodeNum;
		this->moLock.Unlock();

		//set connection close callback
		this->mpfConnCloseCallback = apfCallback;
		this->mpvConnCloseCallbackArg = apvArg;

		int liRetCode = 0;

		this->mbIsClosed = false;
		if( (liRetCode = ai_socket_connect(apcIpAddr, au16Port, auRspTimeout)) < 0)
		{
			 this->mbIsConned = false;
			 this->moConnCloseLock.Lock();
			 this->miConnCloseReason = LOG_SERVER_CONNTION_CLOSE_SEND_FAILED;
			 this->moConnCloseLock.Unlock();
		}
		else
		{
			this->mbIsConned = true;
		}

		//remember when we init
		this->muLastSendTime = time(NULL);
		assert(this->muLastSendTime >= 0);

		do
		{
			this->moThreadPool = AIThrPoolCreate(3);
			assert(this->moThreadPool != NULL);
			
			if ( (liRetCode = AIThrPoolCall(this->moThreadPool, CLogClientApiSingle::ConnSvc, this) ) != 0 )
			{
				//close the conntion here, other failtures would close in ConnSvc
				break;
			}

			if ( (liRetCode = AIThrPoolCall(this->moThreadPool, CLogClientApiSingle::RecvSvc, this) ) != 0 )
			{
				break;
			}

			if ( (liRetCode = AIThrPoolCall(this->moThreadPool, CLogClientApiSingle::HandShakeSvc, this) ) != 0 )
			{
				break;
			}

			return liRetCode;
		}
		while(false);

		this->mbIsClosed = true;
		this->mbIsConned = false;
		this->moConnCloseLock.Lock();
		this->miConnCloseReason = LOG_SERVER_CONNTION_CLOSE_SEND_FAILED;
		this->moConnCloseLock.Unlock();
		ai_socket_close(this->miSocket);

		return liRetCode;
	}

	int SendRequest(
			CLogServerProtocol* aoReq, 
			int32_t* api32Status, 
			uint32_t au32Mode)
	{
		int liRetCode = LOG_SERVER_ERRNO_NOCONN;
	    
		if (this->mbIsConned)
		{
			ssize_t   liMsgLength;
			ssize_t   liResult  = 0;
			size_t    liCurrSlot = 0;

			this->moLock.Lock();
	        
			this->muLastSendTime = time(NULL);
			do
			{
				if(this->muFirstAvailableNode >= this->muTotalNodesNum &&
					au32Mode == LOG_SERVER_MODE_TWO_WAY)
				{
					liRetCode = LOG_SERVER_ERRNO_TOOMANY;
					break;
				}

				aoReq->SetSeq(++this->muMsgID);

				if(au32Mode == LOG_SERVER_MODE_TWO_WAY)
				{
					this->mpoMsgNodes[this->muFirstAvailableNode].muMsgID = this->muMsgID;
					this->mpoMsgNodes[this->muFirstAvailableNode].mbIsSignal = false;
					this->mpoMsgNodes[this->muFirstAvailableNode].mi32StatusCode = -1;
					this->moMsgNodeMap[this->muMsgID] = this->muFirstAvailableNode;
					liCurrSlot = this->muFirstAvailableNode;
					this->muFirstAvailableNode = this->mpoMsgNodes[this->muFirstAvailableNode].muNext;
				}

				if( (liMsgLength = aoReq->Encode(this->mpcSendBuf, this->muSendBufLength) ) <= 0)
				{
					liRetCode = LOG_SERVER_ERRNO_SENDERR;
					break;
				}
	            
				liResult = ai_socket_senddata(this->miSocket, this->mpcSendBuf, liMsgLength, this->muReqTimeout);
				if (liResult != liMsgLength )
				{
					if ( liResult != AI_SOCK_ERROR_PARTIAL )
					{
						this->mbIsConned = false;
						ai_socket_close(this->miSocket);
						this->miSocket = -1;
						liRetCode = LOG_SERVER_ERRNO_BROKEN;
						this->moConnCloseLock.Lock();
						this->miConnCloseReason = LOG_SERVER_CONNTION_CLOSE_SEND_FAILED;
						this->moConnCloseLock.Unlock();
					}
					else
					{
						liRetCode = LOG_SERVER_ERRNO_TIMEDOUT;
					}
					break;
				}

				if(au32Mode == LOG_SERVER_MODE_ONE_WAY)
				{
					liRetCode = 0;
					break;
				}

				if ( liRetCode = this->mpoMsgNodes[liCurrSlot].moCond.WaitFor(this->moLock, this->muRspTimeout * AI_TIME_SEC) == 0
					&& this->mpoMsgNodes[liCurrSlot].mbIsSignal )
				{
	                
					*api32Status = this->mpoMsgNodes[liCurrSlot].mi32StatusCode;
					liRetCode = 0;
				}
				else
				{
					liRetCode = LOG_SERVER_ERRNO_TIMEDOUT;
				}
	            
				this->mpoMsgNodes[liCurrSlot].muNext = this->muFirstAvailableNode;
				this->muFirstAvailableNode = liCurrSlot;
	            
				this->moMsgNodeMap.erase(this->mpoMsgNodes[liCurrSlot].muMsgID);
			}
			while(false);
	        
			this->moLock.Unlock();
		}

		return liRetCode;
	}

	int SendRequest(
			CLogServerMsg const& aoReq, 
			int32_t* api32Status, 
			uint32_t au32Mode)
	{
		CLogServerProtocol loProtocol((CLogServerMsg*)&aoReq);

		loProtocol.SetMode(au32Mode);

		return this->SendRequest(&loProtocol, api32Status, au32Mode);
	}
	static void* ConnSvc( void* apvStream )
	{
		CLogClientApiSingle* lpoStream = (CLogClientApiSingle*)apvStream;
		int liRetCode = 0;
	        
		while(!lpoStream->mbIsClosed)
		{
			if (!lpoStream->mbIsConned)
			{
				lpoStream->moConnCloseLock.Lock();
				//callback 
				if (lpoStream->mpfConnCloseCallback != NULL)
				{
					lpoStream->mpfConnCloseCallback(
							lpoStream->mpvConnCloseCallbackArg,
							lpoStream->miConnCloseReason, 
							lpoStream->macIpAddress,
							lpoStream->mu16Port);
				}

				//reset the close reason
				lpoStream->miConnCloseReason = LOG_SERVER_CONNTION_CLOSE_RECV_FAILED;
				lpoStream->moConnCloseLock.Unlock();
				liRetCode = ai_socket_connect(lpoStream->macIpAddress, lpoStream->mu16Port, 5);
				if (liRetCode != 0)
				{
					AISleepFor(AI_TIME_SEC);
					continue;
				}
				lpoStream->miSocket = liRetCode;
				lpoStream->mbIsConned = true;
	            
				lpoStream->moConnCond.Signal();
			}
	        
			AISleepFor(AI_TIME_SEC);
		}
	    
		return NULL;
	}

    void Release()
    {
    }

	static void* RecvSvc( void* apvStream )
	{
		CLogClientApiSingle* lpoStream = (CLogClientApiSingle*)apvStream;
		ssize_t liResult = 0;
		AITime_t loTimeout = 2 * AI_TIME_SEC;
		AITime_t loCondTimeout = 1 * AI_TIME_SEC;
		CLogServerProtocol loLogMsgResponse(NULL);
		char lacRecvBuffer[LOG_SERVER_PROTOCOL_HEADER_LEN];
		char* lpcCurrBuffer = lacRecvBuffer;
		size_t luBufferSize = sizeof(lacRecvBuffer);

		while(!lpoStream->mbIsClosed)
		{
			if (!lpoStream->mbIsConned)
			{
				lpoStream->moConnLock.Lock();
				lpoStream->moConnCond.WaitFor(lpoStream->moConnLock, loCondTimeout);
				lpoStream->moConnLock.Unlock();
				continue;
			}
	        
			liResult = ai_socket_recvdata(lpoStream->miSocket, lpcCurrBuffer, luBufferSize);
			if (liResult >= 0 && liResult != (ssize_t)luBufferSize)
			{
				lpcCurrBuffer += liResult;
				luBufferSize -= liResult;

				AISleepFor(loTimeout);
				continue;
			}
			else
			{
				lpoStream->mbIsConned = false;
				ai_socket_close(lpoStream->miSocket);
	            
				lpcCurrBuffer = lacRecvBuffer;
				luBufferSize  = sizeof(lacRecvBuffer);

				lpoStream->moConnCloseLock.Lock();
				lpoStream->miConnCloseReason = LOG_SERVER_CONNTION_CLOSE_RECV_FAILED;
				lpoStream->moConnCloseLock.Unlock();
				continue;
			}

			if(loLogMsgResponse.Decode(lacRecvBuffer, sizeof(lacRecvBuffer)) < 0)
			{
				lpoStream->mbIsConned = false;
				ai_socket_close(lpoStream->miSocket);

				lpoStream->moConnCloseLock.Lock();
				lpoStream->miConnCloseReason = LOG_SERVER_CONNTION_CLOSE_RECV_INVALID_MSG;
				lpoStream->moConnCloseLock.Unlock();
				continue;
			}
	        
			do
			{
				AISmartLock loGuard(lpoStream->moLock);

				MsgMapType::iterator loIter = lpoStream->moMsgNodeMap.find(loLogMsgResponse.GetSeq() );
				if (loIter == lpoStream->moMsgNodeMap.end() )
				{
					break;
				}
	            
				lpoStream->mpoMsgNodes[loIter->second].mi32StatusCode = loLogMsgResponse.GetStatus();
	            
				lpoStream->mpoMsgNodes[loIter->second].mbIsSignal = true;
				lpoStream->mpoMsgNodes[loIter->second].moCond.Signal();
			}
			while(false);
	        
			lpcCurrBuffer = lacRecvBuffer;
			luBufferSize  = sizeof(lacRecvBuffer);
		}
	    
		return NULL;
	}

	static void* HandShakeSvc(void* apvStream)
	{
		CLogClientApiSingle* lpoStream =(CLogClientApiSingle*)apvStream;
		CLogServerProtocol loShakeHand(NULL);
		time_t luTimeNow;
		AITime_t loTimeout = 1 * AI_TIME_SEC;

		loShakeHand.SetType(LOG_SERVER_PROTOCOL_TYPE_HSREQ);

		while(!lpoStream->mbIsClosed)
		{
			luTimeNow = time(NULL);
				//send handshakeSvc if timeout
			if(luTimeNow - lpoStream->muLastSendTime >= lpoStream->muSendIdlePeriod * AI_TIME_SEC)
			{
				if(lpoStream->SendRequest(&loShakeHand, NULL, LOG_SERVER_MODE_TWO_WAY) < 0)
				{
					lpoStream->moConnCloseLock.Lock();
					lpoStream->miConnCloseReason = LOG_SERVER_CONNTION_CLOSE_SEND_FAILED;
					lpoStream->moConnCloseLock.Unlock();
					lpoStream->mbIsConned = false;
					ai_socket_close(lpoStream->miSocket);
					continue;
				}
			}
			//sleep for better performance
			AISleepFor(loTimeout);
		}
	    
		return NULL;
	}

protected:
    bool mbIsClosed;
    bool mbIsConned;
	AIMutexLock moLock;
    AIMutexLock  moConnLock;
    AICondition moConnCond;
    size_t  muMsgID;
    AIThrPool_t moThreadPool;
    char macIpAddress[64];
    uint16_t mu16Port;
    //request time out 
    time_t muReqTimeout;
    //response time out
    time_t muRspTimeout;
	time_t muSendIdlePeriod;
	time_t muLastSendTime;
	size_t muFirstAvailableNode;
    size_t  muTotalNodesNum;
    CMsgNode* mpoMsgNodes;
	MsgMapType  moMsgNodeMap;

    //connection closed callback routine
    LogClientConnCloseCallback mpfConnCloseCallback;
    //connection closed callback arg
    void* mpvConnCloseCallbackArg;

    //connection closed reason, see the DEFINE for detail
    AIMutexLock moConnCloseLock;
    int miConnCloseReason;

	int miSocket;
	char* mpcSendBuf;
	size_t muSendBufLength;
};

class CLogClientApiImpl
{
public:
	struct CLogParameters
	{
		char macIpAddr[64]; 
        uint16_t mu16Port;
		size_t muTotalNodeNum;
		time_t muSendIdlePeriod;
        time_t muReqTimeout;
        time_t muRspTimeout;
	};

	typedef std::map<CLogClientApiSingle*, CLogParameters*> ApiMapType;

public:
	CLogClientApiImpl(const char* apcConfigFile)
		: miCurIndex(0)
	{
		int liTotal = 0;
		int liRetCode;
		char lacField[20];
		AIConfig loConfig;
		CLogParameters* lpoParam;
		CLogClientApiSingle* lpoApi;

		liRetCode = loConfig.LoadINI(apcConfigFile);
		assert(liRetCode == 0);

		while(liTotal < 100)
		{
			snprintf(lacField, sizeof(lacField), "LogClientAPI%d", liTotal);
			if(loConfig.GetKeyCount(lacField) > 0)
			{
				liTotal++;
			}
			else
			{
				break;
			}
		}

		assert(liTotal > 0);

		for(int i = 0;i < liTotal;i++)
		{
			AI_NEW_ASSERT(lpoParam, CLogParameters);
			AI_NEW_ASSERT(lpoApi, CLogClientApiSingle);

			snprintf(lacField, sizeof(lacField), "LogClientAPI%d", i);
			if(loConfig.GetIniString(lacField, "server_ip", lpoParam->macIpAddr, 64) == 0)
			{
				snprintf(lpoParam->macIpAddr, sizeof(lpoParam->macIpAddr), "%s", "0.0.0.0");
			}
			lpoParam->mu16Port = loConfig.GetIniInt(lacField, "port", 9696);
			lpoParam->muSendIdlePeriod = loConfig.GetIniInt(lacField, "send_idle_period", 10);
			lpoParam->muReqTimeout = loConfig.GetIniInt(lacField, "request_timeout", 5);
			lpoParam->muRspTimeout = loConfig.GetIniInt(lacField, "response_timeout", 5);
			lpoParam->muSendIdlePeriod = loConfig.GetIniInt(lacField, "max_pending_requests", 100);

			this->moApiMap.insert(ApiMapType::value_type(lpoApi, lpoParam));
			moApiArray.push_back(lpoApi);
		}
	}

	~CLogClientApiImpl()
	{
		this->ShutDown();
		
		for(int i = 0;i < (signed)moApiArray.size();i++)
		{
			AI_DELETE(moApiArray[i]);
		}
	}

	int SendRequest(CLogServerMsg const& aoReq, int32_t* ai32Status, uint32_t au32Mode)
	{
		int liRetCode;

		liRetCode = this->moApiArray[miCurIndex]->SendRequest(aoReq, ai32Status, au32Mode);

		miCurIndex = (miCurIndex + 1) % moApiArray.size();

		return liRetCode;
	}

	int StartUp(LogClientConnCloseCallback apfCallback, void* apvArg)
	{
		ApiMapType::iterator loAIter;

		for(loAIter = this->moApiMap.begin();loAIter != this->moApiMap.end();loAIter++)
		{
			loAIter->first->Initialize(loAIter->second->macIpAddr, 
				loAIter->second->mu16Port,
				loAIter->second->muTotalNodeNum,
				apfCallback,
				apvArg,
				loAIter->second->muSendIdlePeriod,
				loAIter->second->muReqTimeout,
				loAIter->second->muRspTimeout);
		}

        return 0;
	}

	int ShutDown()
	{
		ApiMapType::iterator loAIter;
		
		for(loAIter = this->moApiMap.begin();loAIter != this->moApiMap.end();loAIter++)
		{
			loAIter->first->Release();
		}

        return 0;
	}

private:
	ApiMapType moApiMap;

	std::vector<CLogClientApiSingle*> moApiArray;

	int miCurIndex;
};

CLogClientApi::CLogClientApi(const char* apcConfigFile)
{
	AI_NEW_INIT_ASSERT(this->mpoApiImpl, CLogClientApiImpl, apcConfigFile);
}

CLogClientApi::~CLogClientApi()
{
	AI_DELETE(this->mpoApiImpl);
}

int CLogClientApi::SendRequest(CLogServerMsg const& aoReq,
   int32_t* api32Status,
   uint32_t au32Mode)
{
	return this->mpoApiImpl->SendRequest(aoReq, api32Status, au32Mode);
}

int CLogClientApi::StartUp(LogClientConnCloseCallback apfConnCloseCallback, void* apvArg)
{
	return this->mpoApiImpl->StartUp(apfConnCloseCallback, apvArg);
}

int CLogClientApi::ShutDown()
{
	return this->mpoApiImpl->ShutDown();
}
