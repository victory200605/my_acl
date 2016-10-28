
#include "anf/IoFilter.h"
#include "anf/IoService.h"
#include "anf/SimpleObjectPool.h"
#include "anf/protocol/SimpleAsyncMessage.h"
#include "anf/filter/SimpleAsyncMessageFilter.h"

ANF_NAMESPACE_START

CSimpleAsyncMessageFilter::CSimpleAsyncMessageFilter(void)
    : mpoMessage(NULL)
    , moMessage(mpoMessage)
{
}

CSimpleAsyncMessageFilter::~CSimpleAsyncMessageFilter(void)
{
    ACL_DELETE(this->mpoMessage);
}

void CSimpleAsyncMessageFilter::SessionIdle(
    IoNextFilter* apoNextFilter,
    SessionPtrType& aoSession,
    apl_int_t aiStatus )
{
    if (aiStatus == SESSION_WRITE_IDLE)
    {
        CSimpleAsyncMessage loHeartBeat;

        loHeartBeat.SetMessageID(-1);

        aoSession->Write(aoSession, &loHeartBeat);
    }
    else
    {
        apl_errprintf("Simple async message filter closing a read idle timedout session, (Local=%s:%"APL_PRIdINT",Remote=%s:%"APL_PRIdINT")\n", 
                aoSession->GetLocalAddress().GetIpAddr(), 
                (apl_int_t)aoSession->GetLocalAddress().GetPort(),
                aoSession->GetRemoteAddress().GetIpAddr(), 
                (apl_int_t)aoSession->GetRemoteAddress().GetPort() );

        aoSession->Close(aoSession);
    }

    apoNextFilter->SessionIdle(aoSession, aiStatus);
}

void CSimpleAsyncMessageFilter::MessageReceived(
    IoNextFilter* apoNextFilter,
    SessionPtrType& aoSession,
    acl::CAny const& aoMessage )
{
    acl::CMemoryBlock*  lpoBlock = NULL;

    //Cast to memoryblock
    aoMessage.CastTo<acl::CMemoryBlock*>(lpoBlock);    
    ACL_ASSERT(lpoBlock != NULL);
    
    while(true)
    {
        if(this->mpoMessage == NULL)
        {
            ACL_NEW_ASSERT(this->mpoMessage, CSimpleAsyncMessage);
        }

        //Decode all message
        apl_ssize_t liResult = this->mpoMessage->Decode(*lpoBlock);
        if (liResult == 0)
        {
            if (this->mpoMessage->GetMessageID() == -1)
            {
                //Heart beat response message
                ACL_DELETE(this->mpoMessage);

                continue;
            }

            CSimpleAsyncMessage** lppoTemp = this->moMessage.CastTo<CSimpleAsyncMessage*>();
            ACL_ASSERT(lppoTemp != NULL);
            
            *lppoTemp = this->mpoMessage;
            
            apoNextFilter->MessageReceived(aoSession, this->moMessage);
            
            this->mpoMessage = NULL;
            
            continue;
        }
        else if (liResult < 0 || liResult > (apl_ssize_t)lpoBlock->GetSize() )
        {
            apoNextFilter->MessageReceived(aoSession, aoMessage);

            break;
        }
        
        break;
    }
}

void CSimpleAsyncMessageFilter::FilterWrite(
    IoNextFilter* apoNextFilter,
    SessionPtrType& aoSession,
    CWriteFuture::PointerType& aoWriteFuture )
{
    CSimpleAsyncMessage* lpoMessage = NULL;
    acl::CMemoryBlock*   lpoBlock = NULL;
        
    aoWriteFuture->GetMessage().CastTo<CSimpleAsyncMessage*>(lpoMessage);
    ACL_ASSERT(lpoMessage != NULL);
    
    //Allocate scp message buffer
    //lpoBlock = SimpleObjectPoolInstance<acl::CMemoryBlock>()->Allocate();
    //lpoBlock->Resize(aoSession->GetService()->GetConfiguration()->GetWriteBufferSize() );
    //lpoBlock->Reset();

    apl_int32_t liBufferLen = aoSession->GetService()->GetConfiguration()->GetWriteBufferSize();
    if( liBufferLen < 1024 )
    {   
        liBufferLen += 1024;
    }

    ACL_NEW_ASSERT(lpoBlock, acl::CMemoryBlock(liBufferLen) );
        
    if (lpoMessage->Encode(*lpoBlock) != 0)
    {
        ACL_ASSERT(false);
    }
    
    aoWriteFuture->SetMessage(lpoBlock);
    
    apoNextFilter->FilterWrite(aoSession, aoWriteFuture);
}
    
CIoFilter* CSimpleAsyncMessageFilter::Clone(void)
{
    CSimpleAsyncMessageFilter* lpoFilter = NULL;
    
    ACL_NEW_ASSERT(lpoFilter, CSimpleAsyncMessageFilter);
    
    return lpoFilter;
}

ANF_NAMESPACE_END
