
#include "Page.h"

AIBC_GFQ_NAMESPACE_START

////////////////////////////////////////////////////////////////////////////////////////////
//Page header define
#define GFQ_READ_OFFSET         0
#define GFQ_WRITE_OFFSET        4
#define GFQ_PAGE_HEADER_LEN     8
//------------------------------------------------------------------
//| read_offset (4) | write_offset(4) | ......
//------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////////////////
//Record header define
#define GFQ_TIMESTAMP_OFFSET    0
#define GFQ_LENGTH_OFFSET       8        
#define GFQ_RECORD_HEADER_LEN   12
//------------------------------------------------------------------
//| timestamp(8) | length (4) | data | ......
//------------------------------------------------------------------

CPage::CPage( CStorage* apoStorage, CBufferPool* apoBufferPool, apl_int_t aiPageID )
    : mpoStorage(apoStorage)
    , mpoBufferPool(apoBufferPool)
    , miPageID(aiPageID)
    , miPageSize(apoStorage->GetPageSize() )
{
}

apl_int_t CPage::Init(void)
{
    apl_ssize_t liResult = 0;
    apl_int32_t laiHeader[2] = {GFQ_PAGE_HEADER_LEN, GFQ_PAGE_HEADER_LEN};

    //Initialize page header field
    if ( (liResult = this->mpoStorage->Write(this->miPageID, 0, laiHeader, sizeof(laiHeader) ) ) < 0) 
    {
        apl_errprintf(
            "Page init fail,%s (PageID=%"APL_PRIdINT")\n",
            apl_strerror(apl_get_errno() ),
            (apl_int_t)this->miPageID
        );

        return ERROR_WRITE_FILE;
    }
    
    ACL_ASSERT(liResult == (apl_ssize_t)sizeof(laiHeader) );
    
    //Pre-allocate page buffer in buffer pool
    if (this->mpoBufferPool->Preallocate(this->miPageID) == 0)
    {
        //Initialize cache page header by memory copy instead of read from file
        apl_int_t liRetCode = this->mpoBufferPool->Write(this->miPageID, 0, laiHeader, sizeof(laiHeader) );
        ACL_ASSERT(liRetCode == sizeof(laiHeader) );
    }
    
    this->miReadOffset = laiHeader[0];
    this->miWriteOffset = laiHeader[1];
    
    return 0;
}
    
apl_int_t CPage::Free(void)
{
    //Release cache page from buffer pool
    this->mpoBufferPool->Erase(this->miPageID);
    
    return 0;
}
    
apl_int_t CPage::Open(void)
{
    apl_ssize_t liResult = 0;
    apl_int32_t laiOffset[2] = {0, 0};

    //Read page header from buffer pool only
    if ( (liResult = this->mpoBufferPool->Read(
        this->miPageID,
        GFQ_READ_OFFSET,
        laiOffset,
        sizeof(laiOffset) ) ) < 0)
    {
        return liResult;
    }
    
    ACL_ASSERT(liResult == (apl_ssize_t)sizeof(laiOffset) );

    this->miReadOffset = laiOffset[0];
    this->miWriteOffset = laiOffset[1];
    
    return 0;
}

apl_int_t CPage::Check(void)
{
    apl_ssize_t liResult = 0;
    apl_int32_t laiHeader[2] = {0, 0};

    //Read page header from file directly
    if ( (liResult = this->mpoStorage->Read(this->miPageID, 0, laiHeader, sizeof(laiHeader) ) ) < 0) 
    {
        return ERROR_READ_FILE;
    }
    
    ACL_ASSERT(liResult == (apl_ssize_t)sizeof(laiHeader) );
    
    //Check page integrity
    if ( (laiHeader[0] < GFQ_PAGE_HEADER_LEN || laiHeader[1] < GFQ_PAGE_HEADER_LEN)
         || (laiHeader[0] > this->miPageSize || laiHeader[1] > this->miPageSize)
         || (laiHeader[0] > laiHeader[1]) )
    {
        return ERROR_INVALID_DATA;
    }

    return 0;
}

void CPage::Close(void)
{
}

