
#include "aaf/LogService.h"
#include "VLRSegmentQueue.h"
#include "VLRPage.h"
#include "acl/Dir.h"
#include "acl/stl/set.h"
#include "gfq2/GFQMessage.h"

AIBC_GFQ_NAMESPACE_START

//Checkpoint type define
enum
{
    CHECKPOINT_ALLOCATE     = 1,
    CHECKPOINT_DEALLOCATE   = 2
};

//Storage status define
enum
{
    STORAGE_STATUS_NORMAL    = 0,
    STORAGE_STATUS_EXCEPTION = 1,
};

////////////////////////////////////////////////////////////////////////////////////////////
class CProgressPointer
{
public:
    CProgressPointer( char const* apcMessage, apl_size_t auLen, apl_size_t auMax )
        : moMessage(apcMessage)
        , muLen(auLen)
        , muMax(auMax)
        , muProgress(0)
    {
    }

    void Start(void)
    {
        apl_errprintf("%s 1%%", this->moMessage.c_str() );
    }

    void Progress( apl_size_t auN )
    {
        //Check progress print
        apl_size_t luCurr = (apl_int_t)((((float)auN)/this->muMax) * this->muLen);

        for (; this->muProgress <= luCurr; this->muProgress++)
        {
            if (this->muProgress == this->muLen / 2)
            {
                apl_errprintf("50%%");
            }
            else
            {
                apl_errprintf(".");
            }
        }
    }

