
#ifndef ANF_ASYNCCLIENT_H
#define ANF_ASYNCCLIENT_H

#include "anf/IoSockConnector.h"
#include "anf/IoConfiguration.h"
#include "anf/IoSession.h"
#include "anf/IoHandler.h"
#include "anf/protocol/BencodingMessage.h"
#include "anf/util/ConnectManager.h"
#include "acl/stl/vector.h"
#include "acl/IndexDict.h"

ANF_NAMESPACE_START

///////////////////////////////////////////////////////////////////////////////////////
//start message id trait
template<typename T>
struct TMessageIDTrait
{
    static apl_uint64_t GetMessageID(T const& t) 
    {
        return t.GetMessageID();
    }

    static void SetMessageID(T& t, apl_uint64_t au64MsgID)
    {
        t.SetMessageID(au64MsgID);
    }
};

//trait for CIndexDict
template<>
struct TMessageIDTrait<acl::CIndexDict>
{
    static apl_uint64_t GetMessageID(acl::CIndexDict const& t)
    {
        return t.GetInt(anf::BENC_TAG_SEQ_NO, 0);
    }

    static void SetMessageID(acl::CIndexDict& t, apl_uint64_t au64MsgID)
    {
        t.Set(anf::BENC_TAG_SEQ_NO, au64MsgID);
    }
};
//end message id trait

////////////////////////////////////////////////////////////////////////////////////////////
//message window
template<typename MessageType>
class TMessageWindow
{
    //Async message node define 
    struct CNode
    {
        apl_size_t muMessageID;
        
        MessageType* mpoResponse;
        
        bool mbIsSignal;
        
        acl::CCondition moCond;
            
        apl_size_t muNext;
    };

    struct CMessageWindowHandle
    {
        CMessageWindowHandle( apl_int_t aiSlot = -1, apl_size_t auMessageID = 0 )
            : miSlot(aiSlot)
            , muMessageID(auMessageID)
        {
        }

        apl_size_t GetMessageID(void)
        {
            return this->muMessageID;
        }

        bool IsEmpty(void)
        {
            return this->miSlot < 0;
        }

        apl_int_t miSlot;
        apl_size_t muMessageID;
    };

public:
    typedef std::map<apl_size_t,  apl_size_t> MessageMapType;
    typedef CMessageWindowHandle HandleType;

public:
    TMessageWindow(void)
        : mpoMessageNodes(APL_NULL)
        , muMessageID(0)
        , muFirstAvailableNode(0)
        , muTotalNodeNum(0)
    {
    }

    TMessageWindow( TMessageWindow const& aoOter )
        : mpoMessageNodes(APL_NULL)
        , muMessageID(0)
        , muFirstAvailableNode(0)
        , muTotalNodeNum(0)
    {
    }

    ~TMessageWindow(void)
    {
        ACL_DELETE_N(this->mpoMessageNodes);
    }

    apl_int_t Initialize( apl_size_t auSize )
    {
        ACL_DELETE_N(this->mpoMessageNodes);

        this->muTotalNodeNum = auSize;

        //Initialize message node
        ACL_NEW_N_ASSERT(this->mpoMessageNodes, CNode, this->muTotalNodeNum);
        
        for ( apl_size_t liN = 0; liN < this->muTotalNodeNum; liN++ )
        {
            this->mpoMessageNodes[liN].muNext = liN + 1;
        }
        
        this->muFirstAvailableNode = 0;

        return 0;
    }

    HandleType Get(void)
    {
        HandleType loHandle(-1, 0);

        acl::TSmartLock<acl::CLock> loGuard(this->moLock);

        //Check window size
        if (this->muTotalNodeNum <= this->muFirstAvailableNode)
        {
            return loHandle;
        }
        
        loHandle.muMessageID = this->GetNextMessageID();
        
        //Allocate window node
        this->mpoMessageNodes[this->muFirstAvailableNode].muMessageID = loHandle.muMessageID;
        this->mpoMessageNodes[this->muFirstAvailableNode].mbIsSignal = false;
        this->moMessageNodes[loHandle.muMessageID] = this->muFirstAvailableNode;
        loHandle.miSlot = this->muFirstAvailableNode;
        this->muFirstAvailableNode = this->mpoMessageNodes[this->muFirstAvailableNode].muNext;

       return loHandle; 
    }

