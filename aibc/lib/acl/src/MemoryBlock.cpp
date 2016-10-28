
#include "acl/MemoryBlock.h"
#include "acl/stl/algorithm.h"

ACL_NAMESPACE_START

CDataBlock::CDataBlock( apl_size_t auSize )
    : mpcBase(NULL)
    , mpcEnd(NULL)
    , muCapacity(0)
    , muSize(0)
    , miReferCount(1)
    , miFlag(CMemoryBlock::DO_DELETE)
{
    this->Resize(auSize);
}

CDataBlock::CDataBlock( char const* apcData, apl_size_t auSize, apl_int_t aiFlag )
    : mpcBase(const_cast<char*>(apcData) )
    , mpcEnd(mpcBase + auSize)
    , muCapacity(auSize)
    , muSize(auSize)
    , miReferCount(1)
    , miFlag(aiFlag)
{
}

CDataBlock::~CDataBlock(void)
{
    if (this->miFlag == CMemoryBlock::DO_DELETE)
    {
        ACL_DELETE_N(this->mpcBase);
    }
}

void CDataBlock::SetBase( char const* apcData, apl_size_t auSize, apl_int_t aiFlag )
{
    if (this->miFlag == CMemoryBlock::DO_DELETE)
    {
        ACL_DELETE_N(this->mpcBase);
    }
    
    this->mpcBase = const_cast<char*>(apcData);
    this->mpcEnd  = this->mpcBase + auSize;
    this->muSize  = auSize;
    this->muCapacity = auSize;
    this->miFlag  = aiFlag;
}

char* CDataBlock::GetBase(void)
{
    return this->mpcBase;
}

char* CDataBlock::GetEnd(void)
{
    return this->mpcEnd;
}

char* CDataBlock::GetMark(void)
{
    return this->mpcBase + this->muSize;
}

apl_int_t CDataBlock::Resize( apl_size_t auSize )
{
    if (auSize <= this->muCapacity)
    {
        this->muSize = auSize;
    }
    else
    {
        char* lpcTmp = NULL;
        ACL_NEW_N_ASSERT( lpcTmp, char, auSize );
        
        if (this->mpcBase != NULL)
        {
            apl_memcpy( lpcTmp, this->mpcBase, this->muCapacity );
            if (this->miFlag == CMemoryBlock::DO_DELETE)
            {
                ACL_DELETE_N(this->mpcBase);
            }
        }

        this->mpcBase = lpcTmp;
        this->mpcEnd = this->mpcBase + auSize;
        this->muCapacity = auSize;
        this->muSize = auSize;
        this->miFlag = CMemoryBlock::DO_DELETE;
    }
    
    return 0;
}

apl_size_t CDataBlock::GetSize(void) const
{
    return this->muSize;
}

apl_size_t CDataBlock::GetCapacity(void) const
{
    return this->muCapacity;
}

CDataBlock* CDataBlock::Clone()
{
    CDataBlock* lpoDataBlock = this->CloneNoCopy();

    apl_memcpy(lpoDataBlock->mpcBase, this->mpcBase, this->muSize );
    
    lpoDataBlock->Resize(this->muSize);
    
    return lpoDataBlock;
}

CDataBlock* CDataBlock::CloneNoCopy()
{
    CDataBlock* lpoDataBlock = NULL;
    
    ACL_NEW_ASSERT(lpoDataBlock, CDataBlock(this->muCapacity) );
    
    return lpoDataBlock;
}

CDataBlock* CDataBlock::Duplicate()
{
    this->miReferCount++;
    
    return this;
}

CDataBlock* CDataBlock::Release()
{
    this->miReferCount--;
    
    if (this->miReferCount == 0)
    {
        return this;
    }
    else
    {
        return NULL;
    }
}

apl_int_t CDataBlock::GetReferCount()
{
    return this->miReferCount;
}

apl_int_t CDataBlock::GetFlag(void)
{
    return this->miFlag;
}

/////////////////////////////////////////// MemoryBlock ///////////////////////////////////
CMemoryBlock::CMemoryBlock(void)
    : mpoDataBlock(NULL)
{
    this->Initialize(0, NULL, 0, NULL, NULL);
}

CMemoryBlock::CMemoryBlock( CDataBlock* apoDataBlock, CMemoryBlock* apoCont )
    : mpoDataBlock(apoDataBlock)
{
    this->Initialize(0, NULL, 0, apoDataBlock, apoCont);
}

CMemoryBlock::CMemoryBlock( apl_size_t auSize, CMemoryBlock* apoCont )
    : mpoDataBlock(NULL)
{
    this->Initialize(auSize, NULL, 0, NULL, apoCont);
}

CMemoryBlock::CMemoryBlock( char const* apcData, apl_size_t auSize, EOption aeFlag, CMemoryBlock* apoCont )
    : mpoDataBlock(NULL)
{
    this->Initialize(auSize, apcData, aeFlag, NULL, apoCont);
}