    void End(void)
    {
        apl_errprintf("100%% Completed.\n");
    }

private:
    std::string moMessage;
    apl_size_t muLen;
    apl_size_t muMax;
    apl_size_t muProgress;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CVLRSegmentQueue::CVLRSegmentQueue( char const* apcPath, char const* apcBufferPoolType, apl_int_t aiBufferPoolSize )
    : moPath(apcPath)
    , moBufferPoolType(apcBufferPoolType)
    , miBufferPoolSize(aiBufferPoolSize)
    , mpoStorageHeader(APL_NULL)
    , mpoSegmentHeader(APL_NULL)
    , mpiNextPageEntry(APL_NULL)
    , miSegmentCount(0)
    , miSegmentCapacity(0)
    , miPageSize(0)
    , miTotalPageCount(0)
    , mu64WriteTimes(0)
    , mu64PrevWriteTimes(0)
    , mu64ReadTimes(0)
    , mu64PrevReadTimes(0)
{
}

CVLRSegmentQueue::~CVLRSegmentQueue(void)
{
}

apl_int_t CVLRSegmentQueue::Create(
    char const* apcPath,
    apl_size_t auSegmentCapacity,
    apl_size_t auSegmentCount,
    apl_size_t auFileCount,
    apl_size_t auPageCount,
    apl_size_t auPageSize )
{
    //Segment Queue Storage Header
    //-----------------------------
    //|Storage Header             |
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
    CVLRStorage loStorage;
    apl_int_t liRetCode = 0;

    apl_size_t luTotalPageCount = auFileCount*auPageCount;
    apl_size_t luHeaderSize = 
        sizeof(CStorageHeader) + sizeof(CSegmentHeader) * auSegmentCount + 4*luTotalPageCount;
    
    if ( (liRetCode = CVLRStorage::Create(
        apcPath, 
        "v1.0.0",
        luHeaderSize,
        auSegmentCapacity,
        auSegmentCount,
        auFileCount,
        auPageCount,
        auPageSize ) ) != 0)
    {
        return liRetCode;
    }

    if ( (liRetCode = loStorage.Open(apcPath) ) != 0)
    {
        return liRetCode;
    }

    //Initialize header
    {
        apl_size_t luN = 0;
        CStorageHeader* lpoStorage = static_cast<CStorageHeader*>(loStorage.GetHeader() );

        ACL_ASSERT(luHeaderSize == loStorage.GetHeaderLength() );

        lpoStorage->miFreePageCount = luTotalPageCount;
        lpoStorage->miFirstFreePage = 0;
        
        //Initialize Segment Header
        for (luN = 0; luN < auSegmentCount; luN++)
        {
            CSegmentHeader& loSegmentHeader = lpoStorage->maoSegmentHeader[luN];
            
            loSegmentHeader.miSegmentID = luN;

            CVLRSegmentQueue::InitSegmentHeader(&loSegmentHeader);
        }
        
        //Initialize Page Allocate Link
        apl_int32_t* lpiEntry = (apl_int32_t*)&lpoStorage->maoSegmentHeader[auSegmentCount];
        for (luN = 0; luN < luTotalPageCount - 1; luN++)
        {
            lpiEntry[luN] = luN + 1;
        }
        lpiEntry[luN] = -1;

        //Initialize Checkpoint
        lpoStorage->moCheckpoint.miEnable = false;
        lpoStorage->moCheckpoint.miType = 0;
        lpoStorage->miStatus = STORAGE_STATUS_NORMAL;
    }

    return 0;
}

apl_int_t CVLRSegmentQueue::Open(void)
{
    apl_int_t liRetCode = 0;
    
    if ( (liRetCode = this->moStorage.Open(this->moPath.c_str() ) ) != 0)
    {
        return liRetCode;
    }

    this->mpoStorageHeader = static_cast<CStorageHeader*>(this->moStorage.GetHeader() );
    this->mpoSegmentHeader = this->mpoStorageHeader->maoSegmentHeader; 
    this->miSegmentCount = this->moStorage.GetSegmentCount();
    this->miSegmentCapacity = this->moStorage.GetSegmentCapacity();
    this->miPageSize = this->moStorage.GetPageSize();
    this->miTotalPageCount = this->moStorage.GetPageCount() * this->moStorage.GetFileCount();
    this->mpiNextPageEntry = (apl_int32_t*)(&this->mpoSegmentHeader[this->miSegmentCount]);

    if ( (liRetCode = this->moBufferPool.Initialize(
        &this->moStorage,
        this->moPath.c_str(), 
        this->moBufferPoolType.c_str(),
        this->miTotalPageCount,
        this->miPageSize,
        this->miBufferPoolSize) ) != 0)
    {
        return liRetCode;
    }

    if ( (liRetCode = this->CheckCheckpointAndRecover() ) != 0)
    {
        return liRetCode;
    }

    return 0;
}

apl_int_t CVLRSegmentQueue::Check(void)
{
    apl_int_t liRetCode = 0;
    apl_int_t liFreePageCount = 0;
    apl_int_t liUsedPageCount = 0;
    CProgressPointer loProgress("VLR Checking", 50, this->miSegmentCount);
    bool lbIsException = false;
    std::vector<std::string> loMessage;
    char lacBuffer[1024];

    if (this->mpoStorageHeader->miStatus == STORAGE_STATUS_EXCEPTION)
    {
        return ERROR_INVALID_DATA;
    }

    loProgress.Start();
                
    //Check referenced page
    for (apl_int_t liN = 0; liN < this->miSegmentCount; liN++)
    {
        CSegmentHeader& loSegmentHeader = this->mpoSegmentHeader[liN];
        apl_int_t liReadPage = loSegmentHeader.miReadPage;
        apl_int_t liWritePage = loSegmentHeader.miWritePage;
        apl_int_t liCount = 0;

        loProgress.Progress(liN);

        if (liReadPage == -1 && liWritePage == -1)
        {
            continue;
        }
        else if (liReadPage >= this->miTotalPageCount || liWritePage >= this->miTotalPageCount)
        {
            lbIsException = true;
            continue;
        }

        while(true)
        {
            if ( (liRetCode = CVLRPage(&this->moStorage, &this->moBufferPool, liReadPage).Check() ) != 0)
            {
                apl_snprintf(lacBuffer, sizeof(lacBuffer),
                    "check fail, bad page (SegmentID=%"APL_PRIdINT",PageID=%"APL_PRIdINT")\n",
                    liN, 
                    liReadPage);
                loMessage.push_back(lacBuffer);

                lbIsException = true;

                break;
            }

            liCount++;
            liUsedPageCount++;

            if (liReadPage == liWritePage)
            {
                break;
            }

            if (liCount > this->miTotalPageCount)
            {
                apl_snprintf(lacBuffer, sizeof(lacBuffer),
                    "check fail, segment page link maybe deadloop "
                    "(SegmentID=%"APL_PRIdINT",PageCount=%"APL_PRIdINT")\n",
                    liN, 
                    liCount);
                loMessage.push_back(lacBuffer);

                lbIsException = true;

                break;
            }

            //Check next page
            if (this->mpiNextPageEntry[liReadPage] >= this->miTotalPageCount)
            {
                lbIsException = true;
                break;
            }
            else
            {
                liReadPage = this->mpiNextPageEntry[liReadPage];
            }
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

            if (liCount > this->miTotalPageCount || liFreePage > this->miTotalPageCount)
            {
                apl_snprintf(lacBuffer, sizeof(lacBuffer),
                    "check fail, free page link maybe deadloop "
                    "(PageCount=%"APL_PRIdINT",FreePage=%"APL_PRIdINT",TotalPageCount=%"APL_PRIdINT")\n",
                    liCount,
                    liFreePage,
                    (apl_int_t)this->miTotalPageCount );
                loMessage.push_back(lacBuffer);

                lbIsException = true;
                break;
            }

            //Check next page
            liFreePage = this->mpiNextPageEntry[liFreePage];
        }
    }

    if (lbIsException || liUsedPageCount + liFreePageCount != this->miTotalPageCount)
    {
        apl_snprintf(lacBuffer, sizeof(lacBuffer),
            "check fail, page lose (Used=%"APL_PRIdINT", Free=%"APL_PRIdINT", Total=%"APL_PRIdINT")\n",
            liUsedPageCount,
            liFreePageCount,
            this->miTotalPageCount);
        loMessage.push_back(lacBuffer);

        this->mpoStorageHeader->miStatus = STORAGE_STATUS_EXCEPTION;
        lbIsException = true;
    }
    else
    {
        this->mpoStorageHeader->miStatus = STORAGE_STATUS_NORMAL;
    }

    loProgress.End();

    for (apl_size_t luN = 0; luN < loMessage.size(); luN++)
    {
        apl_errprintf("VLR Checking INFO: %s", lacBuffer);
    }

    return lbIsException ? ERROR_INVALID_DATA : 0;
}

apl_int_t CVLRSegmentQueue::Recover(void)
{
    apl_int_t liRetCode = 0;
    std::set<apl_int_t> loReachable;
    CProgressPointer loProgress("VLR Recovering", 50, this->miSegmentCount);

    if (this->mpoStorageHeader->miStatus == STORAGE_STATUS_NORMAL)
    {
        apl_errprintf("VLR storage status is normal\n");
        return 0;
    }
    
    loProgress.Start();
                
    //Recover referenced page
    for (apl_int_t liN = 0; liN < this->miSegmentCount; liN++)
    {
        std::vector<apl_int_t> loPages;
        CSegmentHeader& loSegmentHeader = this->mpoSegmentHeader[liN];
        apl_int_t liReadPage = loSegmentHeader.miReadPage;
        apl_int_t liWritePage = loSegmentHeader.miWritePage;

        loProgress.Progress(liN);

        if (liReadPage == -1 && liWritePage == -1)
        {
            continue;
        }
        else if (liReadPage >= this->miTotalPageCount || liWritePage >= this->miTotalPageCount)
        {
            this->InitSegmentHeader(&loSegmentHeader);
            continue;
        }

        while(true)
        {
            if ( (liRetCode = CVLRPage(&this->moStorage, &this->moBufferPool, liReadPage).Check() ) != 0)
            {
                //ERROR:Bad page
                CVLRPage(&this->moStorage, &this->moBufferPool, liReadPage).Init();
            }

            loPages.push_back(liReadPage);

            if (liReadPage == liWritePage)
            {
                break;
            }

            if (loPages.size() > (apl_size_t)this->miTotalPageCount)
            {
                //FATAL ERROR:Bad page link and release all
                this->InitSegmentHeader(&loSegmentHeader);
                loPages.clear();
                break;
            }

            //Check next page
            if (this->mpiNextPageEntry[liReadPage] >= this->miTotalPageCount)
            {
                //Invalid page and cuted
                loSegmentHeader.miWritePage = liReadPage;
                break;
            }
            else
            {
                liReadPage = this->mpiNextPageEntry[liReadPage];
            }
        }

        //Save all reachable page
        for (apl_size_t luN = 0; luN < loPages.size(); luN++)
        {
            loReachable.insert(loPages[luN]);
        }
    }

    //Recover free page
    {
        this->mpoStorageHeader->miFirstFreePage = -1;
        for (apl_int_t liN = 0; liN < this->miTotalPageCount; liN++)
        {
            if (loReachable.find(liN) != loReachable.end() )
            {
                continue;
            }

            this->mpiNextPageEntry[liN] = this->mpoStorageHeader->miFirstFreePage;
            this->mpoStorageHeader->miFirstFreePage = liN;
        }
    }

    loProgress.End();

    return 0;
}

void CVLRSegmentQueue::Close(void)
{
    this->mpoStorageHeader->miStatus = STORAGE_STATUS_NORMAL;

    this->moBufferPool.Close();
    this->moStorage.Close();
}

apl_int_t CVLRSegmentQueue::Allocate( apl_size_t auSegmentID, char const* apcQueueName, apl_int64_t ai64Timestamp )
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

apl_int_t CVLRSegmentQueue::Deallocate( apl_size_t auSegmentID )
{
    if (auSegmentID > (apl_size_t)this->miSegmentCount)
    {
        return ERROR_SEGMENT_UNEXISTED;
    }
    
    acl::TSmartLock<acl::CLock> loGuard(this->moLock);
    
    CSegmentHeader* lpoHeader = &this->mpoSegmentHeader[auSegmentID];
    if (lpoHeader->miSize - lpoHeader->miReadOffset <= 0)
    {
        return this->Deallocate0(lpoHeader);
    }
    else
    {
        return ERROR_PERMISSION_DENIED;
    }
}

apl_int_t CVLRSegmentQueue::Pop(
    apl_size_t auSegmentID,
    char const* apcQueueName,
    apl_int_t aiStayTime, 
    acl::CMemoryBlock& aoData )
{
    apl_int_t liRetCode = 0;
    
    if (auSegmentID > (apl_size_t)this->miSegmentCount)
    {
        return ERROR_SEGMENT_UNEXISTED;
    }
    
    acl::TSmartLock<acl::CLock> loGuard(this->moLock);
    
    //Stat.
    this->mu64ReadTimes++;

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

        CVLRPage loPage(&this->moStorage, &this->moBufferPool, lpoHeader->miReadPage);

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
            //ACL_ASSERT(liRetCode != ERROR_INVALID_DATA);
            break;
        }

