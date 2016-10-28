
#include "anf/util/ConnectManager.h"

ANF_NAMESPACE_START

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//connect future node info
struct CConnectFutureNode
{
    std::string moGroup;
    anf::CConnectFuture::PointerType moFuture;
    acl::CSockAddr moRemoteAddress;
    bool mbIsFinished;
};

struct CIsClosedSession
{
    template<typename T> bool operator () ( T aoNode ) const
    {
        return aoNode->GetSession()->IsClosed();
    }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////
CConnectManager::CConnectManager(void)
    : mpoConnector(NULL)
    , moInterval(1)
    , mbIsClosing(true)
    , mbIsForceCloseOnResize(true)
{
}

CConnectManager::~CConnectManager(void)
{
}

apl_int_t CConnectManager::Initialize( anf::CIoConnector* apoConnector, acl::CTimeValue const& aoInterval )
{
    if (this->mpoConnector != NULL)
    {
        return -1;
    }

    this->mpoConnector = apoConnector;
    this->moInterval = aoInterval;
    this->mbIsClosing = false;

    if (this->moThreadManager.Spawn(CConnectManager::Svc, this) != 0)
    {
        this->mpoConnector = NULL;
        this->mbIsClosing = true;
        return -1;
    }

    return 0;
}
    
void CConnectManager::Close(void)
{
    this->mbIsClosing = true;
    this->moThreadManager.WaitAll();
}

apl_ssize_t CConnectManager::Connect( 
    char const* apcName, 
    acl::CSockAddr const& aoRemoteAddress, 
    apl_size_t auConnNum,
    bool abIsWait )
{
    {
        acl::TSmartLock<acl::CLock> loGuard(this->moLock);

        NodeMapType::iterator loIter = this->moNodes.find(apcName);
        if (loIter != this->moNodes.end() )
        {
            char lacTemp[32];
            apl_ssize_t liCurrNum = this->GetSize(apcName);

            aoRemoteAddress.GetIpAddr(lacTemp, sizeof(lacTemp) );

            if ( apl_strcmp(loIter->second.moRemoteAddress.GetIpAddr(), lacTemp) != 0
                 || loIter->second.moRemoteAddress.GetPort() != aoRemoteAddress.GetPort() )
            {
                //Clear last connection first
                if (this->IsForceCloseOnResize() )
                {
                    this->Remove(apcName, loIter->second.muConnNum);
                }

                //Update remote address
                loIter->second.moRemoteAddress.Set(aoRemoteAddress);
            }

            loIter->second.muConnNum = auConnNum;

            return liCurrNum;
        }
    }
    
    {//else
        apl_ssize_t liResult = 0;

        if (abIsWait)
        {
            std::vector<CConnectNode> loConnectNodes;
            loConnectNodes.resize(1);
            loConnectNodes[0].moGroup = apcName;
            loConnectNodes[0].moRemoteAddress.Set(aoRemoteAddress);
            loConnectNodes[0].muCurrNum = 0;
            loConnectNodes[0].muConnNum = auConnNum;

            liResult = this->Connect(loConnectNodes);
        }
        
        acl::TSmartLock<acl::CLock> loGuard(this->moLock);
        
        CNode loNode;
        
        loNode.moRemoteAddress.Set(aoRemoteAddress);
        loNode.muConnNum = auConnNum;
        
        this->moNodes.insert(NodeMapType::value_type(apcName, loNode) );

        return liResult;
    }
}

apl_ssize_t CConnectManager::Connect( std::vector<CConnectNode>& aoConnectNodes )
{
    std::vector<CConnectFutureNode> loConnectFutures;
    CConnectFutureNode loConnectFutureNode;
    apl_ssize_t liResult = 0;

    for (std::vector<CConnectNode>::iterator loIter = aoConnectNodes.begin();
         loIter != aoConnectNodes.end(); ++loIter)
    {
        //Handle connection changed event
        this->HandleChanged(loIter->moGroup.c_str(), loIter->moRemoteAddress, loIter->muCurrNum, loIter->muConnNum); 
        
        if (loIter->muCurrNum < loIter->muConnNum)
        {
            //Increase connections
            for (apl_size_t luN = loIter->muCurrNum; luN < loIter->muConnNum; luN++)
            {
                loConnectFutureNode.moFuture = this->mpoConnector->Connect(loIter->moRemoteAddress);
                loConnectFutureNode.moGroup = loIter->moGroup;
                loConnectFutureNode.moRemoteAddress.Set(loIter->moRemoteAddress);
                loConnectFutureNode.mbIsFinished = false;
                loConnectFutures.push_back(loConnectFutureNode);
            }
        }
        else if (loIter->muCurrNum > loIter->muConnNum)
        {
            //Decrease connections
            this->Remove(loIter->moGroup.c_str(), loIter->muCurrNum - loIter->muConnNum);
        }
    }

    for (apl_size_t luN = loConnectFutures.size(); luN > 0;)
    {
        for(std::vector<CConnectFutureNode>::iterator loIter = loConnectFutures.begin();
            loIter != loConnectFutures.end(); ++loIter)
        {
            if (loIter->mbIsFinished)
            {
                continue;
            }

            if (!loIter->moFuture->IsDone() )
            {
                continue;
            }

            if (loIter->moFuture->IsException() )
            {
                //Handle connect exception event
                this->HandleConnectException(
                    loIter->moGroup.c_str(), 
                    loIter->moRemoteAddress, 
                    loIter->moFuture->GetState(), 
                    loIter->moFuture->GetErrno() );
                
                apl_set_errno(loIter->moFuture->GetErrno() );
            }
            else
            {
                this->Insert(loIter->moGroup.c_str(), loIter->moFuture->GetSession() );

                //Handle session connected event
                this->HandleConnected(loIter->moFuture->GetSession() );
                liResult++;
            }
            
            luN--;
            loIter->mbIsFinished = true;
        }

        apl_sleep(APL_TIME_MSEC * 100);
    }
    
    return liResult;
}

void CConnectManager::Remove( char const* apcName, apl_size_t auNum )
{
    SessionPtrType loSession;
    
    for (apl_size_t auN = 0; auN < auNum; auN++)
    {
        if (!this->FindIf(apcName, CSessionManager::PRED_GROUP_BEGIN, loSession) )
        {
            break;
        }

        this->Erase(loSession);
        loSession->Close(loSession);

        //Handle session closed event
        this->HandleClosed(loSession);
    }
}

void CConnectManager::ClearAllClosedSession(void)
{
    std::vector<anf::SessionPtrType> loSessions;

    //Find all closed session
    this->FindAllIf(CIsClosedSession(), loSessions);

    //Erase all
    for (std::vector<anf::SessionPtrType>::iterator loIter = loSessions.begin();
         loIter != loSessions.end(); ++loIter)
    {
        this->Erase(*loIter);
    }
}

void CConnectManager::SetForceCloseOnResize( bool abIsEnable )
{
    this->mbIsForceCloseOnResize = abIsEnable;
}

bool CConnectManager::IsForceCloseOnResize(void)
{
    return this->mbIsForceCloseOnResize;
}

void* CConnectManager::Svc(void* apvParam)
{
    CConnectManager* lpoManager = static_cast<CConnectManager*>(apvParam);
    std::vector<CConnectNode> loConnectNodes;
    
    while(!lpoManager->mbIsClosing)
    {
        CConnectNode loConnectNode;

        lpoManager->ClearAllClosedSession();
       
        loConnectNodes.clear();
        //Lock and check node connection num
        lpoManager->moLock.Lock();

        for (NodeMapType::iterator loIter = lpoManager->moNodes.begin(); 
             loIter != lpoManager->moNodes.end(); ++loIter)
        {
            loConnectNode.muCurrNum = lpoManager->GetSize(loIter->first);
            loConnectNode.muConnNum = loIter->second.muConnNum;

            if (loConnectNode.muCurrNum == loConnectNode.muConnNum)
            {
                continue;
            }
            else
            {
                loConnectNode.moGroup = loIter->first;
                loConnectNode.moRemoteAddress.Set(loIter->second.moRemoteAddress);

                loConnectNodes.push_back(loConnectNode);
            }
        }
        
        //end
        lpoManager->moLock.Unlock();

        lpoManager->Connect(loConnectNodes);

        apl_sleep(lpoManager->moInterval.Nsec() );
    }

    return NULL;
}

void CConnectManager::HandleChanged( char const* apcName, acl::CSockAddr const& aoRemoteAddress, apl_size_t auCurrNum, apl_size_t auNextNum )
{
}

void CConnectManager::HandleConnected( anf::SessionPtrType& aoSession )
{
}

void CConnectManager::HandleClosed( anf::SessionPtrType& aoSession )
{
}

void CConnectManager::HandleConnectException( char const* apcName, acl::CSockAddr const& aoRemoteAddress, apl_int_t aiState, apl_int_t aiErrno )
{
}

ANF_NAMESPACE_END