CMemoryBlock::CMemoryBlock( CMemoryBlock const& aoMB )
    : mpoDataBlock(NULL)
{
    *this = aoMB;
}

apl_int_t CMemoryBlock::Initialize(
    apl_size_t auSize,
    char const* apcData,
    apl_int_t aiFlag,
    CDataBlock* apoDB,
    CMemoryBlock* apoCont )
{
    this->mpoDataBlock = apoDB;
    
    if (this->mpoDataBlock == NULL)
    {
        if (apcData == NULL)
        {
            ACL_NEW_ASSERT( this->mpoDataBlock, CDataBlock(auSize) );
        }
        else
        {
            ACL_NEW_ASSERT( this->mpoDataBlock, CDataBlock(apcData, auSize, aiFlag) );
        }
    }
    
    this->mpoCont = apoCont;
    this->muReadPos = 0;
    this->muWritePos = 0;
    this->mpoNext = NULL;
    this->mpoPrev = NULL;
    
    return 0;
}

CMemoryBlock::~CMemoryBlock(void)
{
    this->Release();
}

CMemoryBlock* CMemoryBlock::Clone(void)
{
    CMemoryBlock* lpoMemoryBlock = NULL;
    CDataBlock*   lpoDataBlock = NULL;

    lpoDataBlock = this->GetDataBlock()->Clone();
    
    ACL_NEW_ASSERT( lpoMemoryBlock, CMemoryBlock(lpoDataBlock) );
    
    lpoMemoryBlock->muReadPos  = this->muReadPos;
    lpoMemoryBlock->muWritePos = this->muWritePos;

    if ( this->mpoCont != NULL 
        && ( lpoMemoryBlock->mpoCont = this->mpoCont->Clone() ) == NULL )
    {
        ACL_DELETE(lpoMemoryBlock);
        return NULL;
    }
    
    return lpoMemoryBlock;
}

CMemoryBlock* CMemoryBlock::Duplicate(void)
{
    CMemoryBlock* lpoMemoryBlock = NULL;
    CDataBlock* lpoDataBlock = NULL;

    lpoDataBlock = this->GetDataBlock()->Duplicate();
    
    ACL_NEW_ASSERT( lpoMemoryBlock, CMemoryBlock(lpoDataBlock) );
    
    lpoMemoryBlock->muReadPos  = this->muReadPos;
    lpoMemoryBlock->muWritePos = this->muWritePos;
    
    if ( this->mpoCont != NULL 
        && ( lpoMemoryBlock->mpoCont = this->mpoCont->Duplicate() ) == NULL )
    {
        ACL_DELETE(lpoMemoryBlock);
        return NULL;
    }
    
    return lpoMemoryBlock;
}

void CMemoryBlock::operator = ( CMemoryBlock const& aoMB )
{
    CDataBlock* lpoDataBlock = NULL;

    //Release current datablock
    if (this->mpoDataBlock != NULL
        && (lpoDataBlock = this->mpoDataBlock->Release() ) != NULL)
    {
        ACL_DELETE(lpoDataBlock);
    }

    if (aoMB.GetDataBlock()->GetFlag() == DONT_DELETE)
    {
        this->Initialize(0, NULL, 0, aoMB.GetDataBlock()->Duplicate(), NULL);
    }
    else
    {
        this->Initialize(0, NULL, 0, aoMB.GetDataBlock()->Clone(), NULL);
    }
    
    this->muReadPos  = aoMB.muReadPos;
    this->muWritePos = aoMB.muWritePos;
}

void CMemoryBlock::Release(void)
{
    CDataBlock* lpoDataBlock = NULL;

    if ( this->mpoCont != NULL )
    {
        this->mpoCont->Release();
        ACL_DELETE(this->mpoCont);
    }
    
    if ( this->mpoDataBlock != NULL
        && (lpoDataBlock = this->mpoDataBlock->Release() ) != NULL )
    {
        ACL_DELETE(lpoDataBlock);
    }
    
    this->mpoDataBlock = NULL;
}

apl_ssize_t CMemoryBlock::Write( void const* apvBuffer, apl_size_t auN )
{
    if ( this->GetSpace() < auN )
    {
        apl_set_errno(APL_ENOSPC);
        return -1;
    }
    
    apl_memcpy( this->GetWritePtr(), apvBuffer, auN );
    
    this->SetWritePtr(auN);
    
    return auN;
}

apl_ssize_t CMemoryBlock::Write( char const* apcBuffer )
{
    apl_size_t luLen = apl_strlen(apcBuffer) + 1;
    
    if ( this->GetSpace() < luLen )
    {
        apl_set_errno(APL_ENOSPC);
        return -1;
    }
    
    apl_memcpy( this->GetWritePtr(), apcBuffer, luLen );
    
    this->SetWritePtr(luLen);
    
    return luLen;
}

