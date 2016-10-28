
#include "acl/FixMemoryPool.h"

ACL_NAMESPACE_START

CFixMemoryPool::CFixMemoryPool(void)
    : muBlocksAvailable(0)
    , muFirstAvailableBlock(0)
    , mpcData(NULL)
    , mbIsDoDelete(false)
    , muBlockSize(0)
    , muBlockCount(0)
{
}

CFixMemoryPool::~CFixMemoryPool(void)
{
    this->Release();
}

apl_int_t CFixMemoryPool::Initialize( apl_size_t aiBlockSize, apl_size_t aiBlockCount )
{
    unsigned char* lpcPtr = NULL;
    apl_size_t liBlockSize = ACL_DOALIGN(aiBlockSize, 4);
        
    if (aiBlockCount > APL_INT32_MAX)
    {
        apl_set_errno(APL_EINVAL);
        return -1;
    }
    
    if (this->mpcData != NULL)
    {
        apl_set_errno(APL_EBUSY);
        return -1;
    }

    ACL_NEW_N_ASSERT( lpcPtr, unsigned char, aiBlockCount * liBlockSize );
    
    return this->Initialize(lpcPtr, liBlockSize, aiBlockCount, true);
}

apl_int_t CFixMemoryPool::Initialize( void* apvBuffer, apl_size_t aiSize, apl_size_t aiBlockSize, apl_size_t aiBlockCount )
{
    apl_size_t liBlockSize = ACL_DOALIGN(aiBlockSize, 4);
    
    if (aiBlockCount > APL_INT32_MAX 
        || aiSize < aiBlockCount * liBlockSize)
    {
        apl_set_errno(APL_EINVAL);
        return -1;
    }
    
    if (this->mpcData != NULL)
    {
        apl_set_errno(APL_EBUSY);
        return -1;
    }
    
    return this->Initialize(apvBuffer, aiBlockSize, aiBlockCount, false);
}

apl_int_t CFixMemoryPool::Initialize( void* apvBuffer, apl_size_t aiBlockSize, apl_size_t aiBlockCount, bool abIsDoDelete )
{
    this->mpcData = reinterpret_cast<unsigned char*>(apvBuffer);
    
    this->mbIsDoDelete = abIsDoDelete;
    this->muBlocksAvailable = aiBlockCount;
    this->muBlockSize = aiBlockSize;
    this->muBlockCount = aiBlockCount;
    this->muFirstAvailableBlock = 0;
    
    this->Reset();
    
    return 0;
}
    
void* CFixMemoryPool::Allocate( apl_size_t aiSize )
{
    if (aiSize <= 0 || this->muBlockSize < aiSize)
    {
        //Invalid param 
        apl_set_errno(APL_EINVAL);
        return NULL;
    }
    
    if (this->muBlocksAvailable <= 0)
    {
        return NULL;
    }

    unsigned char* lpcResult = 
        this->mpcData + ( this->muFirstAvailableBlock * this->muBlockSize );
		
    this->muFirstAvailableBlock = *reinterpret_cast<apl_int32_t*>(lpcResult);

    --this->muBlocksAvailable;

    return lpcResult;
}

void CFixMemoryPool::Deallocate( void* apvPtr )
{
    unsigned char* lpcToRelease = static_cast<unsigned char*>(apvPtr);

    ACL_ASSERT( (lpcToRelease - this->mpcData) % this->muBlockSize == 0 );

    *reinterpret_cast<apl_int32_t*>(lpcToRelease) = this->muFirstAvailableBlock;

    this->muFirstAvailableBlock = (lpcToRelease - this->mpcData) / this->muBlockSize;

    ++this->muBlocksAvailable;
}

void CFixMemoryPool::Release()
{
    if (this->mbIsDoDelete)
    {
        ACL_DELETE_N(this->mpcData);
    }
    else
    {
        this->mpcData = NULL;
    }
    
    this->muFirstAvailableBlock = 0;
    this->muBlockSize = 0;
    this->muBlocksAvailable = 0;
}
    
void CFixMemoryPool::Reset()
{
    unsigned char* lpcPtr = this->mpcData;

    this->muBlocksAvailable = this->muBlockCount;
    this->muFirstAvailableBlock = 0;

    for( apl_size_t liN = 0; liN < this->muBlockCount; lpcPtr += this->muBlockSize )
    {
        *reinterpret_cast<apl_int32_t*>(lpcPtr) = ++liN;
    }
}
    
apl_size_t CFixMemoryPool::GetSize(void)
{
    return (this->muBlockCount - this->muBlocksAvailable) * this->muBlockSize;
}
    
apl_size_t CFixMemoryPool::GetSpace(void)
{
    return this->muBlocksAvailable * this->muBlockSize;
}

ACL_NAMESPACE_END
