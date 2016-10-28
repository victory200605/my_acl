
#include "BufferPool.h"
#include "Storage.h"

AIBC_GFQ_NAMESPACE_START

#define CACHE_FILE_NAME      "gfq_cache"
#define METADATA_MAX_LENGTH  4096

namespace 
{
    apl_int_t MakeKey( char const* apcName )
    {
        apl_int_t liKey = 0;
        acl::CFileInfo loFileInfo;

        {//Create local file to make shm-key
            acl::CFile loFile;

            loFile.Open(apcName, APL_O_CREAT, 0644);
        }

        if (acl::CFile::GetFileInfo(apcName, loFileInfo) == 0)
        {
            liKey = ((loFileInfo.GetDev() & 0xffff) << 16 ) | (loFileInfo.GetIno() & 0xffff);
        }

        return liKey;
    }

    void* AllocateBufferSHM(
        char const* apcPath,
        acl::CIndexDict& aoMetadata, 
        apl_int_t aiBufferSize,
        acl::CMemMap& aoMemMap )
    {
        acl::CFile loFile;
        acl::CMemoryBlock loMetadataBuffer(METADATA_MAX_LENGTH);
        char lacTemp[APL_NAME_MAX];
        apl_int_t liShmKey = 0;

        apl_snprintf(lacTemp, sizeof(lacTemp), "%s/%s", apcPath, CACHE_FILE_NAME);

        if (loFile.Open(lacTemp, APL_O_CREAT|APL_O_RDWR, 0644) != 0)
        {
            apl_errprintf("BufferPool create cache metadata file fail,%s (Path=%s)\n",
                apl_strerror(apl_get_errno() ),
                lacTemp);

            return APL_NULL;
        }

        if (aoMetadata.Encode(&loMetadataBuffer) != 0)
        {
            apl_errprintf("BufferPool encode metadata fail,maybe too long\n");

            return APL_NULL;
        }
        
        if (loFile.Write(
            loMetadataBuffer.GetBase(), loMetadataBuffer.GetSize() ) != (apl_ssize_t)(loMetadataBuffer.GetSize() ) )
        {
            apl_errprintf("BufferPool write cache metadata file fail,%s (Path=%s)\n",
                apl_strerror(apl_get_errno() ),
                lacTemp);

            return APL_NULL;
        }

        loFile.Close();

        //Create share memory file
        liShmKey = MakeKey(lacTemp);

        apl_snprintf(lacTemp, sizeof(lacTemp), "/gfq2cache-shm-%"APL_PRIdINT"", liShmKey);

        if (loFile.ShmOpen(lacTemp, APL_O_CREAT|APL_O_RDWR, 0644) != 0)
        {
            apl_errprintf("BufferPool create cache file fail,%s (Path=%s)\n",
                apl_strerror(apl_get_errno() ),
                lacTemp);

            return APL_NULL;
        }
        
        if (loFile.Truncate(aiBufferSize) != 0)
        {
            apl_errprintf("BufferPool allocate cache file fail,%s (Path=%s)\n",
                apl_strerror(apl_get_errno() ),
                lacTemp);

            return APL_NULL;
        }

        if (aoMemMap.ShmMap(lacTemp, APL_PROT_READ|APL_PROT_WRITE, APL_MAP_SHARED) != 0)
        {
            apl_errprintf("BufferPool map cache file fail,%s (Path=%s)\n",
                apl_strerror(apl_get_errno() ),
                lacTemp);

            return APL_NULL;
        }
        
        return aoMemMap.GetAddr();
    }

