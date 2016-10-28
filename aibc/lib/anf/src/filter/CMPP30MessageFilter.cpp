/**
 * @file CMPP30MessageFilter.cpp
 */

#include "anf/filter/CMPP30MessageFilter.h"
#include "anf/IoService.h"

ANF_NAMESPACE_START

CCMPP30MessageFilter::CCMPP30MessageFilter(void)
    : mpoMessage(NULL)
    , moMessage(mpoMessage)
{
}

CCMPP30MessageFilter::~CCMPP30MessageFilter(void)
{
    if(mpoMessage != APL_NULL)
    {
        ACL_DELETE(mpoMessage);
    }
}

void CCMPP30MessageFilter::MessageReceived(
    IoNextFilter* apoNextFilter,
    SessionPtrType& aoSession,
    acl::CAny const& aoMessage )
{
    acl::CMemoryBlock* lpoBlock = APL_NULL;

    //Cast to memoryblock
    aoMessage.CastTo<acl::CMemoryBlock*>(lpoBlock);
    ACL_ASSERT(lpoBlock != APL_NULL);

    while(true)
    {
        if(this->mpoMessage == NULL)
        {
            ACL_NEW_ASSERT(this->mpoMessage, cmpp::CCMPP30Message);
        }

        //Decode all message
        apl_ssize_t liResult = this->mpoMessage->Decode(lpoBlock->GetReadPtr(), lpoBlock->GetLength());
        if (liResult > 0 && liResult <= (apl_ssize_t)lpoBlock->GetSize())
        {
            lpoBlock->SetReadPtr(liResult);

            cmpp::CCMPP30Message** lppoTemp = this->moMessage.CastTo<cmpp::CCMPP30Message*>();
            ACL_ASSERT(lppoTemp != NULL);

            *lppoTemp = this->mpoMessage;

            apoNextFilter->MessageReceived(aoSession, this->moMessage);

            this->mpoMessage = NULL;

            continue;
        }
        else if(liResult == 0)
        {
            //else if == 0, break and waiting for next MessageReceived
            break;
        }

        //Decode failed
        apoNextFilter->MessageReceived(aoSession, aoMessage);

        break;
    }

}

void CCMPP30MessageFilter::FilterWrite(
    IoNextFilter* apoNextFilter,
    SessionPtrType& aoSession,
    CWriteFuture::PointerType& aoWriteFuture )
{
    apl_int32_t        liLength = 0;
    cmpp::CCMPP30Message  *lpoMessage = APL_NULL;
    acl::CMemoryBlock* lpoBlock = APL_NULL;

    aoWriteFuture->GetMessage().CastTo<cmpp::CCMPP30Message *>(lpoMessage);
    ACL_ASSERT(lpoMessage != APL_NULL);

    //Allocate cmpp message buffer
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

CIoFilter* CCMPP30MessageFilter::Clone(void)
{
    CCMPP30MessageFilter* lpoFilter = APL_NULL;

    ACL_NEW_ASSERT(lpoFilter, CCMPP30MessageFilter);

    return lpoFilter;
}

ANF_NAMESPACE_END
