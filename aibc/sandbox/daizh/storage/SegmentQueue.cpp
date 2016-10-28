
#include "SegmentQueue.h"
#include "Page.h"
#include "acl/Dir.h"

AIBC_GFQ_NAMESPACE_START

enum
{
    CHECKPOINT_ALLOCATE = 1,
    CHECKPOINT_DEALLOCATE
};

CSegmentQueue::CSegmentQueue(void)
    : mpoStorageHeader(APL_NULL)
    , mpoSegmentHeader(APL_NULL)
    , mpiNextPageEntry(APL_NULL)
    , miSegmentCount(0)
    , miSegmentCapacity(0)
    , miPageSize(0)
    , miTotalPageCount(0)
    , mpoLock(APL_NULL)
{
}

CSegmentQueue::~CSegmentQueue(void)
{
}

apl_int_t CSegmentQueue::Create( char const* apcPath, CStorageMetadata& aoMetadata )
{
    //Segment Queue Storage Header
    //-----------------------------
    //|Metadata                   |
    //-----------------------------
    //|Segment Header 0           |
    //-----------------------------
    //|Segment Header 1           |
    //-----------------------------
    // ...
    //-----------------------------
    //|Segment Header N           |
    //-----------------------------
    //|Page Next Entry 0          |
    //-----------------------------
    //|Page Next Entry 1          |
    //-----------------------------
    // ...
    //-----------------------------
    //|Page Next Entry N          |
    //-----------------------------
    CStorage loStorage;
    apl_int_t liRetCode = 0;

    apl_int_t liSegmentCount = aoMetadata.GetInt(METADATA_SEGMENT_COUNT, -1);
    apl_int_t liFileCount = aoMetadata.GetInt(METADATA_FILE_COUNT, -1);
    apl_int_t liPageCount = aoMetadata.GetInt(METADATA_PAGE_COUNT, -1);
    apl_int_t liTotalPageCount = liFileCount*liPageCount;
    apl_int_t liHeaderSize = 
        sizeof(CStorageHeader) + sizeof(CSegmentHeader) * liSegmentCount + 4*liTotalPageCount;
    
    aoMetadata.Set(METADATA_VERSION, "1.0.0");
    aoMetadata.Set(METADATA_HEADER_SIZE, liHeaderSize);
    aoMetadata.Set(METADATA_TOTAL_PAGE_COUNT, liFileCount*liPageCount);

    if ( (liRetCode = CStorage::Create(apcPath, aoMetadata) ) != 0)
    {
        return liRetCode;
    }

    if ( (liRetCode = loStorage.Open(apcPath) ) != 0)
    {
        return liRetCode;
    }

    //Initialize header
    {
        apl_int_t liN = 0;
        CStorageHeader* lpoStorage = static_cast<CStorageHeader*>(loStorage.GetHeader() );

        ACL_ASSERT(liHeaderSize == (apl_int_t)loStorage.GetHeaderLength() );

        lpoStorage->miFreePageCount = liTotalPageCount;
        lpoStorage->miFirstFreePage = 0;
        
        //Initialize Segment Header
        for (liN = 0; liN < liSegmentCount; liN++)
        {
            CSegmentHeader& loSegmentHeader = lpoStorage->maoSegmentHeader[liN];
            
            loSegmentHeader.miFree = true;
            loSegmentHeader.miSegmentID = liN;
            loSegmentHeader.miReadPage = -1;
            loSegmentHeader.miWritePage = -1;
            loSegmentHeader.mi64Timestamp = 0;
            loSegmentHeader.miReadOffset = 0;
            loSegmentHeader.miSize = 0;
        }
        
        //Initialize Page Allocate Link
        apl_int32_t* lpiEntry = (apl_int32_t*)&lpoStorage->maoSegmentHeader[liSegmentCount];
        for (liN = 0; liN < liTotalPageCount - 1; liN++)
        {
            lpiEntry[liN] = liN + 1;
        }
        lpiEntry[liN] = -1;

        //Initialize Checkpoint
        lpoStorage->moCheckpoint.miEnable = false;
        lpoStorage->moCheckpoint.miType = 0;
    }

    return 0;
}

