
#include "Storage.h"
#include "acl/Dir.h"
#include "acl/Timestamp.h"

namespace 
{
    class CElapsedTimer
    {
    public:
        CElapsedTimer(void)
            : moStart(acl::CTimestamp::PRC_USEC)
        {
        }

        ~CElapsedTimer(void)
        {
            acl::CTimestamp loEnd(acl::CTimestamp::PRC_USEC);

            apl_int_t liElapsed = loEnd.Sec() - this->moStart.Sec();

            if (liElapsed > 0)
            {
                apl_errprintf("Elapsed time = %"APL_PRIdINT"\n", liElapsed);
            }
        }

    private:
        acl::CTimestamp moStart;
    };
};

AIBC_GFQ_NAMESPACE_START

#define HEADER_FILE_NAME      "gfq_header"
#define DATA_FILE_NAME        "gfq_data"
#define METADATA_MAX_LENGTH   4096

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CStorageMetadata::Dump(void)
{
    
    printf("version           = %s\n", this->Get(METADATA_VERSION, "unknown") );
    printf("segment count     = %s\n", this->Get(METADATA_SEGMENT_COUNT, "unknown") );
    printf("segment capacity  = %s\n", this->Get(METADATA_SEGMENT_CAPACITY, "unknown") );
    printf("file count        = %s\n", this->Get(METADATA_FILE_COUNT, "unknown") );
    printf("page count        = %s\n", this->Get(METADATA_PAGE_COUNT, "unknown") );
    printf("total page count  = %s\n", this->Get(METADATA_TOTAL_PAGE_COUNT, "unknown") );
    printf("page size         = %s\n", this->Get(METADATA_PAGE_SIZE, "unknown") );
    printf("header size       = %s\n", this->Get(METADATA_HEADER_SIZE, "unknown") );
}

CStorage::CStorage(void)
    : mpoFile(APL_NULL)
    , miFileCount(0)
    , miPageCount(0)
    , miPageSize(0)
{
}

CStorage::~CStorage(void)
{
}

apl_int_t CStorage::Create( char const* apcPath, CStorageMetadata& aoMetadata )
{
    char lacTemp[APL_NAME_MAX];

    acl::CDir::Make(apcPath);

    {//Storage Header
        acl::CFile loFile;
        acl::CMemoryBlock loBuffer(METADATA_MAX_LENGTH);
        apl_int_t liHeaderSize = aoMetadata.GetInt(METADATA_HEADER_SIZE, -1);
        
        if (aoMetadata.Encode(&loBuffer) != 0)
        {
            apl_errprintf("Storage metadata encode fail, maybe to long\n");

            return ERROR_INVALID_DATA;
        }

        apl_snprintf(lacTemp, sizeof(lacTemp), "%s/%s", apcPath, HEADER_FILE_NAME);
        if (loFile.Open(lacTemp, APL_O_CREAT|APL_O_RDWR, 0644) != 0)
        {
            apl_errprintf("Storage create header file fail,%s (Path=%s)\n",
                apl_strerror(apl_get_errno() ),
                lacTemp);

            return ERROR_OPEN_FILE;
        }

        //Initialize segment storage header context
        if (loFile.Truncate(liHeaderSize + METADATA_MAX_LENGTH) != 0)
        {
            apl_errprintf("Storage allocate header space fail,%s (Path=%s,HeaderSize=%"APL_PRIdINT")\n",
                apl_strerror(apl_get_errno() ),
                lacTemp,
                liHeaderSize);

            return ERROR_NO_ENOUGH_SPACE;
        }
        
        //Initialize segment storage header context
        if (loFile.Write(0, loBuffer.GetReadPtr(), loBuffer.GetSize() ) !=
            (apl_ssize_t)loBuffer.GetSize() )
        {
            apl_errprintf("Storage write metadata file fail,%s (Path=%s)\n",
                apl_strerror(apl_get_errno() ),
                lacTemp);

            return ERROR_WRITE_FILE;
        }
    }//End storage header

    {//Page storage
        acl::CFile loFile;
        apl_int_t liFileCount = aoMetadata.GetInt(METADATA_FILE_COUNT, -1);
        apl_int_t liPageCount = aoMetadata.GetInt(METADATA_PAGE_COUNT, -1);
        apl_int_t liPageSize = aoMetadata.GetInt(METADATA_PAGE_SIZE, -1);
        apl_int_t liFileSize = liPageCount * liPageSize;
        
        for (apl_int_t liN = 0; liN < liFileCount; liN++)
        {
            acl::CFile loFile;

            apl_snprintf(lacTemp, sizeof(lacTemp), "%s/%s.%"APL_PRIdINT, apcPath, DATA_FILE_NAME, liN);

            if (loFile.Open(lacTemp, APL_O_CREAT | APL_O_RDWR, 0644) != 0)
            {
                apl_errprintf("Storage create data file fail,%s (Path=%s)\n",
                    apl_strerror(apl_get_errno() ),
                    lacTemp);

                return ERROR_OPEN_FILE;
            }

            if (loFile.Truncate(liFileSize) != 0)
            {
                apl_errprintf("Storage allocate data file fail,%s (Path=%s)\n",
                    apl_strerror(apl_get_errno() ),
                    lacTemp);

                return ERROR_NO_ENOUGH_SPACE;
            }
        }
    }//End page storage

    return 0;
}