        lpoHeader->miReadOffset++;

        break;
    }

    //if (lpoHeader->miReadOffset >= this->miSegmentCapacity)
    //{
    //    apl_int_t r = this->Deallocate0(lpoHeader);
    //    ACL_ASSERT(r == 0);
    //}

    return liRetCode;
}

apl_int_t CVLRSegmentQueue::Push( apl_size_t auSegmentID, char const* apcQueueName, char const* apcBuffer, apl_size_t auLen )
{
    apl_int_t liRetCode = 0;

    if (auSegmentID > (apl_size_t)this->miSegmentCount)
    {
        return ERROR_SEGMENT_UNEXISTED;
    }

    if (auLen == 0)
    {
        return ERROR_INVALID_DATA;
    }

    acl::TSmartLock<acl::CLock> loGuard(this->moLock);

    //Stat.
    this->mu64WriteTimes++;

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
        CVLRPage loPage(&this->moStorage, &this->moBufferPool, lpoHeader->miWritePage);

        if ( (liRetCode = loPage.Open() ) != 0)
        {
            return liRetCode;
        }

        liRetCode = loPage.Write(apcBuffer, auLen);
        if (liRetCode == 0)
        {
            break;
        }
        else if (liRetCode == ERROR_NO_ENOUGH_SPACE)
        {
            if ( (liRetCode = this->AllocatePage(lpoHeader) ) != 0)
            {
                return liRetCode;
            }

            continue;
        }
        else
        {
            return liRetCode;
        }
    }

    lpoHeader->miSize++;

    return 0;
}

