#include "apl/apl.h"
#include "anf/IoFilter.h"
#include "anf/IoService.h"
#include "anf/SimpleObjectPool.h"
#include "anf/filter/BencodingMessageFilter.h"
#include "acl/IndexDict.h"
#include "acl/MemoryBlock.h"
#include "acl/MemoryStream.h"

#include "anf/IoConnector.h"
#include "anf/IoAcceptor.h"
#include "anf/protocol/BencodingMessage.h"

ANF_NAMESPACE_START
#define BSIZE (64)

CBencodingMessageFilter::CBencodingMessageFilter()
    :mpoIndexDict(APL_NULL)
    ,moMessage(mpoIndexDict)
{
}


CBencodingMessageFilter::~CBencodingMessageFilter()
{
    ACL_DELETE(mpoIndexDict);
}

void CBencodingMessageFilter::SessionIdle(
        IoNextFilter* apoNextFilter,
        SessionPtrType& aoSession,
        apl_int_t aiStatus )
{
    CIoConnector* lpoConnector = dynamic_cast<CIoConnector*>(aoSession->GetService());
    if(lpoConnector != APL_NULL)
    {
        if (aiStatus == SESSION_WRITE_IDLE)
        {
            acl::CIndexDict loHeartBeat;

            loHeartBeat.Set(BENC_TAG_SEQ_NO, -1);
            loHeartBeat.Set(BENC_TAG_CMD_ID, -1);

            aoSession->Write(aoSession, &loHeartBeat);
        }
        else if(aiStatus == SESSION_READ_IDLE)
        {
            apl_errprintf("[Connector] Bencoding message filter closing a read idle timedout session, (Local=%s:%"APL_PRIdINT",Remote=%s:%"APL_PRIdINT")\n", 
                    aoSession->GetLocalAddress().GetIpAddr(), 
                    (apl_int_t)aoSession->GetLocalAddress().GetPort(),
                    aoSession->GetRemoteAddress().GetIpAddr(), 
                    (apl_int_t)aoSession->GetRemoteAddress().GetPort() );

            aoSession->Close(aoSession);
        }

        apoNextFilter->SessionIdle(aoSession, aiStatus);
    }

    CIoAcceptor* lpoAcceptor = dynamic_cast<CIoAcceptor*>(aoSession->GetService());
    if(lpoAcceptor != APL_NULL)
    {
        if (aiStatus == SESSION_READ_IDLE)
        {
            apl_errprintf("[Acceptor] Bencoding message filter closing a read idle timedout session, (Local=%s:%"APL_PRIdINT",Remote=%s:%"APL_PRIdINT"))\n", 
                    aoSession->GetLocalAddress().GetIpAddr(), 
                    (apl_int_t)aoSession->GetLocalAddress().GetPort(),
                    aoSession->GetRemoteAddress().GetIpAddr(),
                    (apl_int_t)aoSession->GetRemoteAddress().GetPort() );

            aoSession->Close(aoSession);
        }

        apoNextFilter->SessionIdle(aoSession, aiStatus);
    }
}