    void* AllocateBufferFIM(
        char const* apcPath,
        acl::CIndexDict& aoMetadata,
        apl_int_t aiBufferSize,
        acl::CMemMap& aoMemMap )
    {
        acl::CFile loFile;
        char lacTemp[APL_NAME_MAX];

        apl_snprintf(lacTemp, sizeof(lacTemp), "%s/%s", apcPath, CACHE_FILE_NAME);

        if (loFile.Open(lacTemp, APL_O_CREAT|APL_O_RDWR, 0644) != 0)
        {
            apl_errprintf("BufferPool create cache file fail,%s (Page=%s)\n",
                apl_strerror(apl_get_errno() ),
                lacTemp);

            return APL_NULL;
        }
        
        if (loFile.Truncate(aiBufferSize + METADATA_MAX_LENGTH) != 0)
        {
            apl_errprintf("BufferPool allocate cache file fail,%s (Path=%s)\n",
                apl_strerror(apl_get_errno() ),
                lacTemp);

            return APL_NULL;
        }

        if (aoMemMap.Map(lacTemp, APL_PROT_READ|APL_PROT_WRITE, APL_MAP_SHARED) != 0)
        {
            apl_errprintf("BufferPool map cache file fail,%s (Path=%s)\n",
                apl_strerror(apl_get_errno() ),
                lacTemp);

            return APL_NULL;
        }

        acl::CMemoryBlock loTempBuffer((char*)aoMemMap.GetAddr(), METADATA_MAX_LENGTH, acl::CMemoryBlock::DONT_DELETE);

        if (aoMetadata.Encode(&loTempBuffer) != 0)
        {
            apl_errprintf("BufferPool encode metadata fail,maybe too long\n");

            return APL_NULL;
        }

        return ((char*)aoMemMap.GetAddr() ) + METADATA_MAX_LENGTH;
    }
};

CBufferPool::CPageEntry::CPageEntry(void)
    : miPageID(-1)
    , miIndex(-1)
    , mpcBuffer(APL_NULL)
{
    ACL_RING_ELEM_INIT(this, moEntry);
}

CBufferPool::CBufferPool(void)
    : mpoStorage(APL_NULL)
    , mpiIndex(APL_NULL)
    , mpcPage(APL_NULL)
    , miBucketNum(0)
    , miPageCount(0)
    , mpoEntry(APL_NULL)
    , miFreePageCount(0)
    , miUsedPageCount(0)
    , mu64ReadTimes(0)
    , mu64ReadMissedTimes(0)
    , mu64WriteTimes(0)
    , mu64WriteMissedTimes(0)
    , mu64SyncTimes(0)
{
    ACL_RING_INIT(&this->moFree, CPageEntry, moEntry);
    ACL_RING_INIT(&this->moUsed, CPageEntry, moEntry);
}

CBufferPool::~CBufferPool(void)
{
    this->Dump();
}

apl_int_t CBufferPool::Initialize(
    CStorage* apoStorage,
    char const* apcPath,
    char const* apcType,
    apl_int_t aiBucketNum,
    apl_int_t aiPageSize,
    apl_int_t aiBufferSize )
{
    char lacTemp[APL_NAME_MAX];

    apl_snprintf(lacTemp, sizeof(lacTemp), "%s/%s", apcPath, CACHE_FILE_NAME);

    this->mpoStorage = apoStorage;
    this->moPath = apcPath;
    this->moType = apcType;

    if (acl::CFile::Access(lacTemp, APL_F_OK) == 0)
    {
        this->ReflushBufferPool(apcPath);
    }
    
    return this->Create(apcPath, apcType, aiBucketNum, aiPageSize, aiBufferSize);
}

