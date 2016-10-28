
#include "anf/IoFilter.h"
#include "anf/IoService.h"
#include "anf/protocol/ScpMessage.h"
#include "anf/filter/ScpMessageFilter.h"

ANF_NAMESPACE_START

void CScpMessageFilter::MessageReceived(
    IoNextFilter* apoNextFilter,
    SessionPtrType& aoSession,
    acl::CAny const& aoMessage )
{
    acl::CMemoryBlock* lpoBlock = NULL;
    scp::CScpMessage*  lpoMessage = NULL;
    bool lbInvaild = false;
    
    //Cast to memoryblock
    aoMessage.CastTo<acl::CMemoryBlock*>(lpoBlock);    
    ACL_ASSERT(lpoBlock != NULL);
    
    while(true)
    { 
        //Decode all message
        if ( (lpoMessage = scp::DecodeMessage(*lpoBlock) ) != NULL)
        {
            if (lpoMessage->GetType() == anf::scp::MSG_TYPE_INVAILD)
            {
                lbInvaild = true;
            }

            acl::CAny loMessage(lpoMessage);
            apoNextFilter->MessageReceived(aoSession, loMessage);
            aoSession->IncreaseReadMessages(1);

            if (lbInvaild)
            {
                lpoBlock->Reset();
                break;
            }
            
            continue;
        }
        
        break;
    }
}

void CScpMessageFilter::FilterWrite(
    IoNextFilter* apoNextFilter,
    SessionPtrType& aoSession,
    CWriteFuture::PointerType& aoWriteFuture )
{
    apl_int32_t        liLength = 0;
    scp::CScpMessage*  lpoMessage = NULL;
    acl::CMemoryBlock* lpoBlock = NULL;
    char*              lpcTempWritePtr = NULL;
        
    aoWriteFuture->GetMessage().CastTo<scp::CScpMessage*>(lpoMessage);
    ACL_ASSERT(lpoMessage != NULL);
    
    //Allocate scp message buffer
    ACL_NEW_ASSERT(lpoBlock, acl::CMemoryBlock(
        aoSession->GetService()->GetConfiguration()->GetWriteBufferSize() ) );

    //Save current write ptr for 4 byte length
    lpcTempWritePtr = lpoBlock->GetWritePtr();
    lpoBlock->SetWritePtr(4);
    
    if (lpoMessage->Encode(*lpoBlock) != 0)
    {
        ACL_ASSERT(false);
    }
    
    liLength = apl_hton32( (apl_int32_t)(lpoBlock->GetWritePtr() - lpcTempWritePtr) );
    
    apl_memcpy(lpcTempWritePtr, &liLength, sizeof(liLength) );
    
    aoWriteFuture->SetMessage(lpoBlock);
    
    apoNextFilter->FilterWrite(aoSession, aoWriteFuture);
    aoSession->IncreaseWrittenMessages(1);
}
    
CIoFilter* CScpMessageFilter::Clone(void)
{
    CScpMessageFilter* lpoFilter = NULL;
    
    ACL_NEW_ASSERT(lpoFilter, CScpMessageFilter);
    
    return lpoFilter;
}

ANF_NAMESPACE_END