apl_int_t CStorage::Open( char const* apcPath )
{
    char lacTemp[APL_NAME_MAX];

    {//Storage header
        apl_snprintf(lacTemp, sizeof(lacTemp), "%s/%s", apcPath, HEADER_FILE_NAME);
        if (this->moHeaderMemMap.Map(lacTemp, APL_PROT_READ|APL_PROT_WRITE, MAP_SHARED) != 0)
        {
            apl_errprintf("Storage map header file fail,%s (Path=%s)\n",
                apl_strerror(apl_get_errno() ),
                lacTemp);

            return ERROR_MAP_FILE;
        }

        ACL_ASSERT(this->moHeaderMemMap.GetSize() > METADATA_MAX_LENGTH);
        
        acl::CMemoryBlock loBuffer(
            (char*)this->moHeaderMemMap.GetAddr(),
            METADATA_MAX_LENGTH,
            acl::CMemoryBlock::DONT_DELETE);
        
        loBuffer.SetWritePtr(METADATA_MAX_LENGTH);
        
        if (this->moMetadata.Decode(&loBuffer) != 0)
        {
            apl_errprintf("Storage metadata decode fail, maybe invalid format\n");

            return ERROR_INVALID_DATA;
        }

        this->miFileCount = this->moMetadata.GetInt(METADATA_FILE_COUNT, -1);
        this->miPageCount = this->moMetadata.GetInt(METADATA_PAGE_COUNT, -1);
        this->miPageSize = this->moMetadata.GetInt(METADATA_PAGE_SIZE, -1);
    }//End storage header

    {//Page storage
        apl_int_t liFileSize = this->miPageCount * this->miPageSize;

        ACL_NEW_N_ASSERT(this->mpoFile, acl::CFile, this->miFileCount);

        for (apl_int_t liN = 0; liN < this->miFileCount; liN++)
        {
            apl_snprintf(lacTemp, sizeof(lacTemp), "%s/%s.%"APL_PRIdINT, apcPath, DATA_FILE_NAME, liN);

            if (this->mpoFile[liN].Open(lacTemp, APL_O_CREAT | APL_O_RDWR) != 0)
            {
                apl_errprintf("Storage open data file fail,%s (Path=%s)\n",
                    apl_strerror(apl_get_errno() ),
                    lacTemp);

                return ERROR_OPEN_FILE;
            }

            if (this->mpoFile[liN].GetFileSize() != liFileSize)
            {
                apl_errprintf("PageStorage data file size exception, "
                    "(FileSize=%"APL_PRIdINT", ExpectSize=%"APL_PRIdINT")\n",
                    (apl_int_t)this->mpoFile[liN].GetFileSize(),
                    liFileSize);

                return ERROR_INVALID_FILE;
            }
        }
    }//End page storage

    return 0;
}

void CStorage::Close(void)
{
    this->moHeaderMemMap.Unmap();

    ACL_DELETE_N(this->mpoFile);
}

CStorageMetadata* CStorage::GetMetadata(void)
{
    return &this->moMetadata;
}

void* CStorage::GetHeader(void)
{
    return ((char*)this->moHeaderMemMap.GetAddr() ) + METADATA_MAX_LENGTH;
}

apl_size_t CStorage::GetHeaderLength(void)
{
    return this->moHeaderMemMap.GetSize() - METADATA_MAX_LENGTH;
}

apl_ssize_t CStorage::Read(
    apl_int_t aiPageID,
    apl_int32_t aiOffset,
    void* apvBuffer,
    apl_size_t auLen )
{
    apl_int_t liIndex = aiPageID / this->miPageCount;
    apl_int_t liOffset = (aiPageID % this->miPageCount) * this->miPageSize + aiOffset;

    ACL_ASSERT(liIndex < this->miPageCount);
    
    return this->mpoFile[liIndex].Read(liOffset, apvBuffer, auLen);
}

apl_ssize_t CStorage::Write(
    apl_int_t aiPageID,
    apl_int32_t aiOffset,
    void const* apvBuffer,
    apl_size_t auLen )
{
    CElapsedTimer loTimer;

    apl_int_t liIndex = aiPageID / this->miPageCount;
    apl_int_t liOffset = (aiPageID % this->miPageCount) * this->miPageSize + aiOffset;

    ACL_ASSERT(liIndex < this->miPageCount);

    return this->mpoFile[liIndex].Write(liOffset, apvBuffer, auLen);
}

apl_int32_t CStorage::GetPageSize(void)
{
    return this->miPageSize;
}

AIBC_GFQ_NAMESPACE_END