apl_int_t CBufferPool::ReflushBufferPool( char const* apcPath )
{
    apl_int_t liRetCode = 0;
    acl::CIndexDict loMetadata;
    char lacTemp[APL_NAME_MAX];
    acl::CMemMap loMemMap;
    acl::CMemMap loShmMemMap;
    void* lpvBuffer = APL_NULL;
    apl_int_t liSize = 0;
    
    apl_snprintf(lacTemp, sizeof(lacTemp), "%s/%s", apcPath, CACHE_FILE_NAME);
    
    if (loMemMap.Map(lacTemp, APL_PROT_READ|APL_PROT_WRITE, MAP_SHARED) != 0)
    {
        apl_errprintf("BufferPool map cache file fail, %s\n", apl_strerror(apl_get_errno() ) );

        return ERROR_MAP_FILE;
    }

    if (loMemMap.GetSize() < METADATA_MAX_LENGTH)
    {
        apl_errprintf("BufferPool invalid cache file, (ExpectSize>%"APL_PRIuINT")\n", 
            (apl_size_t)(METADATA_MAX_LENGTH) );

        return ERROR_INVALID_FILE;
    }

    acl::CMemoryBlock loTempBuffer((char*)loMemMap.GetAddr(), METADATA_MAX_LENGTH, acl::CMemoryBlock::DONT_DELETE);
    loTempBuffer.SetWritePtr(METADATA_MAX_LENGTH);
    if (loMetadata.Decode(&loTempBuffer) != 0)
    {
        apl_errprintf("BufferPool decode metadata fail,maybe format invalid\n");

        return ERROR_INVALID_FILE;
    }
    
    std::string loType = loMetadata.Get(METADATA_TYPE, "");
    if (loType == "fim")
    {
        lpvBuffer = ((char*)loMemMap.GetAddr())+ METADATA_MAX_LENGTH;
        liSize = loMemMap.GetSize() - METADATA_MAX_LENGTH;
    }
    else
    {
        apl_int_t liShmKey = MakeKey(lacTemp);
    
        apl_snprintf(lacTemp, sizeof(lacTemp), "/gfq2cache-shm-%"APL_PRIdINT"", liShmKey);
        
        if (loShmMemMap.ShmMap(lacTemp, APL_PROT_READ|APL_PROT_WRITE, MAP_SHARED) != 0)
        {
            apl_errprintf("BufferPool map cache shm-file fail, %s (Path=%s)\n", 
                apl_strerror(apl_get_errno() ),
                lacTemp);

            return ERROR_MAP_FILE;
        }

        lpvBuffer = loShmMemMap.GetAddr();
        liSize = loShmMemMap.GetSize();
    }
    
    if ( (liRetCode = this->Open(loMetadata, lpvBuffer, liSize) ) != 0)
    {
        return liRetCode;
    }
    
    //Check checkpoint if existed un-commit transaction and rollback
    if ( (liRetCode = this->CheckCheckpointAndRecover() ) != 0)
    {
        return liRetCode;
    }

    this->Close();

    return 0;
}

apl_int_t CBufferPool::Create(
    char const* apcPath,
    char const* apcType,
    apl_int_t aiBucketNum, 
    apl_int_t aiPageSize, 
    apl_int_t aiBufferSize )
{
    //-------------------------------------------------------------
    //| page index (bucketnum*4 byte)                             |
    //-------------------------------------------------------------
    //| checkpoint (sizeof(CBufferPoolCheckpoint) + pagesize byte)        |
    //-------------------------------------------------------------
    //| page ...                                                  |
    //-------------------------------------------------------------
    apl_int_t liRetCode = 0;
    acl::CIndexDict loMetadata;
    void* lpvBuffer = APL_NULL;
    apl_int_t liHeaderSize = aiBucketNum * sizeof(apl_int32_t);
    apl_int_t liCheckpointSize = sizeof(CBufferPoolCheckpoint) + aiPageSize;
    apl_int_t liPageBufferSize = aiBufferSize;
    apl_int_t liTotalSize = liHeaderSize + liCheckpointSize + liPageBufferSize;

    loMetadata.Set(METADATA_TYPE, apcType);
    loMetadata.Set(METADATA_BUCKET_NUM, aiBucketNum);
    loMetadata.Set(METADATA_PAGE_SIZE, aiPageSize);
    loMetadata.Set(METADATA_HEADER_SIZE, liHeaderSize);
    loMetadata.Set(METADATA_CHECKPOINT_SIZE, liCheckpointSize);
    loMetadata.Set(METADATA_BUFFER_SIZE, aiBufferSize);

    if (apl_strcmp(apcType, "fim") == 0)
    {
        lpvBuffer = AllocateBufferFIM(apcPath, loMetadata, liTotalSize, this->moMemMap);
    }
    else
    {
        lpvBuffer = AllocateBufferSHM(apcPath, loMetadata, liTotalSize, this->moMemMap);
    }
    
    if (lpvBuffer == APL_NULL)
    {
        return ERROR_SYSTEM;
    }

    if ( (liRetCode = this->Open(loMetadata, lpvBuffer, liTotalSize) ) != 0)
    {
        return liRetCode;
    }

    //Initialize buffer pool
    for (apl_int_t liN = 0; liN < this->miBucketNum; liN++)
    {
        this->mpiIndex[liN] = -1;
    }

    //Create cache page link
    ACL_NEW_N_ASSERT(this->mpoEntry, CPageEntry, this->miPageCount);

    for (apl_int_t liN = 0; liN < this->miPageCount; liN++)
    {
        ACL_RING_INSERT_TAIL(&this->moFree, &this->mpoEntry[liN], CPageEntry, moEntry);

        this->mpoEntry[liN].mpcBuffer = this->mpcPage + liN * this->mpoStorage->GetPageSize();
        this->mpoEntry[liN].miIndex = liN;
    }

    this->miFreePageCount = this->miPageCount;
    
    return 0;
}

