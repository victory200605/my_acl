
#include "../protocol/CountServerProtocol.h"
#include "../include/CountServerError.h"
#include "CountServerApi.h"
#include "acl/SockConnector.h"
#include "acl/ThreadManager.h"
#include "acl/ObjectPool.h"
#include "acl/stl/string.h"
#include "acl/stl/vector.h"
#include "acl/stl/map.h"

class CCountServerStreamImpl : public ICountServerStream, public acl::CSockStream
{
public:
    struct CMsgNode
    {
        apl_size_t muMsgID;
        
        char macBuffer[CCSPResponse::RESPONSE_SIZE];
        
        bool mbIsSignal;
        
        acl::CCondition moCond;
            
        apl_size_t muNext;
    };
    
public:
    typedef std::map<std::string, apl_int_t>  ConnMapType;
    typedef std::map<apl_size_t,  apl_size_t> MsgMapType;
        
public:
    CCountServerStreamImpl( apl_size_t auSize )
        : mbIsClosed(true)
        , mbIsConned(false)
        , muMsgID(0)
        , muMsgNodeSlot(0)
        , muTotalNodeNum(auSize)
        , mi16Port(0)
    {
        ACL_NEW_N_ASSERT(this->mpoMsgNodes, CMsgNode, auSize);
        
        for ( apl_size_t liN = 0; liN < auSize; liN++ )
        {
            this->mpoMsgNodes[liN].muNext = liN + 1;
        }
        
        this->muFirstAvailableNode = 0;
        
        apl_memset(this->macIpAddress, 0, sizeof(this->macIpAddress) );
    }
    
    virtual ~CCountServerStreamImpl()
    {
        this->mbIsClosed = true;
        
        this->Close();
        
        this->moThreadManager.WaitAll();
        
        ACL_DELETE_N(this->mpoMsgNodes);
    }
    
    virtual int ConnectTo( char const* apcIpAddress, unsigned short ai16Port, int aiTimeout )
    {
        apl_strncpy(this->macIpAddress, apcIpAddress, sizeof(this->macIpAddress) );
        this->mi16Port = ai16Port;
        this->moTimeout = aiTimeout;
        
        apl_int_t           liRetCode = 0;
        acl::CSockAddr      loAddr(this->mi16Port, this->macIpAddress);
        acl::CSockConnector loConnector;
        
        this->mbIsClosed = false;
        if ( (liRetCode = loConnector.Connect(*this, loAddr, this->moTimeout ) ) != 0)
        {
            this->mbIsConned = false;
        }
        else
        {
            this->mbIsConned = true;
        }

        do
        {
            if ( (liRetCode = this->moThreadManager.Spawn( CCountServerStreamImpl::ConnSvc, this) ) != 0 )
            {
                break;
            }
            
            if ( (liRetCode = this->moThreadManager.Spawn( CCountServerStreamImpl::RecvSvc, this) ) != 0 )
            {
                break;
            }
            
            return liRetCode;
        }
        while(false);
        
        this->mbIsClosed = true;
        this->mbIsConned = false;
        this->Close();
        
        return liRetCode;
    }

    virtual int RequestConnToken( char const* apcKey, int aiNum, int aiTimeout )
    {
        acl::CTimeValue loTimeout(aiTimeout);
            
        return this->Request(CNTSVR_REQUEST_CONN, apcKey, aiNum, loTimeout);
    }
    
    virtual int RequestSpeedToken( char const* apcKey, int aiNum, int aiTimeout )
    {
        acl::CTimeValue loTimeout(aiTimeout);
        
        return this->Request(CNTSVR_REQUEST_SEND, apcKey, aiNum, loTimeout);
    }
    
protected:
    apl_int_t Report( acl::CTimeValue const& aoTimeout )
    {
        std::vector<char const*> loList;
        
        {
            acl::TSmartLock<acl::CLock> loGuard(this->moLock);
        
            for ( ConnMapType::iterator loIter = this->moConnMap.begin();
                loIter != this->moConnMap.end(); ++loIter )
            {
                loList.push_back(loIter->first.c_str() );
            }
        }
        
        for ( std::vector<char const*>::iterator loIter = loList.begin(); loIter != loList.end(); ++loIter )
        {
            if ( this->Request(CNTSVR_REQUEST_CONN, *loIter, 0, aoTimeout) != 0 )
            {
                return -1;
            }
        }
        
        return 0;
    }
    
