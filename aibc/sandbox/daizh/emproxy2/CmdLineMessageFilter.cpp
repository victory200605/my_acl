
#include "CmdLineMessageFilter.h"
#include "anf/IoSession.h"
#include "anf/IoService.h"

CCmdLineMessageFilter::CCmdLineMessageFilter(void)
    : mpoMessage(APL_NULL)
    , moMessage(mpoMessage)
{
}


CCmdLineMessageFilter::~CCmdLineMessageFilter(void)
{
    ACL_DELETE(this->mpoMessage);
}

void CCmdLineMessageFilter::MessageReceived(
    anf::IoNextFilter* apoNextFilter,
    anf::SessionPtrType& aoSession,
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
            ACL_NEW_ASSERT(this->mpoMessage, CCmdLineMessage);
        }

        //Decode all message
        apl_int_t liRetCode = this->mpoMessage->Decode(lpoBlock);
        if (liRetCode == 0)
        {
            CCmdLineMessage** lppoTemp = this->moMessage.CastTo<CCmdLineMessage*>();
            ACL_ASSERT(lppoTemp != NULL);
            
            *lppoTemp = this->mpoMessage;
            
            apoNextFilter->MessageReceived(aoSession, this->moMessage);
            
            this->mpoMessage = NULL;
            
            continue;
        }
        else if (liRetCode > 0)
        {
            apoNextFilter->MessageReceived(aoSession, aoMessage);

            break;
        }
        
        break;
    }
}

void CCmdLineMessageFilter::FilterWrite(
        anf::IoNextFilter* apoNextFilter,
        anf::SessionPtrType& aoSession,
        anf::CWriteFuture::PointerType& aoWriteFuture )
{
    CCmdLineMessage* lpoMessage = NULL;
    acl::CMemoryBlock* lpoBlock = NULL;
        
    aoWriteFuture->GetMessage().CastTo<CCmdLineMessage*>(lpoMessage);
    ACL_ASSERT(lpoMessage != NULL);
    
    apl_size_t luBufferSize = aoSession->GetService()->GetConfiguration()->GetWriteBufferSize();

    //Allocate scp message buffer
    ACL_NEW_ASSERT(lpoBlock, acl::CMemoryBlock(luBufferSize) );
        
    if (lpoMessage->Encode(lpoBlock) != 0)
    {
        ACL_ASSERT(false);
    }
    
    aoWriteFuture->SetMessage(lpoBlock);
    
    apoNextFilter->FilterWrite(aoSession, aoWriteFuture);
}
    
anf::CIoFilter* CCmdLineMessageFilter::Clone(void)
{
    CCmdLineMessageFilter* lpoFilter = NULL;
    
    ACL_NEW_ASSERT(lpoFilter, CCmdLineMessageFilter);
    
    return lpoFilter;
}