apl_int_t CBufferPool::Open( acl::CIndexDict const& aoMetadata, void* apvBuffer, apl_int_t aiSize )
{
    char* lpcBase = (char*)(apvBuffer);
    apl_int_t liPageSize = aoMetadata.GetInt(METADATA_PAGE_SIZE, -1);
    apl_int_t liBucketNum = aoMetadata.GetInt(METADATA_BUCKET_NUM, -1);
    apl_int_t liBufferSize = aoMetadata.GetInt(METADATA_BUFFER_SIZE, -1);
    apl_int_t liHeaderSize = aoMetadata.GetInt(METADATA_HEADER_SIZE, -1);
    apl_int_t liCheckpointSize = aoMetadata.GetInt(METADATA_CHECKPOINT_SIZE, -1);

    ACL_ASSERT(aiSize == liHeaderSize + liCheckpointSize + liBufferSize);

    this->mpiIndex = (apl_int32_t*)(lpcBase);
    this->miBucketNum = liBucketNum;
    this->miPageCount = liBufferSize / liPageSize;
    this->mpoCheckpoint = (CBufferPoolCheckpoint*)(lpcBase + liHeaderSize);
    this->mpcPage = (char*)(lpcBase + liHeaderSize + liCheckpointSize);

    return 0;
}

void CBufferPool::Close(void)
{
    char lacTemp[APL_NAME_MAX];

    apl_snprintf(lacTemp, sizeof(lacTemp), "%s/%s", this->moPath.c_str(), CACHE_FILE_NAME);

    //Sync current buffer pool when exception exited last time
    this->Sync();

    if (this->moType == "shm")
    {
        char lacShmFile[APL_NAME_MAX];
        apl_int_t liShmKey = 0;

        liShmKey = MakeKey(lacTemp);
        apl_snprintf(lacShmFile, sizeof(lacShmFile), "/gfq2cache-shm-%"APL_PRIdINT"", liShmKey);
        
        acl::CFile::Remove(lacShmFile);
    }
    
    acl::CFile::Remove(lacTemp);
    
    ACL_DELETE_N(this->mpoEntry);
}

apl_ssize_t CBufferPool::Read(
    apl_int_t aiPageID,
    apl_int32_t aiOffset,
    void* apvBuffer,
    apl_size_t auLen )
{
    apl_int_t liRetCode = 0;
    char* lpcPage = APL_NULL;

    apl_int_t liReadable = this->mpoStorage->GetPageSize() - aiOffset;
    apl_int_t liReadSize = liReadable > (apl_int_t)auLen ? (apl_int_t)auLen : liReadable;

    this->mu64ReadTimes++;

    if ( (liRetCode = this->Get(aiPageID, &this->mu64ReadMissedTimes, true, &lpcPage) ) != 0)
    {
        return liRetCode;
    }

    apl_memcpy(apvBuffer, lpcPage + aiOffset, liReadSize);

    return liReadSize;
}