apl_int_t CSegmentQueue::Open( char const* apcPath, char const* apcType, apl_int_t aiBufferPoolSize )
{
    apl_int_t liRetCode = 0;
    CStorageMetadata* lpoMetadata = APL_NULL;
    
    if ( (liRetCode = this->moStorage.Open(apcPath) ) != 0)
    {
        return liRetCode;
    }
    
    lpoMetadata = this->moStorage.GetMetadata();

    lpoMetadata->Dump();

    this->mpoStorageHeader = static_cast<CStorageHeader*>(this->moStorage.GetHeader() );
    this->mpoSegmentHeader = this->mpoStorageHeader->maoSegmentHeader; 
    this->miSegmentCount = lpoMetadata->GetInt(METADATA_SEGMENT_COUNT, -1);
    this->miSegmentCapacity = lpoMetadata->GetInt(METADATA_SEGMENT_CAPACITY, -1);
    this->miPageSize = lpoMetadata->GetInt(METADATA_PAGE_SIZE, -1);
    this->miTotalPageCount = lpoMetadata->GetInt(METADATA_TOTAL_PAGE_COUNT, -1);
    this->mpiNextPageEntry = (apl_int32_t*)(&this->mpoSegmentHeader[this->miSegmentCount]);

    if ( (liRetCode = this->moBufferPool.Initialize(
        &this->moStorage,
        apcPath, 
        apcType,
        this->miTotalPageCount,
        this->miPageSize,
        aiBufferPoolSize) ) != 0)
    {
        return liRetCode;
    }

    ACL_NEW_N_ASSERT(this->mpoLock, acl::CLock, this->miSegmentCount);
    
    if ( (liRetCode = this->CheckCheckpointAndRecover() ) != 0)
    {
        return liRetCode;
    }

    if ( (liRetCode = this->IntegrityCheck() ) != 0)
    {
        return liRetCode;
    }

    return 0;
}

apl_int_t CSegmentQueue::IntegrityCheck(void)
{
    apl_int_t liRetCode = 0;
    apl_int_t liFreePageCount = 0;
    apl_int_t liUsedPageCount = 0;

    //Check referenced page
    for (apl_int_t liN = 0; liN < this->miSegmentCount; liN++)
    {
        CSegmentHeader& loSegmentHeader = this->mpoSegmentHeader[liN];
        apl_int_t liReadPage = loSegmentHeader.miReadPage;
        apl_int_t liWritePage = loSegmentHeader.miWritePage;
        apl_int_t liCount = 0;

        if (liReadPage == -1 && liWritePage == -1)
        {
            continue;
        }

        while(true)
        {
            if ( (liRetCode = CPage(&this->moStorage, &this->moBufferPool, liReadPage).Check() ) != 0)
            {
                apl_errprintf(
                    "Integrity check fail, (SegmentID=%"APL_PRIdINT",PageID=%"APL_PRIdINT")\n",
                    liN, 
                    liReadPage);

                return liRetCode;
            }

            liCount++;
            liUsedPageCount++;

            if (liReadPage == liWritePage)
            {
                break;
            }

            if (liCount > this->miTotalPageCount)
            {
                apl_errprintf(
                    "Integrity check fail, segment page link maybe deadloop "
                    "(SegmentID=%"APL_PRIdINT",PageCount=%"APL_PRIdINT")\n",
                    liN, 
                    liCount);

                return ERROR_INVALID_FILE;
            }

            //Check next page
            liReadPage = this->mpiNextPageEntry[liReadPage];
        }
    }

    //Check free page
    {
        apl_int_t liFreePage = this->mpoStorageHeader->miFirstFreePage;
        apl_int_t liCount = 0;

        while(liFreePage != -1)
        {
            liCount++;
            liFreePageCount++;

            if (liCount > this->miTotalPageCount)
            {
                apl_errprintf(
                    "Integrity check fail, free page link maybe deadloop (PageCount=%"APL_PRIdINT")\n",
                    liCount);

                return ERROR_INVALID_FILE;
            }

            //Check next page
            liFreePage = this->mpiNextPageEntry[liFreePage];
        }
    }

    if (liUsedPageCount + liFreePageCount != this->miTotalPageCount)
    {
        apl_errprintf(
            "Integrity check fail, page lose (Used=%"APL_PRIdINT", Free=%"APL_PRIdINT", Total=%"APL_PRIdINT")\n",
            liUsedPageCount,
            liFreePageCount,
            this->miTotalPageCount);

        return ERROR_INVALID_FILE;
    }

    return 0;
}

void CSegmentQueue::Close(void)
{
    this->moBufferPool.Close();
    this->moStorage.Close();
}