    apl_int_t Request( apl_int32_t aiCmdID, char const* apcKey, apl_int_t aiNum, acl::CTimeValue const& aoTimeout  )
    {
        apl_int_t liRetCode = 0;
        std::pair<ConnMapType::iterator, bool> loConnPair;
        
        if (this->mbIsConned)
        {
            apl_ssize_t   liResult  = 0;
            apl_size_t    liCurrSlot = 0;
            CCSPRequest   loRequest;
            CCSPResponse  loResponse;
            char          lsSendBuffer[CCSPRequest::REQUEST_SIZE];

            loRequest.SetCmdID(aiCmdID);
            loRequest.SetKey(apcKey);
            loRequest.SetNum(aiNum);
            
            this->moLock.Lock();
            
            do
            {
                if (aiCmdID == CNTSVR_REQUEST_CONN)
                {
                    loConnPair = this->moConnMap.insert( ConnMapType::value_type(apcKey, 0) );
                    if (aiNum + loConnPair.first->second < 0)
                    {
                        liRetCode = CNTSVR_ERROR_LIMITED;
                        break;
                    }
                    
                    loRequest.SetLastNum(loConnPair.first->second);
                    
                    loConnPair.first->second += aiNum;
                }
                
                if (this->muTotalNodeNum <= this->muFirstAvailableNode)
                {
                    liRetCode = CNTSVR_ERROR_NO_ENOUGH_CONNNODE;
                    break;
                }
                
                loRequest.SetMsgID(++this->muMsgID);

                this->mpoMsgNodes[this->muFirstAvailableNode].muMsgID = this->muMsgID;
                this->mpoMsgNodes[this->muFirstAvailableNode].mbIsSignal = false;
                this->moMsgNodeMap[this->muMsgID] = this->muFirstAvailableNode;
                liCurrSlot = this->muFirstAvailableNode;
                this->muFirstAvailableNode = this->mpoMsgNodes[this->muFirstAvailableNode].muNext;

                loRequest.Encode(lsSendBuffer, sizeof(lsSendBuffer) );
                
                liResult = this->Send(lsSendBuffer, sizeof(lsSendBuffer), aoTimeout);
                if (liResult != sizeof(lsSendBuffer) )
                {
                    if ( !( liResult < 0 && apl_get_errno() == APL_ETIMEDOUT ) )
                    {
                    	  //FIX:do by RecvSvc
                        //this->mbIsConned = false;
                        //this->Close();
                        liRetCode = CNTSVR_ERROR_CONNRESET;
                    }
                    else
                    {
                        liRetCode = CNTSVR_ERROR_TIMEOUT;
                    }
                    break;
                }

                if ( ( liRetCode = this->mpoMsgNodes[liCurrSlot].moCond.Wait(this->moLock, aoTimeout) ) == 0
                    && this->mpoMsgNodes[liCurrSlot].mbIsSignal )
                {
                    loResponse.Decode(
                        this->mpoMsgNodes[liCurrSlot].macBuffer, 
                        sizeof(this->mpoMsgNodes[liCurrSlot].macBuffer) );
                    
                    liRetCode = loResponse.GetState();
                }
                else
                {
                    liRetCode = CNTSVR_ERROR_TIMEOUT;
                }
                
                if (aiCmdID == CNTSVR_REQUEST_CONN && liRetCode == CNTSVR_ERROR_LIMITED)
                {
                    loConnPair.first->second -= aiNum;
                }
                
                this->mpoMsgNodes[liCurrSlot].muNext = this->muFirstAvailableNode;
                this->muFirstAvailableNode = liCurrSlot;
                
                this->moMsgNodeMap.erase(this->mpoMsgNodes[liCurrSlot].muMsgID);
            }
            while(false);
            
            this->moLock.Unlock();
            
            return liRetCode == CNTSVR_ERROR_LIMITED ? -1 : 0;
        }
        else
        {
            if (aiCmdID == CNTSVR_REQUEST_CONN)
            {
                acl::TSmartLock<acl::CLock> loGuard(this->moLock);
                loConnPair = this->moConnMap.insert( ConnMapType::value_type(apcKey, 0) );
                if (aiNum + loConnPair.first->second < 0)
                {
                    //CNTSVR_ERROR_LIMITED
                    return -1;
                }
                else
                {
                    loConnPair.first->second += aiNum;
                }
            }
            
            return 0;
        }
    }
    
    static void* ConnSvc( void* apvStream )
    {
        CCountServerStreamImpl*    lpoStream = static_cast<CCountServerStreamImpl*>(apvStream);
        apl_int_t           liRetCode = 0;
        acl::CSockAddr      loAddr(lpoStream->mi16Port, lpoStream->macIpAddress);
        acl::CSockConnector loConnector;
        acl::CTimeValue     loTimeout(10);
            
        while(!lpoStream->mbIsClosed)
        {
            if (!lpoStream->mbIsConned)
            {
                liRetCode = loConnector.Connect(*lpoStream, loAddr, acl::CTimeValue(5) );
                if (liRetCode != 0)
                {
                    apl_sleep(APL_TIME_SEC);
                    continue;
                }
                
                lpoStream->mbIsConned = true;
                
                lpoStream->moConnCond.Signal();
                
                lpoStream->Report(loTimeout);
            }
            
            apl_sleep(APL_TIME_SEC);
        }
        
        return NULL;
    }