apl_ssize_t CBufferPool::Write(
    apl_int_t aiPageID,
    apl_int32_t aiOffset,
    void const* apvBuffer,
    apl_size_t auLen,
    bool abIsAtomic )
{
    apl_int_t liRetCode = 0;
    char* lpcPage = APL_NULL;

    apl_int_t liWritable = this->mpoStorage->GetPageSize() - aiOffset;
    apl_int_t liWriteSize = liWritable > (apl_int_t)auLen ? (apl_int_t)auLen : liWritable;
    
    this->mu64WriteTimes++;

    if ( (liRetCode = this->Get(aiPageID, &this->mu64WriteMissedTimes, true, &lpcPage) ) != 0)
    {
        return liRetCode;
    }

    if (abIsAtomic)
    {
        //Start transaction
        this->mpoCheckpoint->miPageID = aiPageID;
        this->mpoCheckpoint->miOffset = aiOffset;
        this->mpoCheckpoint->miLength = auLen;
        apl_memcpy(this->mpoCheckpoint->macBuffer, apvBuffer, auLen);
        this->mpoCheckpoint->miEnable = true;
    }

    apl_memcpy(lpcPage + aiOffset, apvBuffer, liWriteSize);

    this->mpoCheckpoint->miEnable = false;

    return liWriteSize;
}
    
apl_int_t CBufferPool::Preallocate( apl_int_t aiPageID )
{
    char* lpcPage = APL_NULL;
    apl_uint64_t lu64MissedTimes = 0;

    return this->Get(aiPageID, &lu64MissedTimes, false, &lpcPage);
}

apl_int_t CBufferPool::Hot( apl_int_t aiPageID )
{
    if (this->mpiIndex[aiPageID] >= 0)
    {
        CPageEntry* lpoEntry = &this->mpoEntry[this->mpiIndex[aiPageID] ];

        ACL_RING_REMOVE(lpoEntry, moEntry);
        ACL_RING_INSERT_TAIL(&this->moUsed, lpoEntry, CPageEntry, moEntry);

        return 0;
    }
    else
    {
        return -1;
    }
}

apl_int_t CBufferPool::Erase( apl_int_t aiPageID )
{
    if (this->mpiIndex[aiPageID] >= 0)
    {
        CPageEntry* lpoPage = &this->mpoEntry[this->mpiIndex[aiPageID] ];

        ACL_RING_REMOVE(lpoPage, moEntry);
        ACL_RING_INSERT_TAIL(&this->moFree, lpoPage, CPageEntry, moEntry);
        
        this->miUsedPageCount--;
        this->miFreePageCount++;

        this->mpiIndex[aiPageID] = -1;
    }

    return 0;
}

apl_int_t CBufferPool::Sync( apl_int_t aiPageID )
{
    if (this->mpiIndex[aiPageID] >= 0)
    {
        apl_int_t liPageSize = this->mpoStorage->GetPageSize();
        char* lpcPage = this->mpcPage + this->mpiIndex[aiPageID] * liPageSize;

        if (this->mpoEntry != APL_NULL)
        {
            CPageEntry* lpoPage = &this->mpoEntry[this->mpiIndex[aiPageID] ];

            ACL_RING_REMOVE(lpoPage, moEntry);
            ACL_RING_INSERT_TAIL(&this->moFree, lpoPage, CPageEntry, moEntry);
            
            this->miUsedPageCount--;
            this->miFreePageCount++;
        }
        
        if (this->mpoStorage->Write(aiPageID, 0, lpcPage, liPageSize) != liPageSize)
        {
            return ERROR_WRITE_FILE;
        }

        this->mu64SyncTimes++;
        
        this->mpiIndex[aiPageID] = -1;
    }
    
    return 0;
}

