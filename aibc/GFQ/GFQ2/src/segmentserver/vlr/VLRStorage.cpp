
#include "VLRStorage.h"
#include "aaf/LogService.h"
#include "acl/Dir.h"
#include "acl/Timestamp.h"
#include "acl/DateTime.h"

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

#define GFQ2_HEADER_FILE_NAME      "gfq_header"
#define GFQ2_DATA_FILE_NAME        "gfq_data"
#define GFQ2_METADATA_LENGTH_MAX   4096

/** 
 * @brief Storage metadata type define
 */
enum
{
    MD_TAG_VERSION          = 0,
    MD_TAG_TYPE             = 1,
    MD_TAG_SEGMENT_COUNT    = 2,
    MD_TAG_SEGMENT_CAPACITY = 3,
    MD_TAG_FILE_COUNT       = 4,
    MD_TAG_PAGE_COUNT       = 5,
    MD_TAG_TOTAL_PAGE_COUNT = 6,
    MD_TAG_PAGE_SIZE        = 7,
    MD_TAG_HEADER_SIZE      = 8,
    MD_TAG_CREATE_TIME      = 9
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
CVLRStorage::CVLRStorage(void)
    : mpoFile(APL_NULL)
    , miFileCount(0)
    , miPageCount(0)
    , miPageSize(0)
{
}

CVLRStorage::~CVLRStorage(void)
{
}

apl_int_t CVLRStorage::Create(
    char const* apcPath,
    char const* apcVersion,
    apl_size_t auHeaderSize,
    apl_size_t auSegmentCapacity,
    apl_size_t auSegmentCount,
    apl_size_t auFileCount,
    apl_size_t auPageCount,
    apl_size_t auPageSize )
{
    acl::CDateTime loDateTime;
    acl::CIndexDict loMetadata;
    char lacTemp[APL_NAME_MAX];

    acl::CDir::Make(apcPath);
    loDateTime.Update();
    
    loMetadata.Set(MD_TAG_TYPE, "vlr");
    loMetadata.Set(MD_TAG_VERSION, apcVersion);
    loMetadata.Set(MD_TAG_SEGMENT_CAPACITY, auSegmentCapacity);
    loMetadata.Set(MD_TAG_SEGMENT_COUNT, auSegmentCount);
    loMetadata.Set(MD_TAG_FILE_COUNT, auFileCount);
    loMetadata.Set(MD_TAG_PAGE_COUNT, auPageCount);
    loMetadata.Set(MD_TAG_TOTAL_PAGE_COUNT, auPageCount*auFileCount);
    loMetadata.Set(MD_TAG_PAGE_SIZE, auPageSize);
    loMetadata.Set(MD_TAG_HEADER_SIZE, auHeaderSize);
    loMetadata.Set(MD_TAG_CREATE_TIME, loDateTime.Format("%Y-%m-%d %H:%M:%S") );
    
    {//Storage Header
        acl::CFile loFile;
        acl::CMemoryBlock loBuffer(GFQ2_METADATA_LENGTH_MAX);
        
        if (loMetadata.Encode(&loBuffer) != 0)
        {
            apl_errprintf("VLR storage metadata encode fail, maybe to long\n");

            return ERROR_INVALID_DATA;
        }

        apl_snprintf(lacTemp, sizeof(lacTemp), "%s/%s", apcPath, GFQ2_HEADER_FILE_NAME);
        if (loFile.Open(lacTemp, APL_O_CREAT|APL_O_RDWR, 0644) != 0)
        {
            apl_errprintf("VLR storage create header file fail,%s (Path=%s)\n",
                apl_strerror(apl_get_errno() ),
                lacTemp);

            return ERROR_OPEN_FILE;
        }

        //Initialize segment storage header context
        if (loFile.Truncate(auHeaderSize + GFQ2_METADATA_LENGTH_MAX) != 0)
        {
            apl_errprintf("VLR storage allocate header space fail,%s (Path=%s,HeaderSize=%"APL_PRIuINT")\n",
                apl_strerror(apl_get_errno() ),
                lacTemp,
                auHeaderSize);

            return ERROR_NO_ENOUGH_SPACE;
        }
        
        //Initialize segment storage header context
        if (loFile.Write(0, loBuffer.GetReadPtr(), loBuffer.GetSize() ) !=
            (apl_ssize_t)loBuffer.GetSize() )
        {
            apl_errprintf("VLR storage write metadata file fail,%s (Path=%s)\n",
                apl_strerror(apl_get_errno() ),
                lacTemp);

            return ERROR_WRITE_FILE;
        }
    }//End storage header

    {//Page storage
        acl::CFile loFile;
        apl_size_t luFileSize = auPageCount * auPageSize;
        
        for (apl_size_t luN = 0; luN < auFileCount; luN++)
        {
            acl::CFile loFile;

            apl_snprintf(lacTemp, sizeof(lacTemp), "%s/%s.%"APL_PRIuINT, apcPath, GFQ2_DATA_FILE_NAME, luN);

            if (loFile.Open(lacTemp, APL_O_CREAT | APL_O_RDWR, 0644) != 0)
            {
                apl_errprintf("VLR storage create data file fail,%s (Path=%s)\n",
                    apl_strerror(apl_get_errno() ),
                    lacTemp);

                return ERROR_OPEN_FILE;
            }

            if (loFile.Truncate(luFileSize) != 0)
            {
                apl_errprintf("VLR storage allocate data file fail,%s (Path=%s)\n",
                    apl_strerror(apl_get_errno() ),
                    lacTemp);

                return ERROR_NO_ENOUGH_SPACE;
            }
        }
    }//End page storage

    return 0;
}

apl_int_t CVLRStorage::Open( char const* apcPath )
{
    char lacTemp[APL_NAME_MAX];

    {//Storage header
        apl_snprintf(lacTemp, sizeof(lacTemp), "%s/%s", apcPath, GFQ2_HEADER_FILE_NAME);
        if (this->moHeaderMemMap.Map(lacTemp, APL_PROT_READ|APL_PROT_WRITE, MAP_SHARED) != 0)
        {
            apl_errprintf("VLR storage map header file fail,%s (Path=%s)\n",
                apl_strerror(apl_get_errno() ),
                lacTemp);

            return ERROR_MAP_FILE;
        }

        ACL_ASSERT(this->moHeaderMemMap.GetSize() > GFQ2_METADATA_LENGTH_MAX);
        
        acl::CMemoryBlock loBuffer(
            (char*)this->moHeaderMemMap.GetAddr(),
            GFQ2_METADATA_LENGTH_MAX,
            acl::CMemoryBlock::DONT_DELETE);
        
        loBuffer.SetWritePtr(GFQ2_METADATA_LENGTH_MAX);
        
        //Decode storage file metadata
        if (this->moMetadata.Decode(&loBuffer) != 0)
        {
            apl_errprintf("VLR storage metadata decode fail, maybe invalid format\n");

            return ERROR_INVALID_DATA;
        }

        //Check storage type
        if ( apl_strcmp(this->moMetadata.Get(MD_TAG_TYPE, ""), "vlr") != 0)
        {
            apl_errprintf("VLR storage type check fail, non-vlr-storage\n");

            return ERROR_INVALID_DATA;
        }

        //Check storage version
        //...

        this->miSegmentCount = this->moMetadata.GetInt(MD_TAG_SEGMENT_COUNT, -1);
        this->miSegmentCapacity = this->moMetadata.GetInt(MD_TAG_SEGMENT_CAPACITY, -1);
        this->miFileCount = this->moMetadata.GetInt(MD_TAG_FILE_COUNT, -1);
        this->miPageCount = this->moMetadata.GetInt(MD_TAG_PAGE_COUNT, -1);
        this->miPageSize = this->moMetadata.GetInt(MD_TAG_PAGE_SIZE, -1);
    }//End storage header

    {//Page storage
        apl_int_t liFileSize = this->miPageCount * this->miPageSize;

        ACL_NEW_N_ASSERT(this->mpoFile, acl::CFile, this->miFileCount);

        for (apl_int_t liN = 0; liN < this->miFileCount; liN++)
        {
            apl_snprintf(lacTemp, sizeof(lacTemp), "%s/%s.%"APL_PRIdINT, apcPath, GFQ2_DATA_FILE_NAME, liN);

            if (this->mpoFile[liN].Open(lacTemp, APL_O_CREAT | APL_O_RDWR) != 0)
            {
                apl_errprintf("VLR storage open data file fail,%s (Path=%s)\n",
                    apl_strerror(apl_get_errno() ),
                    lacTemp);

                return ERROR_OPEN_FILE;
            }

            if (this->mpoFile[liN].GetFileSize() != liFileSize)
            {
                apl_errprintf("VLR storage data file size exception, "
                    "(File=%s,FileSize=%"APL_PRIdINT", ExpectSize=%"APL_PRIdINT")\n",
                    lacTemp,
                    (apl_int_t)this->mpoFile[liN].GetFileSize(),
                    liFileSize);

                return ERROR_INVALID_FILE;
            }
        }
    }//End page storage

    return 0;
}

void CVLRStorage::Close(void)
{
    this->moHeaderMemMap.Unmap();

    ACL_DELETE_N(this->mpoFile);
}

void* CVLRStorage::GetHeader(void)
{
    return ((char*)this->moHeaderMemMap.GetAddr() ) + GFQ2_METADATA_LENGTH_MAX;
}

apl_size_t CVLRStorage::GetHeaderLength(void)
{
    return this->moHeaderMemMap.GetSize() - GFQ2_METADATA_LENGTH_MAX;
}

apl_ssize_t CVLRStorage::Read(
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

apl_ssize_t CVLRStorage::Write(
    apl_int_t aiPageID,
    apl_int32_t aiOffset,
    void const* apvBuffer,
    apl_size_t auLen )
{
//    CElapsedTimer loTimer;

    apl_int_t liIndex = aiPageID / this->miPageCount;
    apl_int_t liOffset = (aiPageID % this->miPageCount) * this->miPageSize + aiOffset;

    ACL_ASSERT(liIndex < this->miPageCount);

    return this->mpoFile[liIndex].Write(liOffset, apvBuffer, auLen);
}

char const* CVLRStorage::GetVersion(void)
{
    return this->moMetadata.Get(MD_TAG_VERSION, "unknown");
}

char const* CVLRStorage::GetType(void)
{
    return this->moMetadata.Get(MD_TAG_TYPE, "unknown");
}

char const* CVLRStorage::GetCreateTime(void)
{
    return this->moMetadata.Get(MD_TAG_CREATE_TIME, "unknown");
}

apl_int32_t CVLRStorage::GetSegmentCount(void)
{
    return this->miSegmentCount;
}
    
apl_int32_t CVLRStorage::GetSegmentCapacity(void)
{
    return this->miSegmentCapacity;
}
    
apl_int32_t CVLRStorage::GetFileCount(void)
{
    return this->miFileCount;
}
    
apl_int32_t CVLRStorage::GetPageCount(void)
{
    return this->miPageCount;
}
    
apl_int32_t CVLRStorage::GetPageSize(void)
{
    return this->miPageSize;
}

AIBC_GFQ_NAMESPACE_END

