#include "anf/protocol/SMMCMessage.h"
#include "anf/filter/SMMCMessageFilter.h"
#include "anf/IoService.h"
#include "anf/IoFilter.h"
#include "anf/IoService.h"

ANF_NAMESPACE_START

void CSMMCMessageFilter::MessageReceived(
    IoNextFilter* apoNextFilter,
    SessionPtrType& aoSession,
    acl::CAny const& aoMessage )
{
    acl::CMemoryBlock* lpoBlock = APL_NULL;
    smmc::CSMMCMessage loMessage;
    apl_int_t liLength;
    //Cast to memoryblock
    aoMessage.CastTo<acl::CMemoryBlock*>(lpoBlock);
    ACL_ASSERT(lpoBlock != APL_NULL);
    
    while(true)
    { 
        //Decode all message
        if ( (liLength = loMessage.Decode(lpoBlock->GetReadPtr(), lpoBlock->GetLength()) ) != -1)
        {
            lpoBlock->SetReadPtr(liLength);

            acl::CAny loAny(&loMessage);
            apoNextFilter->MessageReceived(aoSession, loAny);
            
            continue;
        }
        
        break;
    }

    if (liLength > 0)
    {
        apl_memmove(lpoBlock->GetBase(), lpoBlock->GetReadPtr(), liLength);
        lpoBlock->Reset();
        lpoBlock->SetWritePtr(liLength);
    }
}

void CSMMCMessageFilter::FilterWrite(
    IoNextFilter* apoNextFilter,
    SessionPtrType& aoSession,
    CWriteFuture::PointerType& aoWriteFuture )
{
    apl_int32_t        liLength = 0;
    smmc::CSMMCMessage  *lpoMessage = APL_NULL;
    acl::CMemoryBlock* lpoBlock = APL_NULL;
    
    aoWriteFuture->GetMessage().CastTo<smmc::CSMMCMessage *>(lpoMessage);
    ACL_ASSERT(lpoMessage != APL_NULL);
    //Allocate smmc message buffer

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
    
CIoFilter* CSMMCMessageFilter::Clone(void)
{
    CSMMCMessageFilter* lpoFilter = APL_NULL;
    
    ACL_NEW_ASSERT(lpoFilter, CSMMCMessageFilter);
    
    return lpoFilter;
}

ANF_NAMESPACE_END