apl_int_t CBufferPool::Sync(void)
{
    apl_int_t liRetCode = 0;
    
    for (apl_int_t liI = 0; liI < this->miBucketNum; liI++)
    {
        //Sync page
        if ( (liRetCode = this->Sync(liI) ) != 0)
        {
            return liRetCode;
        }
    }

    return 0;
}

apl_int_t CBufferPool::Get(
    apl_int_t aiPageID,
    apl_uint64_t* apu64MissedTimes,
    bool abIsMissedAndLoad,
    char** apcPage )
{
    apl_int_t liRetCode = 0;
    apl_int_t liPageSize = this->mpoStorage->GetPageSize();
    CPageEntry* lpoEntry = APL_NULL;

    if (this->mpiIndex[aiPageID] < 0)
    {
        (*apu64MissedTimes)++;

        //Load page
        while(true)
        {
            if (ACL_RING_IS_EMPTY(&this->moFree, CPageEntry, moEntry) )
            {
                if ( (liRetCode = this->ClearExpirePage() ) == 0)
                {
                    continue;
                }

                return liRetCode;
            }

            CPageEntry* lpoFirst = ACL_RING_FIRST(&this->moFree);
            lpoFirst->miPageID = aiPageID;

            if (abIsMissedAndLoad)
            {
                if (this->mpoStorage->Read(aiPageID, 0, lpoFirst->mpcBuffer, liPageSize) != liPageSize)
                {
                    return ERROR_READ_FILE;
                }
            }
            
            ACL_RING_REMOVE(lpoFirst, moEntry);
            ACL_RING_INSERT_TAIL(&this->moUsed, lpoFirst, CPageEntry, moEntry);
            
            this->miUsedPageCount++;
            this->miFreePageCount--;
            
            this->mpiIndex[aiPageID] = lpoFirst->miIndex;
            lpoEntry = lpoFirst;

            break;
        }
    }
    else
    {
        lpoEntry = &this->mpoEntry[this->mpiIndex[aiPageID] ];

        ACL_RING_REMOVE(lpoEntry, moEntry);
        ACL_RING_INSERT_TAIL(&this->moUsed, lpoEntry, CPageEntry, moEntry);
    }
    
    *apcPage = lpoEntry->mpcBuffer;

    return 0;
}

apl_int_t CBufferPool::ClearExpirePage(void)
{
    CPageEntry* lpoFirst = ACL_RING_FIRST(&this->moUsed);

    return this->Sync(lpoFirst->miPageID);
}

apl_int_t CBufferPool::CheckCheckpointAndRecover(void)
{
    if (this->mpoCheckpoint->miEnable)
    {
        ACL_ASSERT(this->mpiIndex[this->mpoCheckpoint->miPageID] >= 0);
        
        apl_int_t liPageSize = this->mpoStorage->GetPageSize();
        char* lpcPage = this->mpcPage + this->mpiIndex[this->mpoCheckpoint->miPageID] * liPageSize;

        apl_memcpy(
            lpcPage + this->mpoCheckpoint->miOffset, 
            this->mpoCheckpoint->macBuffer, 
            this->mpoCheckpoint->miLength
        );

        this->mpoCheckpoint->miEnable = false;
    }

    return 0;
}

void CBufferPool::Dump(void)
{
    printf("buffer pool read times         = %"APL_PRIu64"\n", this->mu64ReadTimes);
    printf("buffer pool read missed times  = %"APL_PRIu64"\n", this->mu64ReadMissedTimes);
    printf("buffer pool write times        = %"APL_PRIu64"\n", this->mu64WriteTimes);
    printf("buffer pool write missed times = %"APL_PRIu64"\n", this->mu64WriteMissedTimes);
    printf("buffer pool sync times         = %"APL_PRIu64"\n", this->mu64SyncTimes);
}

AIBC_GFQ_NAMESPACE_END