    static void* RecvSvc( void* apvStream )
    {
        CCountServerStreamImpl* lpoStream = static_cast<CCountServerStreamImpl*>(apvStream);
        apl_ssize_t         liResult = 0;
        acl::CTimeValue     loTimeout(2);
        acl::CTimeValue     loCondTimeout(0.5);
        CCSPHead            loCSPHead;
        char                lacRecvBuffer[CCSPResponse::RESPONSE_SIZE];
        char*               lpcCurrBuffer = lacRecvBuffer;
        apl_size_t          luBufferSize = sizeof(lacRecvBuffer);
            
        while(!lpoStream->mbIsClosed)
        {
            if (!lpoStream->mbIsConned)
            {
                lpoStream->moConnLock.Lock();
                lpoStream->moConnCond.Wait(lpoStream->moConnLock, loCondTimeout);
                lpoStream->moConnLock.Unlock();
                continue;
            }
            
            liResult = lpoStream->Recv(lpcCurrBuffer, luBufferSize, loTimeout );
            if (liResult != (apl_ssize_t)luBufferSize)
            {
                if (liResult < 0 && apl_get_errno() == APL_ETIMEDOUT)
                {
                    continue;
                }
                else if (liResult < (apl_ssize_t)luBufferSize && apl_get_errno() == APL_ETIMEDOUT)
                {
                    lpcCurrBuffer += liResult;
                    luBufferSize  -= liResult;
                    continue;
                }
                else
                {
                    lpoStream->mbIsConned = false;
                    lpoStream->Close();
                    
                    lpcCurrBuffer = lacRecvBuffer;
                    luBufferSize  = sizeof(lacRecvBuffer);
                    
                    continue;
                }
            }

            loCSPHead.Decode(lacRecvBuffer, sizeof(lacRecvBuffer) );
            
            do
            {
                acl::TSmartLock<acl::CLock> loGuard(lpoStream->moLock);

                MsgMapType::iterator loIter = lpoStream->moMsgNodeMap.find(loCSPHead.GetMsgID() );
                if (loIter == lpoStream->moMsgNodeMap.end() )
                {
                    break;
                }
                
                apl_memcpy(
                    lpoStream->mpoMsgNodes[loIter->second].macBuffer,
                    lacRecvBuffer,
                    sizeof(lpoStream->mpoMsgNodes[loIter->second].macBuffer) );
                
                lpoStream->mpoMsgNodes[loIter->second].mbIsSignal = true;
                lpoStream->mpoMsgNodes[loIter->second].moCond.Signal();
            }
            while(false);
            
            lpcCurrBuffer = lacRecvBuffer;
            luBufferSize  = sizeof(lacRecvBuffer);
        }
        
        return NULL;
    }
    
protected:
    bool mbIsClosed;
    
    bool mbIsConned;
    
    ConnMapType moConnMap;
    
    CMsgNode*   mpoMsgNodes;
    
    MsgMapType  moMsgNodeMap;
    
    acl::CLock  moLock;
    
    acl::CLock  moConnLock;
    
    acl::CCondition moConnCond;
    
    apl_size_t  muMsgID;
    
    apl_size_t  muMsgNodeSlot;
    
    apl_size_t  muFirstAvailableNode;
    
    apl_size_t  muTotalNodeNum;
    
    acl::CThreadManager moThreadManager;
    
    char macIpAddress[64];
    
    apl_uint16_t mi16Port;
    
    acl::CTimeValue moTimeout;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
CCountServerApi::CCountServerApi()
    : mpoImpl(NULL)
{
    ACL_NEW_ASSERT(this->mpoImpl, CCountServerStreamImpl(2048) );
}
    
CCountServerApi::~CCountServerApi()
{
    ACL_DELETE(this->mpoImpl);
}

int CCountServerApi::Initialize( const char* apcIpAddr, unsigned short ai16Port, int aiTimeout )
{
    return this->mpoImpl->ConnectTo(apcIpAddr, ai16Port, aiTimeout);
}
    
int CCountServerApi::Close(void)
{
    ACL_DELETE(this->mpoImpl);
    return 0;
}

int CCountServerApi::AcquireConnToken( char const* apcKey, size_t auNum, int aiTimeout )
{
    return this->mpoImpl->RequestConnToken(apcKey, auNum, aiTimeout);
}

int CCountServerApi::ReleaseConnToken( char const* apcKey, size_t auNum, int aiTimeout )
{
    return this->mpoImpl->RequestConnToken(apcKey, -1 * (int)auNum, aiTimeout);
}
    
int CCountServerApi::AcquireSpeedToken( char const* apcKey, size_t auNum, int aiTimeout )
{
    return this->mpoImpl->RequestSpeedToken(apcKey, auNum, aiTimeout);
}