    void Put( HandleType* apoHandle )
    {
        if (apoHandle->miSlot >= 0)
        {
            acl::TSmartLock<acl::CLock> loGuard(this->moLock);

            this->mpoMessageNodes[apoHandle->miSlot].muNext = this->muFirstAvailableNode;
            this->muFirstAvailableNode = apoHandle->miSlot;
            
            this->moMessageNodes.erase(this->mpoMessageNodes[apoHandle->miSlot].muMessageID);

            apoHandle->miSlot = -1;
            apoHandle->muMessageID = 0;
        }
    }

    MessageType* Wait( HandleType* apoHandle, acl::CTimeValue const& aoTimeout )
    {
        MessageType* lpoResponse = APL_NULL;

        if (apoHandle->IsEmpty() )
        {
            return APL_NULL;
        }

        this->moLock.Lock();

        //Wait response
        acl::CTimestamp loExpired(acl::CTimestamp::PRC_NSEC);
        loExpired += aoTimeout;
        while(true)
        {
            if (this->mpoMessageNodes[apoHandle->miSlot].mbIsSignal)
            {
                lpoResponse = this->mpoMessageNodes[apoHandle->miSlot].mpoResponse;
                break;
            }

            if (this->mpoMessageNodes[apoHandle->miSlot].moCond.Wait(this->moLock, loExpired) != 0)
            {
                break;
            }
        }
        
        this->mpoMessageNodes[apoHandle->miSlot].muNext = this->muFirstAvailableNode;
        this->muFirstAvailableNode = apoHandle->miSlot;
        
        this->moMessageNodes.erase(this->mpoMessageNodes[apoHandle->miSlot].muMessageID);

        apoHandle->miSlot = -1;
        apoHandle->muMessageID = 0;

        this->moLock.Unlock();

        return lpoResponse;
    }

    bool Signal( apl_size_t auMessageID, MessageType* apoResponse )
    {
        acl::TSmartLock<acl::CLock> loGuard(this->moLock);

        MessageMapType::iterator loIter = this->moMessageNodes.find(auMessageID);

        if (loIter == this->moMessageNodes.end() )
        {
            //Timeout
            return false;
        }
        else
        {
            this->mpoMessageNodes[loIter->second].mpoResponse = apoResponse;
            this->mpoMessageNodes[loIter->second].mbIsSignal = true;
            this->mpoMessageNodes[loIter->second].moCond.Signal();

            return true;
        }
    }

protected:
    apl_size_t GetNextMessageID(void)
    {
        this->muMessageID = this->muMessageID % 100000000;
        
        return ++this->muMessageID;
    }

private:
    acl::CLock  moLock;
    
    CNode* mpoMessageNodes;
    
    apl_size_t  muMessageID;
    
    apl_size_t  muFirstAvailableNode;
    
    apl_size_t  muTotalNodeNum;
    
    MessageMapType  moMessageNodes;
};

////////////////////////////////////////////////////////////////////////////////////////////
template<typename MessageType, typename MessageIDTraitType = TMessageIDTrait<MessageType> >
class TAsyncClient : public IoHandler, public CConnectManager
{
public:
    typedef TMessageWindow<MessageType> MessageWindowType;
    typedef std::vector<SessionPtrType> SessionListType;
    typedef TAsyncClient<MessageType, MessageIDTraitType> Self;

public:
    TAsyncClient(void)
        : muWindSize(1000)
        , muWindName(0)
        , muIndex(0)
    {
    }
    
    virtual ~TAsyncClient()
    {
        this->Close();
    }

