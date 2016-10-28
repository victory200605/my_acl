#include "anf/protocol/SMPPMessage.h"
#include "anf/filter/SMPPMessageFilter.h"
#include "anf/IoService.h"
#include "anf/IoFilter.h"
#include "anf/IoService.h"

ANF_NAMESPACE_START

void CSMPPMessageFilter::MessageReceived(
    IoNextFilter* apoNextFilter,
    SessionPtrType& aoSession,
    acl::CAny const& aoMessage )
{
    acl::CMemoryBlock* lpoBlock = APL_NULL;
    smpp::CSMPPMessage loMessage;
    apl_int_t liLength;
    //Cast to memoryblock
    aoMessage.CastTo<acl::CMemoryBlock*>(lpoBlock);
    ACL_ASSERT(lpoBlock != APL_NULL);
    
    while(true)
    { 
        //Decode all message
        liLength = loMessage.Decode(lpoBlock->GetReadPtr(), lpoBlock->GetLength());
        if ( liLength > 0 && liLength <= (apl_int_t)lpoBlock->GetSize() )
        {
            lpoBlock->SetReadPtr(liLength);

            acl::CAny loAny(&loMessage);
            apoNextFilter->MessageReceived(aoSession, loAny);
            
            continue;
        }
        else if( liLength == 0 )
        {
            //haven't finish receive
            break;
        }
        //else liLength > lpoBlock->GetSize() || liLength < 0

        //decode fail
        apoNextFilter->MessageReceived(aoSession, aoMessage);
        break;
    }
}

void CSMPPMessageFilter::FilterWrite(
    IoNextFilter* apoNextFilter,
    SessionPtrType& aoSession,
    CWriteFuture::PointerType& aoWriteFuture )
{
    apl_int32_t        liLength = 0;
    smpp::CSMPPMessage  *lpoMessage = APL_NULL;
    acl::CMemoryBlock* lpoBlock = APL_NULL;
    
    aoWriteFuture->GetMessage().CastTo<smpp::CSMPPMessage *>(lpoMessage);
    ACL_ASSERT(lpoMessage != APL_NULL);
    //Allocate smpp message buffer

    ACL_NEW_ASSERT(lpoBlock, acl::CMemoryBlock(
        aoSession->GetService()->GetConfiguration()->GetWriteBufferSize() ) );

    if(lpoBlock->GetSpace()<1024)
    {
        lpoBlock->Resize(lpoBlock->GetSize() + 1024);
        lpoBlock->Reset();
    }

    if ( (liLength = lpoMessage->Encode(lpoBlock->GetWritePtr(), 1024)) == -1)
    {
        ACL_ASSERT(false);
    }

    lpoBlock -> SetWritePtr(liLength);

    aoWriteFuture->SetMessage(lpoBlock);
    
    apoNextFilter->FilterWrite(aoSession, aoWriteFuture);
}
    
CIoFilter* CSMPPMessageFilter::Clone(void)
{
    CSMPPMessageFilter* lpoFilter = APL_NULL;
    
    ACL_NEW_ASSERT(lpoFilter, CSMPPMessageFilter);
    
    return lpoFilter;
}

ANF_NAMESPACE_END