apl_int_t CVLRSegmentQueue::Clear( char const* apcQueueName )
{
    //Check referenced page
    for (apl_int_t liN = 0; liN < this->miSegmentCount; liN++)
    {
        CSegmentHeader& loSegmentHeader = this->mpoSegmentHeader[liN];

        if (apcQueueName != APL_NULL && apl_strcmp(loSegmentHeader.macQueueName, apcQueueName) == 0)
        {
            this->Deallocate0(&loSegmentHeader);
        }

    }

    return 0;
}

apl_int_t CVLRSegmentQueue::ClearAll(void)
{
    //Check referenced page
    for (apl_int_t liN = 0; liN < this->miSegmentCount; liN++)
    {
        CSegmentHeader& loSegmentHeader = this->mpoSegmentHeader[liN];

        this->Deallocate0(&loSegmentHeader);
    }

    return 0;
}

apl_int_t CVLRSegmentQueue::GetSegmentInfo( apl_size_t auSegmentID, CSegmentInfo& aoSegmentInfo )
{
    if (auSegmentID > (apl_size_t)this->miSegmentCount)
    {
        return ERROR_SEGMENT_UNEXISTED;
    }

    acl::TSmartLock<acl::CLock> loGuard(this->moLock);

    CSegmentHeader* lpoHeader = &this->mpoSegmentHeader[auSegmentID];
    
    aoSegmentInfo.SetSegmentID(auSegmentID);
    aoSegmentInfo.SetTimestamp(lpoHeader->mi64Timestamp);
    aoSegmentInfo.SetQueueName(lpoHeader->macQueueName);
    aoSegmentInfo.SetSize(lpoHeader->miSize - lpoHeader->miReadOffset);
    aoSegmentInfo.SetCapacity(this->miSegmentCapacity);

    return 0;
}