apl_int_t CPage::Read( apl_int_t aiStayTime, std::string& aoData )
{
    ACL_ASSERT(this->miWriteOffset - this->miReadOffset > GFQ_RECORD_HEADER_LEN);

    apl_int64_t li64Now = apl_time();
    apl_int64_t li64Timestamp = 0;
    apl_int32_t liLen = 0;
    apl_ssize_t liResult = 0;
    apl_int32_t liReadOffset(this->miReadOffset);
    char lacBuffer[1024];

    //Read record header from bufferpool
    if ( (liResult = this->mpoBufferPool->Read(this->miPageID, liReadOffset, lacBuffer, sizeof(lacBuffer) ) ) < 0)
    {
        return liResult;
    }

    ACL_ASSERT(liResult > GFQ_RECORD_HEADER_LEN);
    
    //Parse record header field
    apl_memcpy(&li64Timestamp, lacBuffer + GFQ_TIMESTAMP_OFFSET, sizeof(li64Timestamp) );
    apl_memcpy(&liLen, lacBuffer + GFQ_LENGTH_OFFSET, sizeof(liLen) );

    if (liLen <= 0 || liLen > this->miPageSize)
    {
        return ERROR_INVALID_DATA;
    }

    if (li64Now - li64Timestamp < aiStayTime)
    {
        return ERROR_TIMEDOUT_UNEXISTED;
    }

    if (liLen <= liResult - GFQ_RECORD_HEADER_LEN)
    {
        //Enough in buffer
        aoData.assign(lacBuffer + GFQ_RECORD_HEADER_LEN, liLen);
    }
    else
    {
        aoData.assign(lacBuffer + GFQ_RECORD_HEADER_LEN, liResult - GFQ_RECORD_HEADER_LEN);
        liLen -= (liResult - GFQ_RECORD_HEADER_LEN);
        liReadOffset += liResult;
        
        while(liLen > 0)
        {
            apl_int_t liNeedSize = liLen > (apl_int_t)sizeof(lacBuffer) ? sizeof(lacBuffer) : liLen;

            if ( (liResult = this->mpoBufferPool->Read(this->miPageID, liReadOffset, lacBuffer, liNeedSize) ) < 0)
            {
                return liResult;
            }

            aoData.append(lacBuffer, liResult);

            liLen -= liResult;
            liReadOffset += liResult;
        } 
    }

    this->miReadOffset += (aoData.length() + GFQ_RECORD_HEADER_LEN);
    
    //Write back read-offset
    if ( (liResult = this->mpoBufferPool->Write(this->miPageID, GFQ_READ_OFFSET, &this->miReadOffset, 4, true) ) < 0)
    {
        return liResult;
    }

    ACL_ASSERT(liResult == 4);
    
    return 0;
}

apl_int_t CPage::Write( char const* apcBuffer, apl_size_t auLen )
{
    ACL_ASSERT((apl_int32_t)(auLen + GFQ_RECORD_HEADER_LEN) < this->miPageSize - this->miWriteOffset);
    
    char lacHeader[GFQ_RECORD_HEADER_LEN];
    apl_ssize_t liResult = 0;
    apl_int32_t liLen(auLen);
    apl_int64_t li64Timestamp = apl_time();

    apl_memcpy(lacHeader + GFQ_TIMESTAMP_OFFSET, &li64Timestamp, 8);
    apl_memcpy(lacHeader + GFQ_LENGTH_OFFSET, &liLen, 4);
    
    //Write record header
    if ( (liResult = this->mpoBufferPool->Write(
        this->miPageID,
        this->miWriteOffset,
        lacHeader,
        sizeof(lacHeader) ) ) < 0 )
    {
        return liResult;
    }
    
    ACL_ASSERT(liResult == (apl_ssize_t)sizeof(lacHeader) );
    
    //Write record data
    if ( (liResult = this->mpoBufferPool->Write(
        this->miPageID,
        this->miWriteOffset + GFQ_RECORD_HEADER_LEN,
        apcBuffer,
        auLen) ) < 0)
    {
        return liResult;
    }
    
    ACL_ASSERT(liResult == (apl_ssize_t)auLen );
    
    this->miWriteOffset += (auLen + GFQ_RECORD_HEADER_LEN);
    
    //Write back write-offset
    if ( (liResult = this->mpoBufferPool->Write(this->miPageID, GFQ_WRITE_OFFSET, &this->miWriteOffset, 4) ) < 0)
    {
        return liResult;
    }
    
    ACL_ASSERT(liResult == 4);
    
    return 0;
}
    
bool CPage::IsEmpty(void)
{
    return this->miReadOffset == this->miWriteOffset;
}

apl_int_t CPage::GetPageID(void)
{
    return this->miPageID;
}

apl_int32_t CPage::GetPageSize(void)
{
    return this->miPageSize;
}

apl_int32_t CPage::GetReadoffset(void)
{
    return this->miReadOffset;
}

apl_int32_t CPage::GetWriteOffset(void)
{
    return this->miWriteOffset;
}
    
apl_int32_t CPage::GetSpace(void)
{
    return this->miPageSize - this->miWriteOffset - GFQ_RECORD_HEADER_LEN;
}

AIBC_GFQ_NAMESPACE_END

