
#include "FLRStorage.h"
#include "acl/Synch.h"
#include "acl/Dir.h"
#include "acl/DateTime.h"

AIBC_GFQ_NAMESPACE_START

#define GFQ2_HEADER_FILE_NAME      "gfq_header"
#define GFQ2_DATA_FILE_NAME        "gfq_data"
#define GFQ2_METADATA_LENGTH_MAX   4096

//storage matadata
enum {
    MD_TAG_VERSION = 0,
    MD_TAG_TYPE,
    MD_TAG_SLOT_CAPACITY,
    MD_TAG_SEGMENT_CAPACITY,
    MD_TAG_SEGMENT_COUNT,
    MD_TAG_FILE_COUNT,
    MD_TAG_HEADER_SIZE,
    MD_TAG_CREATE_TIME
};

//storage record header
struct CFLRRecordHeader
{
    apl_int32_t miLength;

    apl_int32_t miTimestamp;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
CFLRStorage::CFLRStorage(void)
    : muSlotCapacity(0)
    , muSegmentCapacity(0)
    , muSegmentCount(0)
    , muFileCount(0)
    , muFileCapacity(0)
{
}

CFLRStorage::~CFLRStorage(void)
{
}

apl_int_t CFLRStorage::Create( 
    char const* apcPath,
    char const* apcVersion,
    apl_size_t auHeaderSize,
    apl_size_t auSlotCapacity,
    apl_size_t auSegmentCapacity,
    apl_size_t auSegmentCount,
    apl_size_t auFileCount )
{
    acl::CDateTime loDateTime;
    acl::CIndexDict loMetadata;
    acl::CMemoryBlock loBuffer(GFQ2_METADATA_LENGTH_MAX);
    char lacTemp[APL_NAME_MAX];

    acl::CDir::Make(apcPath);
    loDateTime.Update();

    auSlotCapacity += sizeof(CFLRRecordHeader);

    loMetadata.Set(MD_TAG_TYPE, "flr");
    loMetadata.Set(MD_TAG_VERSION, apcVersion);
    loMetadata.Set(MD_TAG_SLOT_CAPACITY, auSlotCapacity);
    loMetadata.Set(MD_TAG_SEGMENT_CAPACITY, auSegmentCapacity);
    loMetadata.Set(MD_TAG_SEGMENT_COUNT, auSegmentCount);
    loMetadata.Set(MD_TAG_FILE_COUNT, auFileCount);
    loMetadata.Set(MD_TAG_HEADER_SIZE, auHeaderSize);
    loMetadata.Set(MD_TAG_CREATE_TIME, loDateTime.Format("%Y-%m-%d %H:%M:%S") );

    if (loMetadata.Encode(&loBuffer) != 0)
    {
        apl_errprintf("FLR storage metadata encode fail, maybe to long\n");

        return ERROR_INVALID_DATA;
    }
   
    {//create segment headers storage file
        acl::CFile loFile;

        apl_snprintf(lacTemp, sizeof(lacTemp), "%s/%s", apcPath, GFQ2_HEADER_FILE_NAME);
        if (loFile.Open(lacTemp, APL_O_CREAT|APL_O_RDWR, 0644) != 0)
        {
            apl_errprintf("FLR storage create header file fail,%s (Path=%s)\n",
                apl_strerror(apl_get_errno() ),
                lacTemp);

            return ERROR_OPEN_FILE;
        }

        //Initialize segment storage header context
        if (loFile.Truncate(auHeaderSize + GFQ2_METADATA_LENGTH_MAX) != 0)
        {
            apl_errprintf("FLR storage allocate header space fail,%s (Path=%s,HeaderSize=%"APL_PRIuINT")\n",
                apl_strerror(apl_get_errno() ),
                lacTemp,
                auHeaderSize);

            return ERROR_NO_ENOUGH_SPACE;
        }
        
        //Initialize segment storage header context
        if (loFile.Write(0, loBuffer.GetReadPtr(), loBuffer.GetSize() ) !=
            (apl_ssize_t)loBuffer.GetSize() )
        {
            apl_errprintf("FLR storage write metadata file fail,%s (Path=%s)\n",
                apl_strerror(apl_get_errno() ),
                lacTemp);

            return ERROR_WRITE_FILE;
        }
    }
    
    {//create segment node storage file
        acl::CFile loFile;

        apl_size_t luFileSize = auSlotCapacity * auSegmentCapacity * auSegmentCount;
        
        for (apl_size_t luN = 0; luN < auFileCount; luN++)
        {
            acl::CFile loFile;

            apl_snprintf(lacTemp, sizeof(lacTemp), "%s/%s.%"APL_PRIuINT, apcPath, GFQ2_DATA_FILE_NAME, luN);

            if (loFile.Open(lacTemp, APL_O_CREAT | APL_O_RDWR, 0644) != 0)
            {
                apl_errprintf("FLR storage create data file fail,%s (Path=%s)\n",
                    apl_strerror(apl_get_errno() ),
                    lacTemp);

                return ERROR_OPEN_FILE;
            }

            if (loFile.Truncate(luFileSize) != 0)
            {
                apl_errprintf("FLR storage allocate data file fail,%s (Path=%s)\n",
                    apl_strerror(apl_get_errno() ),
                    lacTemp);

                return ERROR_NO_ENOUGH_SPACE;
            }
        }
    }

    return 0;
}
    
apl_int_t CFLRStorage::Open( char const* apcPath )
{
    char lacTempName[APL_NAME_MAX];
    
    apl_snprintf(lacTempName, sizeof(lacTempName), "%s/%s", apcPath, GFQ2_HEADER_FILE_NAME );
   
    {//open storage file
        if (this->moMemMap.Map(lacTempName, PROT_WRITE | PROT_READ, MAP_SHARED) != 0)
        {
            apl_errprintf("FLR mmap file %s fail, %s\n", lacTempName, apl_strerror(apl_get_errno() ) );
            
            return ERROR_MAP_FILE;
        }

        ACL_ASSERT(this->moMemMap.GetSize() > GFQ2_METADATA_LENGTH_MAX);
        
        acl::CMemoryBlock loBuffer(
            (char*)this->moMemMap.GetAddr(),
            GFQ2_METADATA_LENGTH_MAX,
            acl::CMemoryBlock::DONT_DELETE);
        
        loBuffer.SetWritePtr(GFQ2_METADATA_LENGTH_MAX);
        
        if (this->moMetadata.Decode(&loBuffer) != 0)
        {
            apl_errprintf("FLR storage metadata decode fail, maybe invalid format\n");

            return ERROR_INVALID_DATA;
        }

        //Check storage type
        if ( apl_strcmp(this->moMetadata.Get(MD_TAG_TYPE, ""), "flr") != 0)
        {
            apl_errprintf("FLR storage type check fail, non-flr-storage\n");

            return ERROR_INVALID_DATA;
        }

        //Check storage version
        //...

        this->muFileCount = this->moMetadata.GetInt(MD_TAG_FILE_COUNT, -1);
        this->muSegmentCount = this->moMetadata.GetInt(MD_TAG_SEGMENT_COUNT, -1);
        this->muSegmentCapacity = this->moMetadata.GetInt(MD_TAG_SEGMENT_CAPACITY, -1);
        this->muSlotCapacity = this->moMetadata.GetInt(MD_TAG_SLOT_CAPACITY, -1);
        this->muFileCapacity = this->muSegmentCount * this->muSegmentCapacity;
    }

    {
        ACL_NEW_N_ASSERT(this->mpoFiles, acl::CFile, this->muFileCount);
        ACL_NEW_N_ASSERT(this->mpoLocks, acl::CLock, this->muFileCount);

        apl_size_t luFileSize = this->muFileCapacity * this->muSlotCapacity;

        for (apl_size_t luN = 0; luN < this->muFileCount; luN++)
        {
            apl_snprintf(lacTempName, sizeof(lacTempName), 
                "%s/%s.%"APL_PRIuINT, apcPath, GFQ2_DATA_FILE_NAME, luN);
            
            if (this->mpoFiles[luN].Open(lacTempName, APL_O_CREAT | APL_O_RDWR, 0755) != 0)
            {
                apl_errprintf("FLR open file %s fail, %s\n", lacTempName, apl_strerror(apl_get_errno() ) );
                
                this->Close();

                return ERROR_OPEN_FILE;
            }

            if (this->mpoFiles[luN].GetFileSize() != luFileSize)
            {
                apl_errprintf(
                    "FLR check file size fail, Invalid data file, "
                    "(File=%s,FileSize=%"APL_PRIuINT",ExpectSize=%"APL_PRIuINT")\n",
                    lacTempName, 
                    this->mpoFiles[luN].GetFileSize(),
                    luFileSize );

                this->Close();
                
                return ERROR_INVALID_FILE;
            }
        }
    }

    return 0;
}

void CFLRStorage::Close(void)
{
    this->moMemMap.Unmap();
    
    for (apl_size_t luN = 0; luN < this->muFileCount; luN++)
    {
        this->mpoFiles[luN].Close();
    }
    
    ACL_DELETE_N(this->mpoFiles);
    ACL_DELETE_N(this->mpoLocks);
}

apl_int_t CFLRStorage::Write( apl_size_t auIndex, char const* apcBuffer, apl_size_t auLength )
{
    apl_size_t luFile = auIndex / this->muFileCapacity;
    apl_size_t luOffset = auIndex % this->muFileCapacity;
    CFLRRecordHeader loHeader = {auLength, apl_time() / APL_TIME_SEC};
    
    ACL_ASSERT(luFile < this->muFileCount);

    if (sizeof(loHeader) + loHeader.miLength > this->muSlotCapacity)
    {
        return ERROR_OUT_OF_RANGE;
    }
    
    apl_iovec_t laoIovec[2];

    laoIovec[0].iov_base = &loHeader;
    laoIovec[0].iov_len = sizeof(loHeader);
    laoIovec[1].iov_base = const_cast<char*>(apcBuffer);
    laoIovec[1].iov_len = auLength;
    
    acl::TSmartLock<acl::CLock> loGuard(this->mpoLocks[luFile]);

    if (this->mpoFiles[luFile].WriteV(luOffset * this->muSlotCapacity, laoIovec, 2) 
        == (apl_ssize_t)(sizeof(loHeader) + auLength) )
    {
        return 0;
    }
    else
    {
        apl_errprintf("FLR file write fail, %s, (FileNo=%"APL_PRIuINT",Offset=%"APL_PRIuINT",Length=%"APL_PRIuINT")\n",
            apl_strerror(apl_get_errno() ), 
            luOffset * this->muSlotCapacity,
            (apl_size_t)(sizeof(loHeader) + auLength) );

        return ERROR_WRITE_FILE;
    }
}
    
apl_int_t CFLRStorage::Read( apl_size_t auIndex, acl::CMemoryBlock& aoData, acl::CTimestamp* apoTimestamp )
{
    apl_ssize_t liResult = 0;
    apl_size_t  luFile = auIndex / this->muFileCapacity;
    apl_size_t  luOffset = auIndex % this->muFileCapacity;
    CFLRRecordHeader loHeader = {0, 0};
    apl_int32_t liMaxLength = this->muSlotCapacity - sizeof(loHeader);

    ACL_ASSERT(luFile < this->muFileCount);
    
    aoData.Resize(liMaxLength);
    
    apl_iovec_t laoIovec[2];

    laoIovec[0].iov_base = &loHeader;
    laoIovec[0].iov_len = sizeof(loHeader);
    laoIovec[1].iov_base = aoData.GetWritePtr();
    laoIovec[1].iov_len = liMaxLength;

    {
        acl::TSmartLock<acl::CLock> loGuard(this->mpoLocks[luFile]);

        liResult = this->mpoFiles[luFile].ReadV(luOffset * this->muSlotCapacity, laoIovec, 2);
    }

    if (liResult == (apl_ssize_t)this->muSlotCapacity && loHeader.miLength <= liMaxLength)
    {
        aoData.SetWritePtr(loHeader.miLength);

        if (apoTimestamp != APL_NULL)
        {
            apoTimestamp->Sec(loHeader.miTimestamp);
        }
        
        return 0;
    }
    else
    {
        apl_errprintf("FLR file read fail, %s, (FileNo=%"APL_PRIuINT",Offset=%"APL_PRIuINT",Length=%"APL_PRIuINT")\n",
            apl_strerror(apl_get_errno() ),
            luFile,
            luOffset * this->muSlotCapacity,
            (apl_size_t)(liMaxLength) );

        return liResult == (apl_ssize_t)this->muSlotCapacity ? ERROR_INVALID_DATA : ERROR_READ_FILE;
    }
}

void* CFLRStorage::GetHeader(void)
{
    return ((char*)this->moMemMap.GetAddr() ) + GFQ2_METADATA_LENGTH_MAX;
}

apl_size_t CFLRStorage::GetHeaderLength(void)
{
    return this->moMemMap.GetSize() - GFQ2_METADATA_LENGTH_MAX;
}
char const* CFLRStorage::GetVersion(void)
{
    return this->moMetadata.Get(MD_TAG_VERSION, "unknown");
}

char const* CFLRStorage::GetType(void)
{
    return this->moMetadata.Get(MD_TAG_TYPE, "unknown");
}

char const* CFLRStorage::GetCreateTime(void)
{
    return this->moMetadata.Get(MD_TAG_CREATE_TIME, "unknown");
}

apl_size_t CFLRStorage::GetSegmentCount(void)
{
    return this->muSegmentCount * this->muFileCount;
}

apl_size_t CFLRStorage::GetSegmentCapacity(void)
{
    return this->muSegmentCapacity;
}

apl_size_t CFLRStorage::GetSlotCapacity(void)
{
    return this->muSlotCapacity - sizeof(CFLRRecordHeader);
}

apl_size_t CFLRStorage::GetFileCount(void)
{
    return this->muFileCount;
}

AIBC_GFQ_NAMESPACE_END