apl_int_t CVLRSegmentQueue::GetAllSegmentInfo( std::vector<CSegmentInfo>& aoSegmentInfos )
{
    apl_int_t liRetCode = 0;

    aoSegmentInfos.resize(this->miSegmentCount);

    for (apl_int_t liN = 0; liN < this->miSegmentCount; liN++)
    {
        if ( (liRetCode = this->GetSegmentInfo(liN, aoSegmentInfos[liN]) ) != 0)
        {
            return liRetCode;
        }
    }

    return 0;
}

apl_size_t CVLRSegmentQueue::GetSegmentCount(void)
{
    return this->miSegmentCount;
}

apl_size_t CVLRSegmentQueue::GetSegmentCapacity(void)
{
    return this->miSegmentCapacity;
}

void CVLRSegmentQueue::Dump(void)
{
    AAF_LOG_INFO("%s", "-------------------------------------------------------------------------------------");
    AAF_LOG_INFO("|-%-40s :%-40s|", "version", this->moStorage.GetVersion() );
    AAF_LOG_INFO("|-%-40s :%-40s|", "type", this->moStorage.GetType() );
    AAF_LOG_INFO("|-%-40s :%-40"APL_PRIuINT"|", "segment-count", this->moStorage.GetSegmentCount() );
    AAF_LOG_INFO("|-%-40s :%-40"APL_PRIuINT"|", "segment-capacity", this->moStorage.GetSegmentCapacity() );
    AAF_LOG_INFO("|-%-40s :%-40"APL_PRIuINT"|", "file-count", this->moStorage.GetFileCount() );
    AAF_LOG_INFO("|-%-40s :%-40"APL_PRIuINT"|", "page-count", this->moStorage.GetPageCount() );
    AAF_LOG_INFO("|-%-40s :%-40"APL_PRIuINT"|", "total-page-count", this->moStorage.GetPageCount() * this->moStorage.GetFileCount() );
    AAF_LOG_INFO("|-%-40s :%-40"APL_PRIuINT"|", "page-size", this->moStorage.GetPageSize() );
    AAF_LOG_INFO("|-%-40s :%-40s|", "buffer-pool-type", this->moBufferPoolType.c_str() );
    AAF_LOG_INFO("|-%-40s :%-40"APL_PRIdINT"|", "buffer-pool-size", this->miBufferPoolSize);
    AAF_LOG_INFO("|-%-40s :%-40s|", "create-time", this->moStorage.GetCreateTime() );
    AAF_LOG_INFO("%s", "-------------------------------------------------------------------------------------");
}

