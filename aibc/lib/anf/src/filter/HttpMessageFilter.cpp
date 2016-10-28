
#include "anf/IoFilter.h"
#include "anf/IoService.h"
#include "anf/SimpleObjectPool.h"
#include "anf/protocol/HttpMessage.h"
#include "anf/filter/HttpMessageFilter.h"

ANF_NAMESPACE_START

void CHttpMessageFilter::MessageReceived(
    IoNextFilter* apoNextFilter,
    SessionPtrType& aoSession,
    acl::CAny const& aoMessage )
{
    acl::CMemoryBlock* lpoBlock = NULL;
    http::CHttpMessage*  lpoMessage = NULL;
    char* lpcReadPtr = APL_NULL;
    apl_size_t luStLnLen = 0;
    apl_int_t liHeaderLen = 0;
    apl_int_t liErrStatus = 0;
    apl_size_t luLenght = 0;
    apl_size_t luSize = 0;
    apl_size_t luNewSize = 0;

    //Cast to memoryblock
    aoMessage.CastTo<acl::CMemoryBlock*>(lpoBlock);    
    ACL_ASSERT(lpoBlock != NULL);
    
    while (true)
    {
        luStLnLen = 0;
        liHeaderLen = 0;
        lpoMessage = APL_NULL;
        liErrStatus = 0;

        //save memblock
        lpcReadPtr = lpoBlock->GetReadPtr();

        if ((lpoMessage = http::DecodeStartLine(lpoBlock, &luStLnLen)) != APL_NULL)
        {
            if (lpoMessage->GetMsgType() == http::MESSAGE_TYPE_HTTP_INVALID)
            {
                liErrStatus = lpoMessage->GetErrCode();
                goto HTTP_RECV_INVALID;
            }

            liHeaderLen = lpoMessage->DecodeHeader(lpoBlock);
            liErrStatus = liHeaderLen;

            //not enough content to decode header
            if (liHeaderLen == 0) 
            { 
                break; 
            }
            else if (liHeaderLen < 0)
            {
                goto HTTP_RECV_INVALID;
            }

            liErrStatus = lpoMessage->DecodeBody(lpoBlock);

            if (liErrStatus < 0)
            {
                goto HTTP_RECV_INVALID;
            }
            //not enough content to decode body
            else if (liErrStatus == 0)
            {
                break;
            }

            acl::CAny loMessage(lpoMessage);
            apoNextFilter->MessageReceived(aoSession, loMessage);
            continue;
        }

        break;
    }

    //restore memblock
    if (liErrStatus == 0)
    {
        if (lpoMessage != APL_NULL)
        {
            ACL_DELETE(lpoMessage);
        }

        lpoBlock->SetReadPtr(lpcReadPtr);
    }

    luLenght = lpoBlock->GetLength();
    luSize = lpoBlock->GetSize();

    if (luLenght > 0)
    {
        apl_memmove(lpoBlock->GetBase(), lpoBlock->GetReadPtr(), luLenght);
        lpoBlock->Reset();
        lpoBlock->SetWritePtr(luLenght);

        apl_memset(lpoBlock->GetWritePtr(), 0, lpoBlock->GetSpace());
    }

    //memblock full
    if (lpoBlock->GetSpace() < 1)
    {
        luNewSize = luSize << 1;

        if (luNewSize > liHeaderLen + luStLnLen + ANF_HTTP_LIMIT_BODY_SIZE)
        {
            luNewSize = liHeaderLen + luStLnLen + ANF_HTTP_LIMIT_BODY_SIZE;
        }

        if (luNewSize <= luSize)
        {
            //already exceed package limit, pass a invalid msg to next
            ACL_NEW_ASSERT(lpoMessage, (http::CHttpInvalid));
            acl::CAny loMessage(lpoMessage);
            apoNextFilter->MessageReceived(aoSession, loMessage);
            lpoBlock->Reset();
            return;
        }

        lpoBlock->Resize(luNewSize);
    }

    return;

HTTP_RECV_INVALID:

    //pass invalid msg to next filter
    if (lpoMessage != APL_NULL && (lpoMessage->GetMsgType() != http::MESSAGE_TYPE_HTTP_INVALID))
    {
        ACL_DELETE(lpoMessage);
        lpoMessage = APL_NULL;
    }

    if (lpoMessage == APL_NULL)
    {
        ACL_NEW_ASSERT(lpoMessage, (http::CHttpInvalid));
    }

    lpoMessage->SetErrCode(liErrStatus);
    
    acl::CAny loMessage(lpoMessage);
    apoNextFilter->MessageReceived(aoSession, loMessage);
    lpoBlock->Reset();

    return;
}

void CHttpMessageFilter::FilterWrite(
    IoNextFilter* apoNextFilter,
    SessionPtrType& aoSession,
    CWriteFuture::PointerType& aoWriteFuture )
{
    http::CHttpMessage*  lpoMessage = APL_NULL;
    acl::CMemoryBlock* lpoBlock = APL_NULL;
    apl_int_t liStatus = 0;
    apl_size_t luSize = 0;

    aoWriteFuture->GetMessage().CastTo<http::CHttpMessage*>(lpoMessage);
    ACL_ASSERT(lpoMessage != APL_NULL);

    //Allocate dsmp message buffer
    luSize = aoSession->GetService()->GetConfiguration()->GetWriteBufferSize();

    lpoBlock = anf::SimpleObjectPoolInstance<acl::CMemoryBlock>()->Allocate();
    lpoBlock->Resize(luSize);
    lpoBlock->Reset();

    liStatus = lpoMessage->EncodeMessage(lpoBlock);

    while (liStatus == ANF_HTTP_ERROR_TOOBIG 
            && luSize < ANF_HTTP_LIMIT_BODY_SIZE)
    {
        luSize = luSize << 1;
        lpoBlock->Resize(luSize);
        lpoBlock->Reset();
        liStatus = lpoMessage->EncodeMessage(lpoBlock);
    }

    if (liStatus < 0)
    {
        //encode failed, fire a message sent error
        aoWriteFuture->Exception(-1, -1);
        anf::SimpleObjectPoolInstance<acl::CMemoryBlock>()->Destroy(lpoBlock);
        return;
    }

    aoWriteFuture->SetMessage(lpoBlock);
    
    apoNextFilter->FilterWrite(aoSession, aoWriteFuture);
    
    return;
}
    
CIoFilter* CHttpMessageFilter::Clone(void)
{
    CHttpMessageFilter* lpoFilter = NULL;
    
    ACL_NEW_ASSERT(lpoFilter, CHttpMessageFilter);
    
    return lpoFilter;
}

ANF_NAMESPACE_END