apl_int_t CSegmentQueue::Allocate( apl_size_t auSegmentID, char const* apcQueueName, apl_int64_t ai64Timestamp )
{
    if (auSegmentID > (apl_size_t)this->miSegmentCount)
    {
        return ERROR_SEGMENT_UNEXISTED;
    }
    
    acl::TSmartLock<acl::CLock> loGuard(this->moLock);
    
    CSegmentHeader* lpoHeader = &this->mpoSegmentHeader[auSegmentID];
    if (lpoHeader->miFree == true)
    {
        lpoHeader->miReadPage = -1;
        lpoHeader->miWritePage = -1;
        lpoHeader->miReadOffset = 0;
        lpoHeader->miSize = 0;
        lpoHeader->mi64Timestamp = ai64Timestamp;
        apl_strncpy(lpoHeader->macQueueName, apcQueueName, sizeof(lpoHeader->macQueueName) );
        lpoHeader->miFree = false;

        return 0;
    }
    else
    {
        return ERROR_PERMISSION_DENIED;
    }
}

apl_int_t CSegmentQueue::Deallocate( apl_size_t auSegmentID )
{
    if (auSegmentID > (apl_size_t)this->miSegmentCount)
    {
        return ERROR_SEGMENT_UNEXISTED;
    }
    
    acl::TSmartLock<acl::CLock> loGuard(this->moLock);
    
    CSegmentHeader* lpoHeader = &this->mpoSegmentHeader[auSegmentID];
    if (lpoHeader->miSize <= 0)
    {
        return this->Deallocate0(lpoHeader);
    }
    else
    {
        return ERROR_PERMISSION_DENIED;
    }
}

apl_int_t CSegmentQueue::Get(
    apl_size_t auSegmentID,
    char const* apcQueueName,
    apl_int_t aiStayTime, 
    std::string& aoData )
{
    apl_int_t liRetCode = 0;
    
    if (auSegmentID > (apl_size_t)this->miSegmentCount)
    {
        return ERROR_SEGMENT_UNEXISTED;
    }
    
    acl::TSmartLock<acl::CLock> loGuard(this->moLock);
    
    CSegmentHeader* lpoHeader = &this->mpoSegmentHeader[auSegmentID];

    if (lpoHeader->miFree == true 
        || apl_strcmp(lpoHeader->macQueueName, apcQueueName) != 0)
    {
        //Permission deny
        return ERROR_PERMISSION_DENIED;
    }
 
    while(true)
    {
        if (lpoHeader->miReadPage < 0
            || (lpoHeader->miReadOffset >= lpoHeader->miSize 
                && lpoHeader->miReadPage == lpoHeader->miWritePage) )
        {
            if (lpoHeader->miSize < this->miSegmentCapacity)
            {
                //Empty segment and return
                liRetCode = ERROR_EMPTY_OF_SEGMENT;
            }
            else
            {
                liRetCode = ERROR_END_OF_SEGMENT;
            }

            break;
        }

        CPage loPage(&this->moStorage, &this->moBufferPool, lpoHeader->miReadPage);

        if ( (liRetCode = loPage.Open() ) != 0)
        {
            break;
        }
    
        if (loPage.IsEmpty() )
        {
            //Is last page and return error
            if (lpoHeader->miReadPage == lpoHeader->miWritePage)
            {
                lpoHeader->miReadOffset = lpoHeader->miSize;

                continue;
            }

            //Deallocate current read page and read next page now
            if ( (liRetCode = this->DeallocatePage(lpoHeader) ) != 0)
            {
                break;
            }

            continue;
        }

        if ( (liRetCode = loPage.Read(aiStayTime, aoData) ) != 0)
        {
            break;
        }

        lpoHeader->miReadOffset++;
       
        //apl_errprintf("read segment %"APL_PRIdINT" from page %"APL_PRIdINT"\n", aiSegmentID, lpoHeader->miReadPage);

        break;
    }

    if (lpoHeader->miReadOffset >= this->miSegmentCapacity)
    {
        apl_int_t r = this->Deallocate0(lpoHeader);
        ACL_ASSERT(r == 0);
    }

    return liRetCode;
}