void CBencodingMessageFilter::MessageReceived(
    IoNextFilter* apoNextFilter,
    SessionPtrType& aoSession,
    acl::CAny const& aoMessage )
{
    acl::CMemoryBlock*  lpoBlock = APL_NULL;

    //Cast to memoryblock
    aoMessage.CastTo<acl::CMemoryBlock*>(lpoBlock);    
    ACL_ASSERT(lpoBlock != APL_NULL);
    acl::CMemoryStream loMS(lpoBlock);
    char* lpcLastReadPtr = lpoBlock->GetReadPtr();

    ACL_ASSERT(lpoBlock->GetReadPtr() == lpoBlock->GetBase());

    while(true)
    {
        apl_uint32_t lu32Len;

        if (loMS.ReadToH(lu32Len) != sizeof(lu32Len))
        {
            break;
        }

        //apl_errprintf(">>pkt_len:%"APL_PRIu32"\n",lu32Len);

        if (lu32Len + sizeof(lu32Len) > lpoBlock->GetSize()) // bigger then buffer
        {
            apoNextFilter->MessageReceived(aoSession, aoMessage); 
            //aoSession->Close(aoSession);
            break;
        }

        //apl_errprintf(">>blk_len:%"APL_PRIuINT"\n",lpoBlock->GetLength());

        if (lpoBlock->GetLength() < lu32Len)
        {
            break;
        }

        //apl_errprintf(">>ben_data:%.*s\n",lu32Len, lpoBlock->GetReadPtr());
        
        if (mpoIndexDict == APL_NULL)
        {
            ACL_NEW_ASSERT(mpoIndexDict, acl::CIndexDict(BSIZE));
        }

        if (mpoIndexDict->Decode(lpoBlock) < 0)
        {
            apoNextFilter->MessageReceived(aoSession, aoMessage); 
            break;
        }

        if((apl_int_t)mpoIndexDict->GetInt(BENC_TAG_SEQ_NO) == (apl_int_t)-1 
                && (apl_int_t)mpoIndexDict->GetInt(BENC_TAG_CMD_ID) == (apl_int_t)-1)
        {
            CIoAcceptor* lpoAcceptor = dynamic_cast<CIoAcceptor*>(aoSession->GetService());
            if(lpoAcceptor != APL_NULL)
            {
                aoSession->Write(aoSession, mpoIndexDict);

                // HeartBeat
                ACL_DELETE(mpoIndexDict);

                lpcLastReadPtr = lpoBlock->GetReadPtr(); 

                continue;
            }

            CIoConnector* lpoConnector = dynamic_cast<CIoConnector*>(aoSession->GetService());
            if(lpoConnector != APL_NULL)
            {
                // HeartBeatResp
                ACL_DELETE(mpoIndexDict);

                lpcLastReadPtr = lpoBlock->GetReadPtr();

                continue;
            }
        }

        acl::CIndexDict** lppoTemp = this->moMessage.CastTo<acl::CIndexDict*>();
        ACL_ASSERT(lppoTemp != APL_NULL);
                           
        *lppoTemp = mpoIndexDict;
                                        
        apoNextFilter->MessageReceived(aoSession, moMessage);
                                                     
        this->mpoIndexDict = APL_NULL;
                                                                  
        lpcLastReadPtr = lpoBlock->GetReadPtr(); 
    }

    lpoBlock->SetReadPtr(lpcLastReadPtr); 
}

void CBencodingMessageFilter::FilterWrite(
    IoNextFilter* apoNextFilter,
    SessionPtrType& aoSession,
    CWriteFuture::PointerType& aoWriteFuture )
{
    acl::CIndexDict* lpoMessage = APL_NULL;
    acl::CMemoryBlock*   lpoBlock = APL_NULL;
        
    aoWriteFuture->GetMessage().CastTo<acl::CIndexDict*>(lpoMessage);
    ACL_ASSERT(lpoMessage != APL_NULL);
    
    //Allocate scp message buffer
    ACL_NEW_ASSERT(lpoBlock, acl::CMemoryBlock(aoSession->GetService()->GetConfiguration()->GetWriteBufferSize()));
    //lpoBlock = SimpleObjectPoolInstance<acl::CMemoryBlock>()->Allocate();
    //lpoBlock->Resize(aoSession->GetService()->GetConfiguration()->GetWriteBufferSize() );
    acl::CMemoryStream loMS(lpoBlock);
    char* lpcWritePtr;

    while (true)
    {

        lpcWritePtr = lpoBlock->GetWritePtr();

        loMS.WriteToN((apl_uint32_t)0);

        if (lpoMessage->Encode(lpoBlock) == 0)
        {
            break;
        }

        lpoBlock->Resize(lpoBlock->GetSize()*2);
        lpoBlock->Reset();
        continue;
    }
  
    //apl_errprintf("<<pkt_data:%.*s\n", lpoBlock->GetLength(), lpoBlock->GetReadPtr() + 4);

    char* lpcWritePtr2 = lpoBlock->GetWritePtr();
    apl_size_t luTotalLength = lpcWritePtr2 - lpcWritePtr;
    apl_uint32_t lu32DataLength = luTotalLength - 4;

    //apl_errprintf("<<pkt_len:%"APL_PRIu32"\n", lu32DataLength);

    lpoBlock->SetWritePtr(lpcWritePtr);
    loMS.WriteToN(lu32DataLength);
    lpoBlock->SetWritePtr(lpcWritePtr2);

    aoWriteFuture->SetMessage(lpoBlock);
    
    apoNextFilter->FilterWrite(aoSession, aoWriteFuture);
}
    
CIoFilter* CBencodingMessageFilter::Clone(void)
{
    CBencodingMessageFilter* lpoFilter = APL_NULL;
    
    ACL_NEW_ASSERT(lpoFilter, CBencodingMessageFilter);
    
    return lpoFilter;
}

ANF_NAMESPACE_END

