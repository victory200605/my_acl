
#include "VLRPage.h"

AIBC_GFQ_NAMESPACE_START

CVLRPage::CPageHeader::CPageHeader(apl_int32_t aiReadOffset, apl_int32_t aiWriteOffset)
    : miReadOffset(aiReadOffset)
    , miWriteOffset(aiWriteOffset)
{
}

CVLRPage::CRecordHeader::CRecordHeader(apl_int32_t aiTimestamp, apl_int32_t aiLength)
    : miTimestamp(aiTimestamp)
    , miLength(aiLength)
{
}

CVLRPage::CVLRPage( CVLRStorage* apoStorage, CVLRBufferPool* apoBufferPool, apl_int_t aiPageID )
    : mpoStorage(apoStorage)
    , mpoBufferPool(apoBufferPool)
    , miPageID(aiPageID)
    , miPageSize(apoStorage->GetPageSize() )
{
}

apl_int_t CVLRPage::Init(void)
{
    apl_ssize_t liResult = 0;

    this->moHeader.miReadOffset = sizeof(CPageHeader);
    this->moHeader.miWriteOffset = sizeof(CPageHeader);

    //Initialize page header field
    if ( (liResult = this->mpoStorage->Write(this->miPageID, 0, &this->moHeader, sizeof(this->moHeader) ) ) < 0) 
    {
        apl_errprintf(
            "Page init fail,%s (PageID=%"APL_PRIdINT")\n",
            apl_strerror(apl_get_errno() ),
            (apl_int_t)this->miPageID
        );

        return ERROR_WRITE_FILE;
    }
    
    ACL_ASSERT(liResult == (apl_ssize_t)sizeof(this->moHeader) );
    
    //Pre-allocate page buffer in buffer pool
    this->mpoBufferPool->Preallocate(this->miPageID, &this->moHeader, sizeof(this->moHeader) );
    
    return 0;
}
    
apl_int_t CVLRPage::Free(void)
{
    //Release cache page from buffer pool
    this->mpoBufferPool->Erase(this->miPageID);
    
    return 0;
}
    
apl_int_t CVLRPage::Open(void)
{
    apl_ssize_t liResult = 0;

    //Read page header from buffer pool only
    if ( (liResult = this->mpoBufferPool->Read(
        this->miPageID,
        0,
        &this->moHeader,
        sizeof(this->moHeader) ) ) < 0)
    {
        return liResult;
    }
    
    ACL_ASSERT(liResult == (apl_ssize_t)sizeof(this->moHeader) );
    
    return 0;
}

apl_ssize_t CVLRPage::Check(void)
{
    static acl::CMemoryBlock sloBuffer;

    apl_ssize_t liResult = 0;
    char* lpoBuffer = APL_NULL;

    sloBuffer.Resize(this->miPageSize);

    lpoBuffer = sloBuffer.GetBase();

    //Read page header from file directly
    if ( (liResult = this->mpoStorage->Read(this->miPageID, 0, lpoBuffer, this->miPageSize) ) < 0) 
    {
        return ERROR_READ_FILE;
    }
    
    ACL_ASSERT(liResult == this->miPageSize);

    apl_memcpy(&this->moHeader, lpoBuffer, sizeof(this->moHeader) );
    
    //Check page integrity
    if ( (this->moHeader.miReadOffset < (apl_int32_t)sizeof(CPageHeader) 
             || this->moHeader.miWriteOffset < (apl_int32_t)sizeof(CPageHeader) )
         || (this->moHeader.miReadOffset > this->miPageSize 
             || this->moHeader.miWriteOffset > this->miPageSize)
         || (this->moHeader.miReadOffset > this->moHeader.miWriteOffset) )
    {
        return ERROR_INVALID_DATA;
    }

    CRecordHeader loRecordHeader;
    apl_int32_t liReadOffset = this->moHeader.miReadOffset;
    apl_int32_t liWriteOffset = this->moHeader.miWriteOffset;

    //Check if all record is reachable
    while(liReadOffset < liWriteOffset)
    {
        apl_memcpy(&loRecordHeader, lpoBuffer + liReadOffset, sizeof(CRecordHeader) );

        liReadOffset += (loRecordHeader.miLength + sizeof(CRecordHeader) );
    }

    if (liReadOffset != liWriteOffset)
    {
        return ERROR_INVALID_DATA;
    }

    return 0;
}

void CVLRPage::Close(void)
{
}

