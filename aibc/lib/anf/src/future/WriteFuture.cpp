
#include "anf/future/WriteFuture.h"
#include "anf/SimpleObjectPool.h"
#include "acl/MemoryBlock.h"

ANF_NAMESPACE_START

CWriteFuture::~CWriteFuture(void)
{
    acl::CMemoryBlock* lpoBlock = NULL;

    if (this->moMessage.CastTo<acl::CMemoryBlock*>(lpoBlock) )
    {
        SimpleObjectPoolInstance<acl::CMemoryBlock>()->Destroy(lpoBlock);
    }
}

acl::CAny const& CWriteFuture::GetMessage(void) const
{
    return this->moMessage;
}

acl::CSockAddr const& CWriteFuture::GetRemoteAddress(void) const
{
    return this->moRemoteAddress;
}

bool CWriteFuture::IsHasRemoteAddress(void) const
{
    return this->mbIsHasRemoteAddress;
}

void* CWriteFuture::GetAct(void) const
{
    return this->mpvAct;
}

acl::CTimestamp const& CWriteFuture::GetTimestamp(void) const
{
    return this->moTimestamp;
}

apl_size_t CWriteFuture::GetFlushCount(void) const
{
    return this->muFlushCount;
}

void CWriteFuture::IncreaseFlushCount( apl_size_t auCount ) 
{
    this->muFlushCount += auCount;
}

ANF_NAMESPACE_END