apl_int_t CSegmentQueue::Put( apl_size_t auSegmentID, char const* apcQueueName, char const* apcBuffer, apl_size_t auLen )
{
    apl_int_t liRetCode = 0;

    if (auSegmentID > (apl_size_t)this->miSegmentCount)
    {
        return ERROR_SEGMENT_UNEXISTED;
    }

    acl::TSmartLock<acl::CLock> loGuard(this->moLock);

    CSegmentHeader* lpoHeader = &this->mpoSegmentHeader[auSegmentID];
    
    if (lpoHeader->miFree == true 
        || apl_strcmp(lpoHeader->macQueueName, apcQueueName) != 0)
    {
        //Permission deny
        return ERROR_PERMISSION_DENIED;
    }
    
    if (lpoHeader->miSize >= this->miSegmentCapacity)
    {
        //Segment is full
        return ERROR_END_OF_SEGMENT;
    }
    
    if (lpoHeader->miWritePage < 0 && (liRetCode = this->AllocatePage(lpoHeader) ) != 0)
    {
        return liRetCode;
    }
    
    ACL_ASSERT(lpoHeader->miWritePage >= 0);

    while(true)
    {
        CPage loPage(&this->moStorage, &this->moBufferPool, lpoHeader->miWritePage);

        if ( (liRetCode = loPage.Open() ) != 0)
        {
            return liRetCode;
        }

        if (loPage.GetSpace() < (apl_int32_t)auLen)
        {
            if ( (liRetCode = this->AllocatePage(lpoHeader) ) != 0)
            {
                return liRetCode;
            }
            
            continue;
        }
        
        if ( (liRetCode = loPage.Write(apcBuffer, auLen) ) != 0)
        {
            return liRetCode;
        }
        
        //apl_errprintf("write segment %"APL_PRIdINT" from page %"APL_PRIdINT"\n", aiSegmentID, lpoHeader->miWritePage);

        break;
    }

    lpoHeader->miSize++;

    return 0;
}

apl_int_t CSegmentQueue::Clear(void)
{
    //Check referenced page
    for (apl_int_t liN = 0; liN < this->miSegmentCount; liN++)
    {
        CSegmentHeader& loSegmentHeader = this->mpoSegmentHeader[liN];

        this->Deallocate0(&loSegmentHeader);
    }

    return 0;
}

apl_int_t CSegmentQueue::Deallocate0( CSegmentHeader* apoHeader )
{
    while(apoHeader->miReadPage != -1)
    {
        apl_int_t liRetCode = this->DeallocatePage(apoHeader);
        ACL_ASSERT(liRetCode == 0);
    }

    apoHeader->miFree = true;
    apoHeader->miReadOffset = 0;
    apoHeader->miSize = 0;
    apoHeader->macQueueName[0] = '\0';
    apoHeader->mi64Timestamp = 0;

    return 0;
}

apl_int_t CSegmentQueue::AllocatePage( CSegmentHeader* apoSegmentHeader )
{
    apl_int_t liRetCode = 0;
    CStorageCheckpoint& loCheckpoint = this->mpoStorageHeader->moCheckpoint;

    if (this->mpoStorageHeader->miFirstFreePage >= 0)
    {
        apl_int_t liNew = this->mpoStorageHeader->miFirstFreePage;
        apl_int32_t liNextFreePage = this->mpiNextPageEntry[liNew];

        if ( (liRetCode = CPage(&this->moStorage, &this->moBufferPool, liNew).Init() ) != 0)
        {
            return liRetCode;
        }
    
        //Start transaction
        loCheckpoint.miType = CHECKPOINT_ALLOCATE;
        loCheckpoint.maiPoint[0] = this->mpoStorageHeader->miFirstFreePage;
        loCheckpoint.maiPoint[1] = this->mpoStorageHeader->miFreePageCount;
        loCheckpoint.maiPoint[2] = apoSegmentHeader->miSegmentID;
        loCheckpoint.maiPoint[3] = apoSegmentHeader->miWritePage;
        loCheckpoint.maiPoint[4] = apoSegmentHeader->miReadPage;

        loCheckpoint.miEnable = true;
    
        if (apoSegmentHeader->miWritePage >= 0)
        {
            this->mpiNextPageEntry[apoSegmentHeader->miWritePage] = liNew;
            apoSegmentHeader->miWritePage = liNew;
        }
        else
        {
            apoSegmentHeader->miWritePage = liNew;
            apoSegmentHeader->miReadPage = liNew;
        }

        this->mpoStorageHeader->miFirstFreePage = liNextFreePage;
        this->mpoStorageHeader->miFreePageCount--;
    
        //Commit
        loCheckpoint.miEnable = false;
        
        return 0;
    }
    else
    {
        return ERROR_NO_ENOUGH_SPACE;
    }
}