void CVLRSegmentQueue::Dump( acl::CIndexDict& aoMessage )
{
    apl_uint64_t lu64WriteTimes(this->mu64WriteTimes);
    apl_uint64_t lu64ReadTimes(this->mu64ReadTimes);

    aoMessage.Set(GFQ_TAG_NODE_WRITE_TIMES, lu64WriteTimes - this->mu64PrevWriteTimes);
    aoMessage.Set(GFQ_TAG_NODE_READ_TIMES, lu64ReadTimes - this->mu64PrevReadTimes);
    aoMessage.Set(GFQ_TAG_NODE_FREE_PAGE_COUNT, this->mpoStorageHeader->miFreePageCount);
    aoMessage.Set(GFQ_TAG_NODE_USED_PAGE_COUNT, this->miTotalPageCount - this->mpoStorageHeader->miFreePageCount);

    this->moBufferPool.Dump(aoMessage);

    this->mu64PrevWriteTimes = lu64WriteTimes;
    this->mu64PrevReadTimes = lu64ReadTimes;
}

void CVLRSegmentQueue::InitSegmentHeader( CSegmentHeader* apoHeader )
{
    apoHeader->miFree = true;
    apoHeader->miReadPage = -1;
    apoHeader->miWritePage = -1;
    apoHeader->mi64Timestamp = 0;
    apoHeader->miReadOffset = 0;
    apoHeader->miSize = 0;
    apoHeader->miStatus = STORAGE_STATUS_NORMAL;
}

apl_int_t CVLRSegmentQueue::Deallocate0( CSegmentHeader* apoHeader )
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

apl_int_t CVLRSegmentQueue::AllocatePage( CSegmentHeader* apoSegmentHeader )
{
    apl_int_t liRetCode = 0;
    CCheckpoint& loCheckpoint = this->mpoStorageHeader->moCheckpoint;

    if (this->mpoStorageHeader->miFirstFreePage >= 0)
    {
        apl_int_t liNew = this->mpoStorageHeader->miFirstFreePage;
        apl_int32_t liNextFreePage = this->mpiNextPageEntry[liNew];

        if ( (liRetCode = CVLRPage(&this->moStorage, &this->moBufferPool, liNew).Init() ) != 0)
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

apl_int_t CVLRSegmentQueue::DeallocatePage( CSegmentHeader* apoSegmentHeader )
{
    apl_int_t liRetCode = 0;
    CCheckpoint& loCheckpoint = this->mpoStorageHeader->moCheckpoint;
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
    
    if ( (liRetCode = CVLRPage(&this->moStorage, &this->moBufferPool, liDel).Free() ) != 0)
    {
        return liRetCode;
    }

    //Commit
    loCheckpoint.miEnable = false;

    return 0;
}

apl_int_t CVLRSegmentQueue::CheckCheckpointAndRecover(void)
{
    apl_int_t liRetCode = 0;
    CCheckpoint& loCheckpoint = this->mpoStorageHeader->moCheckpoint;

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
            
            if ( (liRetCode = CVLRPage(&this->moStorage, &this->moBufferPool, lpoSegmentHeader->miReadPage).Free() ) != 0)
            {
                return liRetCode;
            }

            break;
        }
    }

    return 0;
}

AIBC_GFQ_NAMESPACE_END