apl_ssize_t CMemoryBlock::Read( void* apvBuffer, apl_size_t auN )
{
    apl_size_t luCurrBuffLen = this->GetLength();
    
    if ( luCurrBuffLen == 0 )
    {
        apl_set_errno(APL_ENODATA);
        return -1;
    }
    else if ( luCurrBuffLen < auN )
    {
        apl_memcpy( apvBuffer, this->GetReadPtr(), luCurrBuffLen );
        
        this->SetReadPtr(luCurrBuffLen);
        
        return luCurrBuffLen;
    }
    else
    {
        apl_memcpy( apvBuffer, this->GetReadPtr(), auN );
        
        this->SetReadPtr(auN);
        
        return auN;
    }
}

void CMemoryBlock::Reset(void)
{
    this->SetWritePtr( this->GetBase() );
    this->SetReadPtr( this->GetBase() );
}

char* CMemoryBlock::GetBase(void) const
{
    return this->mpoDataBlock->GetBase();
}

void  CMemoryBlock::SetBase(const char* apcData, apl_size_t auSize, EOption aeFlag)
{
    this->mpoDataBlock->SetBase(apcData, auSize, aeFlag);
}

char* CMemoryBlock::GetEnd(void) const
{
    return this->mpoDataBlock->GetEnd();
}

char* CMemoryBlock::GetReadPtr(void) const
{
    return this->GetBase() + this->muReadPos;
}

void  CMemoryBlock::SetReadPtr(char* apcPtr)
{
    this->muReadPos = apcPtr - this->GetBase();
}

void  CMemoryBlock::SetReadPtr(apl_size_t auN)
{
    this->muReadPos += auN;
}

char* CMemoryBlock::GetWritePtr(void) const
{
    return this->GetBase() + this->muWritePos;
}

void  CMemoryBlock::SetWritePtr(char* apcPtr)
{
    this->muWritePos = apcPtr - this->GetBase();
}

void  CMemoryBlock::SetWritePtr(apl_size_t auN)
{
    this->muWritePos += auN;
}

apl_size_t CMemoryBlock::GetSize(void) const
{
    return this->mpoDataBlock->GetSize();
}

apl_size_t CMemoryBlock::GetLength(void) const
{
    return this->muWritePos - this->muReadPos;
}

void CMemoryBlock::SetLength(apl_size_t auLength)
{
    this->muWritePos = this->muReadPos + auLength;
}

void CMemoryBlock::Resize(apl_size_t auSize)
{
    this->mpoDataBlock->Resize(auSize);
}

apl_size_t CMemoryBlock::GetCapacity(void) const
{
    return this->mpoDataBlock->GetCapacity();
}

apl_size_t CMemoryBlock::GetSpace(void) const
{
    return this->mpoDataBlock->GetMark() - this->GetWritePtr();
}

apl_size_t CMemoryBlock::GetTotalLength(void) const
{
    if ( this->GetCont() != 0 )
    {
        return this->GetLength() + this->GetCont()->GetLength();
    }
    else
    {
        return this->GetLength();
    }
}

apl_size_t CMemoryBlock::GetTotalSize(void) const
{
    if ( this->GetCont() != 0 )
    {
        return this->GetSize() + this->GetCont()->GetSize();
    }
    else
    {
        return this->GetSize();
    }
}

apl_size_t CMemoryBlock::GetTotalCapacity(void) const
{
    if ( this->GetCont() != 0 )
    {
        return this->GetCapacity() + this->GetCont()->GetCapacity();
    }
    else
    {
        return this->GetCapacity();
    }
}

CDataBlock* CMemoryBlock::GetDataBlock(void) const
{
    return this->mpoDataBlock;
}

CDataBlock* CMemoryBlock::SetDataBlock(CDataBlock* apoDB)
{
    CDataBlock* lpoBak = this->mpoDataBlock;
    
    this->mpoDataBlock = apoDB;
    
    this->SetWritePtr( apoDB->GetBase() );
    this->SetReadPtr( apoDB->GetBase() );
    
    return lpoBak;
}

void CMemoryBlock::SwapDataBlock( CMemoryBlock& aoMB )
{
    std::swap(this->mpoDataBlock, aoMB.mpoDataBlock);
    std::swap(this->muReadPos, aoMB.muReadPos);
    std::swap(this->muWritePos, aoMB.muWritePos);
}

CMemoryBlock* CMemoryBlock::GetCont(void) const
{
    return this->mpoCont;
}

void CMemoryBlock::SetCont(CMemoryBlock* apoMB)
{
    this->mpoCont = apoMB;
}

CMemoryBlock* CMemoryBlock::GetNext(void) const
{
    return this->mpoNext;
}

void CMemoryBlock::SetNext(CMemoryBlock* apoMB)
{
    this->mpoNext = apoMB;
}

CMemoryBlock* CMemoryBlock::GetPrev(void) const
{
    return this->mpoPrev;
}

void CMemoryBlock::SetPrev(CMemoryBlock* apoMB)
{
    this->mpoPrev = apoMB;
}

apl_int_t CMemoryBlock::GetReferCount(void) const
{
    return this->mpoDataBlock->GetReferCount();
}

ACL_NAMESPACE_END