    apl_int_t Initialize( apl_size_t auWindSize, acl::CTimeValue const& aoTimeout )
    {
        this->muWindSize = auWindSize;

        this->moConnector.SetConnectTimeout(aoTimeout);
        this->moConnector.SetHandler(this);
        
        return CConnectManager::Initialize(&this->moConnector, 1);
    }
    
    apl_ssize_t Connect( char const* apcName, acl::CSockAddr const& aoRemoteAddress, apl_size_t auConnSize, bool abIsWait = true )
    {
        apl_ssize_t liResult = CConnectManager::Connect(apcName, aoRemoteAddress, auConnSize, abIsWait);
        
        //At least one connection existed
        while (liResult > 0 && this->moSessions.size() == 0)
        {
            apl_sleep(APL_TIME_SEC);
        }

        return liResult;
    }
    
    void Close(void)
    {
        this->moConnector.Dispose();
        CConnectManager::Close();
    }

    apl_int_t Request( MessageType& aoRequest, MessageType** appoResponse, acl::CTimeValue const& aoTimeout )
    {
        apl_int_t  liRetCode = 0;
        
        do
        {
            SessionPtrType loSession = this->FindSession();
            if (loSession == APL_NULL)
            {
                liRetCode = E_SOCK_CONNECT;
                break;
            }
            
            //Allocate window node
            CIoSessionAttribute& loAttribute = loSession->GetAttribute();
            MessageWindowType* lpoWind = anf::Find<MessageWindowType>(loAttribute, this->muWindName);
            ACL_ASSERT(lpoWind != APL_NULL);
            
            typename MessageWindowType::HandleType loHandle = lpoWind->Get();
            if (loHandle.IsEmpty() )
            {
                liRetCode = E_WINDOW_FULL;
                break;
            }

            MessageIDTraitType::SetMessageID(aoRequest, loHandle.GetMessageID() );

            //Send request
            anf::CWriteFuture::PointerType loFuture = loSession->Write(loSession, &aoRequest);
            if (loFuture->IsException() )
            {
                liRetCode = loFuture->GetState();
                lpoWind->Put(&loHandle);

                break;
            }

            if( (*appoResponse = lpoWind->Wait(&loHandle, aoTimeout) ) == APL_NULL)
            {
                //printf("AsyncClient sent a timedout message, (MessageID=%"APL_PRIuINT")\n",
                //    (apl_size_t)MessageIDTraitType::GetMessageID(aoRequest) );
                liRetCode = E_TIMEOUT;
            }
            else
            {
                liRetCode = 0;
            }
        }
        while(false);
        
        return liRetCode;
    }
    
    CIoConfiguration* GetConfiguration(void)
    {
        return this->moConnector.GetConfiguration();
    }
    
    CIoFilterChain* GetFilterChain(void)
    {
        return this->moConnector.GetFilterChain();
    }

    CIoSockConnector* GetConnector(void)
    {
        return &this->moConnector;
    }

//Handler hook method
protected:
    /**
     * Invoked when a exception caught.
     */
    virtual void Exception( CThrowable const& aoCause )
    {
        apl_errprintf("AsyncClient catch a exception,%s (State=%"APL_PRIdINT",Errno=%"APL_PRIdINT")\n",
            aoCause.GetMessage(),
            (apl_int_t)aoCause.GetState(),
            (apl_int_t)aoCause.GetErrno() );
    }
    
    /**
     * Invoked when a session exception caught.
     */
    virtual void SessionException( SessionPtrType& aoSession, CThrowable const& aoCause )
    {
        apl_errprintf("AsyncClient catch a session exception,%s (SessionID=%"APL_PRIu64",Remote=%s:%"APL_PRIdINT",State=%"APL_PRIdINT",Errno=%"APL_PRIdINT")\n",
            aoCause.GetMessage(),
            (apl_uint64_t)aoSession->GetSessionID(),
            aoSession->GetRemoteAddress().GetIpAddr(),
            (apl_int_t)aoSession->GetRemoteAddress().GetPort(),
            (apl_int_t)aoCause.GetState(),
            (apl_int_t)aoCause.GetErrno() );
    }