apl_int_t CSegmentQueue::DeallocatePage( CSegmentHeader* apoSegmentHeader )
{
    apl_int_t liRetCode = 0;
    CStorageCheckpoint& loCheckpoint = this->mpoStorageHeader->moCheckpoint;
    apl_int32_t liDel = apoSegmentHeader->miReadPage;
    apl_int32_t liNextReadablePage = -1;
    apl_int32_t liNextWritablePage = -1;

    if (apoSegmentHeader->miReadPage != apoSegmentHeader->miWritePage)
    {
        liNextReadablePage = this->mpiNextPageEntry[liDel];
        liNextWritablePage = apoSegmentHeader->miWritePage;
        ACL_ASSERT(liNextReadablePage >= 0);
    }

    //Start transaction
    loCheckpoint.miType = CHECKPOINT_DEALLOCATE;
    loCheckpoint.maiPoint[0] = this->mpoStorageHeader->miFirstFreePage;
    loCheckpoint.maiPoint[1] = this->mpoStorageHeader->miFreePageCount;
    loCheckpoint.maiPoint[2] = apoSegmentHeader->miSegmentID;
    loCheckpoint.maiPoint[3] = liNextReadablePage;
    loCheckpoint.maiPoint[4] = apoSegmentHeader->miReadPage;
    loCheckpoint.maiPoint[5] = liNextWritablePage;
    loCheckpoint.maiPoint[6] = apoSegmentHeader->miWritePage;

    loCheckpoint.miEnable = true;
    
    this->mpiNextPageEntry[liDel] = this->mpoStorageHeader->miFirstFreePage;
    this->mpoStorageHeader->miFirstFreePage = liDel;
    this->mpoStorageHeader->miFreePageCount++;
    apoSegmentHeader->miReadPage = liNextReadablePage;
    apoSegmentHeader->miWritePage = liNextWritablePage;
    
    if ( (liRetCode = CPage(&this->moStorage, &this->moBufferPool, liDel).Free() ) != 0)
    {
        return liRetCode;
    }

    //Commit
    loCheckpoint.miEnable = false;

    return 0;
}

apl_int_t CSegmentQueue::CheckCheckpointAndRecover(void)
{
    apl_int_t liRetCode = 0;
    CStorageCheckpoint& loCheckpoint = this->mpoStorageHeader->moCheckpoint;

    if (!loCheckpoint.miEnable)
    {
        return 0;
    }

    switch(loCheckpoint.miType)
    {
        case CHECKPOINT_ALLOCATE:
        {
            this->mpoStorageHeader->miFirstFreePage = loCheckpoint.maiPoint[0];
            this->mpoStorageHeader->miFreePageCount = loCheckpoint.maiPoint[1];
            CSegmentHeader* lpoSegmentHeader = &this->mpoSegmentHeader[loCheckpoint.maiPoint[2] ];
            lpoSegmentHeader->miWritePage = loCheckpoint.maiPoint[3];
            lpoSegmentHeader->miReadPage = loCheckpoint.maiPoint[4];

            apl_errprintf("Rollback REALLOCATE (FirstFreePage=%"APL_PRIdINT", WritePage=%"APL_PRIdINT", ReadPage=%"APL_PRIdINT")\n",
                this->mpoStorageHeader->miFirstFreePage,
                lpoSegmentHeader->miWritePage,
                lpoSegmentHeader->miReadPage);

            break;
        }

        case CHECKPOINT_DEALLOCATE:
        {
            this->mpoStorageHeader->miFirstFreePage = loCheckpoint.maiPoint[0];
            this->mpoStorageHeader->miFreePageCount = loCheckpoint.maiPoint[1];
            CSegmentHeader* lpoSegmentHeader = &this->mpoSegmentHeader[loCheckpoint.maiPoint[2] ];
            apl_int32_t liNextReadablePage = loCheckpoint.maiPoint[3];
            lpoSegmentHeader->miReadPage = loCheckpoint.maiPoint[4];
            apl_int32_t liNextWritablePage = loCheckpoint.maiPoint[5];
            lpoSegmentHeader->miWritePage = loCheckpoint.maiPoint[6];
            
            apl_errprintf("Rollback DEALLOCATE (FirstFreePage=%"APL_PRIdINT", WritePage=%"APL_PRIdINT", NextReadPage=%"APL_PRIdINT")\n",
                this->mpoStorageHeader->miFirstFreePage,
                lpoSegmentHeader->miReadPage,
                lpoSegmentHeader->miReadPage);
                
            this->mpiNextPageEntry[lpoSegmentHeader->miReadPage] = this->mpoStorageHeader->miFirstFreePage;
            this->mpoStorageHeader->miFirstFreePage = lpoSegmentHeader->miReadPage;
            this->mpoStorageHeader->miFreePageCount++;
            lpoSegmentHeader->miReadPage = liNextReadablePage;
            lpoSegmentHeader->miWritePage = liNextWritablePage;
            
            if ( (liRetCode = CPage(&this->moStorage, &this->moBufferPool, lpoSegmentHeader->miReadPage).Free() ) != 0)
            {
                return liRetCode;
            }

            break;
        }
    }

    return 0;
}

AIBC_GFQ_NAMESPACE_END