apl_int_t CVLRPage::Read( apl_int_t aiStayTime, acl::CMemoryBlock& aoData )
{
    ACL_ASSERT(this->moHeader.miWriteOffset - this->moHeader.miReadOffset > (apl_int32_t)sizeof(CRecordHeader) );

    apl_int32_t liNow = apl_time()/APL_TIME_SEC;
    apl_ssize_t liResult = 0;
    CRecordHeader loRecordHeader;

    //Read record header from bufferpool
    if ( (liResult = this->mpoBufferPool->Read(
        this->miPageID,
        this->moHeader.miReadOffset,
        &loRecordHeader,
        sizeof(loRecordHeader) ) ) < 0)
    {
        return liResult;
    }

    ACL_ASSERT(liResult == sizeof(CRecordHeader) );
    
    if (loRecordHeader.miLength <= 0 || loRecordHeader.miLength > this->miPageSize)
    {
        //Is impossible! data valid checked on startup
        bool error_invalid_data_exception = false;
        ACL_ASSERT(error_invalid_data_exception);

        return ERROR_INVALID_DATA;
    }

    if (liNow - loRecordHeader.miTimestamp < aiStayTime)
    {
        return ERROR_TIMEDOUT_UNEXISTED;
    }
    
    aoData.Resize(loRecordHeader.miLength);
    
    if ( (liResult = this->mpoBufferPool->Read(
        this->miPageID,
        this->moHeader.miReadOffset + sizeof(CRecordHeader),
        aoData.GetWritePtr(), 
        loRecordHeader.miLength) ) < 0)
    {
        return liResult;
    }
    
    ACL_ASSERT(liResult == loRecordHeader.miLength);
    
    aoData.SetWritePtr(loRecordHeader.miLength);

    this->moHeader.miReadOffset += (loRecordHeader.miLength + sizeof(CRecordHeader) );
    
    //Write back read-offset
    if ( (liResult = this->mpoBufferPool->Write(
        this->miPageID, 
        CPageHeader::ROV_OFFSET, 
        &this->moHeader.miReadOffset, 
        sizeof(this->moHeader.miReadOffset), 
        CVLRBufferPool::OPT_ATOMIC) ) < 0)
    {
        return liResult;
    }

    ACL_ASSERT(liResult == sizeof(this->moHeader.miReadOffset) );
    
    return 0;
}

apl_int_t CVLRPage::Write( char const* apcBuffer, apl_size_t auLen )
{
    if ( (apl_int_t)auLen > 
        this->miPageSize - (apl_int_t)(sizeof(CPageHeader) - sizeof(CRecordHeader) ) )
    {
        return ERROR_OUT_OF_RANGE;
    }
    else if ( (apl_int_t)auLen > 
        this->miPageSize - this->moHeader.miWriteOffset - (apl_int_t)sizeof(CRecordHeader) )
    {
        return ERROR_NO_ENOUGH_SPACE;
    }
    
    apl_ssize_t liResult = 0;
    CRecordHeader loRecordHeader(apl_time()/APL_TIME_SEC, auLen);
    
    //Write record header
    if ( (liResult = this->mpoBufferPool->Write(
        this->miPageID,
        this->moHeader.miWriteOffset,
        &loRecordHeader,
        sizeof(loRecordHeader) ) ) < 0 )
    {
        return liResult;
    }
    
    ACL_ASSERT(liResult == (apl_ssize_t)sizeof(CRecordHeader) );
    
    //Write record data
    if ( (liResult = this->mpoBufferPool->Write(
        this->miPageID,
        this->moHeader.miWriteOffset + sizeof(CRecordHeader),
        apcBuffer,
        auLen) ) < 0)
    {
        return liResult;
    }
    
    ACL_ASSERT(liResult == (apl_ssize_t)auLen);
    
    this->moHeader.miWriteOffset += (sizeof(CRecordHeader) + auLen);
    
    //Write back write-offset
    if ( (liResult = this->mpoBufferPool->Write(
        this->miPageID, 
        CPageHeader::WOV_OFFSET, 
        &this->moHeader.miWriteOffset, 
        sizeof(this->moHeader.miWriteOffset),
        CVLRBufferPool::OPT_ATOMIC) ) < 0)
    {
        return liResult;
    }
    
    ACL_ASSERT(liResult == sizeof(this->moHeader.miReadOffset) );
    
    return 0;
}
    
bool CVLRPage::IsEmpty(void)
{
    return this->moHeader.miReadOffset == this->moHeader.miWriteOffset;
}

apl_int_t CVLRPage::GetPageID(void)
{
    return this->miPageID;
}

apl_int32_t CVLRPage::GetPageSize(void)
{
    return this->miPageSize;
}

apl_int32_t CVLRPage::GetReadoffset(void)
{
    return this->moHeader.miReadOffset;
}

apl_int32_t CVLRPage::GetWriteOffset(void)
{
    return this->moHeader.miWriteOffset;
}
    
apl_int32_t CVLRPage::GetSpace(void)
{
    return this->miPageSize - this->moHeader.miWriteOffset - sizeof(CRecordHeader);
}

AIBC_GFQ_NAMESPACE_END

