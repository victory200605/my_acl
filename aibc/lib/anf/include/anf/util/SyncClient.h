
#ifndef ANF_SYNCCLIENT_H
#define ANF_SYNCCLIENT_H

#include "anf/IoSockConnector.h"
#include "anf/IoConfiguration.h"
#include "anf/IoSession.h"
#include "anf/IoHandler.h"
#include "anf/protocol/BencodingMessage.h"
#include "anf/util/ConnectManager.h"
#include "acl/stl/vector.h"
#include "acl/MsgQueue.h"
#include "acl/IndexDict.h"

ANF_NAMESPACE_START

////////////////////////////////////////////////////////////////////////////////////////////
template<typename MessageType> class TSyncClient 
{
};

//////////////////////////////////////// index-dict ////////////////////////////////////////
template<>
class TSyncClient<acl::CIndexDict> : public IoHandler, public CConnectManager
{
    struct CSessionNode
    {
        CSessionNode( SessionPtrType& aoSession ) 
            : moSession(aoSession)
            , muMessageID(0) 
        {
        }

        SessionPtrType moSession;
        apl_size_t muMessageID;
    };

public:
    typedef TSyncClient<acl::CIndexDict> Self;

public:
    TSyncClient(void)
    {
    }
    
    virtual ~TSyncClient()
    {
        this->Close();
    }

    apl_int_t Initialize( apl_size_t auWindSize, acl::CTimeValue const& aoTimeout )
    {
        this->moConnector.SetConnectTimeout(aoTimeout);
        this->moConnector.SetHandler(this);
        this->moConnector.GetConfiguration()->SetAutoResumeRead(false);
        
        return CConnectManager::Initialize(&this->moConnector, 1);
    }
    
    apl_ssize_t Connect( char const* apcName, acl::CSockAddr const& aoRemoteAddress, apl_size_t auConnSize, bool abIsWait = true )
    {
        return CConnectManager::Connect(apcName, aoRemoteAddress, auConnSize, abIsWait);
    }
    
    void Close(void)
    {
        CSessionNode* lpoNode = NULL;

        this->moConnector.Dispose();
        CConnectManager::Close();

        while(this->moIdle.Pop(lpoNode, acl::CTimeValue::ZERO) == 0)
        {
            ACL_DELETE(lpoNode);
        }
    }

    apl_int_t Request( 
        acl::CIndexDict& aoRequest, acl::CIndexDict** appoResponse, acl::CTimeValue const& aoTimeout )
    {
        apl_int_t     liRetCode = 0;
        apl_ssize_t   liResult  = 0;
        apl_uint32_t  liLength  = 0;
        CSessionNode* lpoNode   = NULL;
        
        do
        {
            if (this->moIdle.Pop(lpoNode, aoTimeout) != 0)
            {
                liRetCode = E_SOCK_CONNECT;
                break;
            }

            aoRequest.Set(anf::BENC_TAG_SEQ_NO, ++lpoNode->muMessageID);
            
            acl::CMemoryBlock* lpoBlock = lpoNode->moSession->GetReadBuffer();
            lpoBlock->Reset();
            lpoBlock->SetWritePtr(4);//reserve 4byte length

            if (aoRequest.Encode(lpoBlock) != 0)
            {
                liRetCode = E_INVALID_MESSAGE;
                break;
            }

            //reset buffer length
            liLength = lpoBlock->GetLength();
            apl_uint32_t liTemp = apl_hton32(liLength - 4);
            apl_memcpy(lpoBlock->GetReadPtr(), &liTemp, sizeof(liTemp) );

            //Send request
            acl::CSockStream loPeer;
            loPeer.SetHandle(lpoNode->moSession->GetHandle() );
            liResult = loPeer.Send(lpoBlock->GetReadPtr(), liLength, aoTimeout);
            if (liResult <= 0)
            {
                liRetCode = E_SOCK_SEND;
                break;
            }

            //recv response
            liResult = loPeer.Recv(&liLength, sizeof(liLength), aoTimeout);
            if (liResult != sizeof(liLength) ) 
            {
                liRetCode = E_TIMEOUT;
                break;
            }

            liLength = apl_ntoh32(liLength);

            if (liLength > lpoBlock->GetSize() )
            {
                liRetCode = E_INVALID_MESSAGE;
                break;
            }

            lpoBlock->Reset();
            liResult = loPeer.Recv(lpoBlock->GetWritePtr(), liLength, aoTimeout);
            if (liResult != (apl_ssize_t)liLength)
            {
                liRetCode = E_TIMEOUT;
                break;
            }
            lpoBlock->SetWritePtr(liResult);
            
            ACL_NEW_ASSERT(*appoResponse, acl::CIndexDict);
            if ( (*appoResponse)->Decode(lpoBlock) != 0)
            {
                liRetCode = E_INVALID_MESSAGE;
                ACL_DELETE(*appoResponse);
                break;
            }
        }
        while(false);

        if (lpoNode != NULL)
        {
            if (liRetCode == E_TIMEOUT)
            {
                lpoNode->moSession->Close(lpoNode->moSession);
                ACL_DELETE(lpoNode);
            }
            else
            {
                this->moIdle.Push(lpoNode);
            }
        }
        
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

//Handler hook method
protected:
    /**
     * Invoked when a connection has been opened.
     */
    virtual void SessionOpened( SessionPtrType& aoSession )
    {
        this->AddSession(aoSession);
    }

    /**
     * Invoked when a connection is closed.
     */
    virtual void SessionClosed( SessionPtrType& aoSession )
    {
        CConnectManager::Erase(aoSession);
    }

    /**
     * Invoked when a message is received.
     */
    virtual void MessageReceived( SessionPtrType& aoSession, acl::CAny const& aoMessage )
    {
        ACL_ASSERT(false);
    }
   
protected:
    void AddSession( SessionPtrType& aoSession )
    {
        CSessionNode* lpoNode = NULL;
        ACL_NEW_ASSERT(lpoNode, CSessionNode(aoSession) );

        this->moIdle.Push(lpoNode);
    }
    
protected:
    acl::CLock moLock;

    acl::TMsgQueue<CSessionNode*> moIdle;
    
    CIoSockConnector moConnector;
};

ANF_NAMESPACE_END

#endif//ANF_SYNCCLIENT_H