    /**
     * Invoked when a connection has been opened.
     */
    virtual void SessionOpened( SessionPtrType& aoSession )
    {
        CIoSessionAttribute& loAttribute = aoSession->GetAttribute();
        //loAttribute.Insert(this->muWindName, MessageWindowType() );
        loAttribute.PushBack(MessageWindowType() );
        MessageWindowType* lpoWind = anf::Find<MessageWindowType>(loAttribute, this->muWindName);
        ACL_ASSERT(lpoWind != APL_NULL);
        lpoWind->Initialize(this->muWindSize);
        
        this->AddSession(aoSession);
    }

    /**
     * Invoked when a connection is closed.
     */
    virtual void SessionClosed( SessionPtrType& aoSession )
    {
        this->DelSession(aoSession);
        CConnectManager::Erase(aoSession);
    }

    /**
     * Invoked when a message is received.
     */
    virtual void MessageReceived( SessionPtrType& aoSession, acl::CAny const& aoMessage )
    {
        MessageType* lpoResponse = APL_NULL;

        if (!aoMessage.CastTo(lpoResponse) )
        {
            apl_errprintf("AsyncClient recv invalid message (SessionID=%"APL_PRIu64",Remote=%s:%"APL_PRIdINT")\n",
                (apl_uint64_t)aoSession->GetSessionID(),
                aoSession->GetRemoteAddress().GetIpAddr(),
                (apl_int_t)aoSession->GetRemoteAddress().GetPort() );

            aoSession->Close(aoSession);
            return;
        }

        ACL_ASSERT(lpoResponse != APL_NULL);

        CIoSessionAttribute& loAttribute = aoSession->GetAttribute();
        MessageWindowType* lpoWind = anf::Find<MessageWindowType>(loAttribute, this->muWindName);
        ACL_ASSERT(lpoWind != APL_NULL);

        if (!lpoWind->Signal(MessageIDTraitType::GetMessageID(*lpoResponse), lpoResponse) )
        {
            //printf("AsyncClient recv a timedout message, (this = %p, sessionid = %d, MessageID=%"APL_PRIuINT")\n", 
            //    this,
            //    (int)aoSession->GetSessionID(),
            //    (apl_size_t)MessageIDTraitType::GetMessageID(*lpoResponse) );
            ACL_DELETE(lpoResponse);
        }
    }
   
protected:
    void AddSession( SessionPtrType& aoSession )
    {
        acl::TSmartLock<acl::CLock> loGuard(this->moLock);
        
        this->moSessions.push_back(aoSession);
    }
    
    void DelSession( SessionPtrType& aoSession )
    {
        acl::TSmartLock<acl::CLock> loGuard(this->moLock);
        
        for(SessionListType::iterator loIter = this->moSessions.begin();
            loIter != this->moSessions.end(); ++loIter)
        {
            if (aoSession->GetSessionID() == (*loIter)->GetSessionID() )
            {
                this->moSessions.erase(loIter);
                break;
            }
        }
    }
    
    SessionPtrType FindSession(void)
    {
        acl::TSmartLock<acl::CLock> loGuard(this->moLock);
        
        if ( this->moSessions.size() == 0 )
        {
            return SessionPtrType();
        }

        this->muIndex = this->muIndex % this->moSessions.size();
            
        return this->moSessions[this->muIndex++];
    }

protected:
    SessionListType  moSessions;
    
    acl::CLock  moLock;
    
    apl_size_t  muWindSize;

    apl_size_t muWindName;

    apl_size_t muIndex;
    
    acl::CThreadManager moThreadManager;
    
    CIoSockConnector moConnector;

};

ANF_NAMESPACE_END

#endif//ANF_ASYNCCLIENT_H
