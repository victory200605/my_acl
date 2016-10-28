
#include "FLRSegmentQueue.h"
#include "acl/Synch.h"
#include "gfq2/GFQMessage.h"
#include "aaf/LogService.h"

AIBC_GFQ_NAMESPACE_START

CFLRSegmentQueue::CFLRSegmentQueue( char const* apcPath )
    : moPath(apcPath)
    , muSegmentCount(0)
    , muSegmentCapacity(0)
    , mpoSegmentHeader(APL_NULL)
    , mpoLock(APL_NULL)
    , mu64WriteTimes(0)
    , mu64PrevWriteTimes(0)
    , mu64ReadTimes(0)
    , mu64PrevReadTimes(0)
{
}

CFLRSegmentQueue::~CFLRSegmentQueue(void)
{
}

apl_int_t CFLRSegmentQueue::Create(
    char const* apcPath, 
    apl_size_t auSlotCapacity,
    apl_size_t auSegmentSize,
    apl_size_t auSegmentCount,
    apl_size_t auFileCount )
{
    apl_int_t liRetCode = 0;
    apl_size_t luHeaderSize = sizeof(CSegmentHeader) * auSegmentCount * auFileCount;
    
    if ( (liRetCode = CFLRStorage::Create(
        apcPath,
        "v1.0.0",
        luHeaderSize,
        auSlotCapacity, 
        auSegmentSize, 
        auSegmentCount,
        auFileCount ) ) != 0)
    {
        return liRetCode;
    }
    
    {//Initialize
        CFLRStorage loStorage;
        
        liRetCode = loStorage.Open(apcPath);
        ACL_ASSERT(liRetCode == 0);
        
        apl_size_t luSegmentCount = loStorage.GetSegmentCount();
        CSegmentHeader* lpoSegmentHeader = static_cast<CSegmentHeader*>(loStorage.GetHeader() );
        apl_memset(lpoSegmentHeader, 0, sizeof(CSegmentHeader) * luSegmentCount);
    }
    
    return 0;
}

apl_int_t CFLRSegmentQueue::Open(void)
{
    apl_int_t liRetCode = 0;

    if ( (liRetCode = this->moStorage.Open(this->moPath.c_str() ) ) != 0)
    {
        return liRetCode;
    }

    this->muSegmentCount = this->moStorage.GetSegmentCount();
    this->muSegmentCapacity = this->moStorage.GetSegmentCapacity();
    this->mpoSegmentHeader = static_cast<CSegmentHeader*>(this->moStorage.GetHeader() );

    ACL_NEW_N_ASSERT(this->mpoLock, acl::CLock, this->muSegmentCount);

    return 0;
}

apl_int_t CFLRSegmentQueue::Check(void)
{
    return 0;
}

apl_int_t CFLRSegmentQueue::Recover(void)
{
    return 0;
}

apl_int_t CFLRSegmentQueue::Clear( char const* apcQueueName )
{
    for (apl_size_t luN = 0; luN < this->muSegmentCount; luN++)
    {
        CSegmentHeader& loSegmentHeader = this->mpoSegmentHeader[luN];

        if (apcQueueName != APL_NULL && apl_strcmp(loSegmentHeader.macQueueName, apcQueueName) == 0)
        {
            apl_memset(&loSegmentHeader, 0, sizeof(CSegmentHeader) );
        }
    }

    return 0;
}

apl_int_t CFLRSegmentQueue::ClearAll(void)
{
    apl_memset(this->mpoSegmentHeader, 0, sizeof(CSegmentHeader) * this->muSegmentCount);

    return 0;
}

void CFLRSegmentQueue::Close(void)
{
    this->moStorage.Close();
    ACL_DELETE_N(this->mpoLock);
}

apl_int_t CFLRSegmentQueue::Allocate( apl_size_t auSegmentID, char const* apcQueueName, apl_int64_t ai64Timestamp )
{
    if (auSegmentID >= this->muSegmentCount)
    {
        return ERROR_SEGMENT_UNEXISTED;
    }

    acl::TSmartLock<acl::CLock> loGuard(this->mpoLock[auSegmentID]);
    
    if (this->mpoSegmentHeader[auSegmentID].mi64Timestamp <= 0)
    {
        this->mpoSegmentHeader[auSegmentID].mi64Timestamp = ai64Timestamp;
        apl_strncpy(
            this->mpoSegmentHeader[auSegmentID].macQueueName,
            apcQueueName,
            sizeof(this->mpoSegmentHeader[auSegmentID].macQueueName) );
        this->mpoSegmentHeader[auSegmentID].miReadPos = 0;
        this->mpoSegmentHeader[auSegmentID].miWritePos = 0;

        return 0;
    }
    else
    {
        return ERROR_PERMISSION_DENIED;
    }

}

apl_int_t CFLRSegmentQueue::Deallocate( apl_size_t auSegmentID )
{
    if (auSegmentID >= this->muSegmentCount)
    {
        return ERROR_SEGMENT_UNEXISTED;
    }

    acl::TSmartLock<acl::CLock> loGuard(this->mpoLock[auSegmentID]);
    
    if (this->mpoSegmentHeader[auSegmentID].miReadPos == this->mpoSegmentHeader[auSegmentID].miWritePos)
    {
        this->mpoSegmentHeader[auSegmentID].mi64Timestamp = 0;
        this->mpoSegmentHeader[auSegmentID].macQueueName[0] = '\0';
        this->mpoSegmentHeader[auSegmentID].miReadPos = 0;
        this->mpoSegmentHeader[auSegmentID].miWritePos = 0;

        return 0;
    }
    else
    {
        return ERROR_PERMISSION_DENIED;
    }
}

apl_int_t CFLRSegmentQueue::Push( apl_size_t auSegmentID, char const* apcQueueName, char const* apcData, apl_size_t auLen )
{
    if (auSegmentID >= this->muSegmentCount)
    {
        return ERROR_SEGMENT_UNEXISTED;
    }

    if (auLen == 0)
    {
        return ERROR_INVALID_DATA;
    }

    acl::TSmartLock<acl::CLock> loGuard(this->mpoLock[auSegmentID]);

    this->mu64WriteTimes++;
    
    apl_int_t liRetCode = 0;
    apl_int32_t& liWritePos  = this->mpoSegmentHeader[auSegmentID].miWritePos;
    
    if (apl_strcmp(this->mpoSegmentHeader[auSegmentID].macQueueName, apcQueueName) != 0)
    {
        return ERROR_PERMISSION_DENIED;
    }

    if ((apl_size_t)liWritePos < this->muSegmentCapacity)
    {
        if ( (liRetCode = this->moStorage.Write(
            auSegmentID * this->muSegmentCapacity + liWritePos,
            apcData,
            auLen ) ) != 0)
        {
            return liRetCode;
        }

        ++liWritePos;
    }
    else
    {
        return ERROR_END_OF_SEGMENT;
    }
    
    return 0;

}

apl_int_t CFLRSegmentQueue::Pop(
    apl_size_t auSegmentID, char const* apcQueueName, apl_int_t aiStayTime, acl::CMemoryBlock& aoData )
{
    if (auSegmentID >= this->muSegmentCount)
    {
        return ERROR_SEGMENT_UNEXISTED;
    }
    
    acl::TSmartLock<acl::CLock> loGuard(this->mpoLock[auSegmentID]);

    this->mu64ReadTimes++;

    apl_int_t liRetCode = 0;
    apl_int32_t& liReadPos = this->mpoSegmentHeader[auSegmentID].miReadPos;
    apl_int32_t& liWritePos = this->mpoSegmentHeader[auSegmentID].miWritePos;
    acl::CTimestamp loTimestamp;

    if (apl_strcmp(this->mpoSegmentHeader[auSegmentID].macQueueName, apcQueueName) != 0)
    {
        return ERROR_PERMISSION_DENIED;
    }

    while(true)
    {
        if (liReadPos < liWritePos)
        {
            if ( (liRetCode = this->moStorage.Read(
                auSegmentID * this->muSegmentCapacity + liReadPos,
                aoData,
                &loTimestamp ) ) == 0)
            {
                if (aiStayTime > 0)
                {
                    acl::CTimestamp loCurrTime(acl::CTimestamp::PRC_SEC);

                    if (loCurrTime.Sec() - loTimestamp.Sec() < aiStayTime)
                    {
    
                        return ERROR_TIMEDOUT_UNEXISTED;;
                    }
                }
                
                ++liReadPos;
                    
                return 0;
            }
            else if (liRetCode == ERROR_INVALID_DATA)
            {
                ++liReadPos;

                continue;
            }
            
            return liRetCode;
        }
        else if ((apl_size_t)liWritePos < this->muSegmentCapacity)
        {
            return ERROR_EMPTY_OF_SEGMENT;
        }
        else
        {
            return ERROR_END_OF_SEGMENT;
        }
    }
}

apl_int_t CFLRSegmentQueue::GetSegmentInfo( apl_size_t auSegmentID, CSegmentInfo& aoSegmentInfo )
{
    if (auSegmentID >= this->muSegmentCount)
    {
        return ERROR_SEGMENT_UNEXISTED;
    }

    acl::TSmartLock<acl::CLock> loGuard(this->mpoLock[auSegmentID]);

    aoSegmentInfo.SetSegmentID(auSegmentID);
    aoSegmentInfo.SetTimestamp(this->mpoSegmentHeader[auSegmentID].mi64Timestamp);
    aoSegmentInfo.SetQueueName(this->mpoSegmentHeader[auSegmentID].macQueueName);
    aoSegmentInfo.SetSize(this->mpoSegmentHeader[auSegmentID].miWritePos - this->mpoSegmentHeader[auSegmentID].miReadPos);
    aoSegmentInfo.SetCapacity(this->muSegmentCapacity);

    return 0;
}
    
apl_int_t CFLRSegmentQueue::GetAllSegmentInfo( std::vector<CSegmentInfo>& aoSegmentInfos )
{
    aoSegmentInfos.resize(this->muSegmentCount);

    for (apl_size_t luN = 0; luN < this->muSegmentCount; luN++)
    {
        this->GetSegmentInfo(luN, aoSegmentInfos[luN]);
    }

    return 0;
}

apl_size_t CFLRSegmentQueue::GetSegmentCount(void)
{
    return this->muSegmentCount;
}

apl_size_t CFLRSegmentQueue::GetSegmentCapacity(void)
{
    return this->muSegmentCapacity;
}

void CFLRSegmentQueue::Dump(void)
{
    AAF_LOG_INFO("%s", "-------------------------------------------------------------------------------------");
    AAF_LOG_INFO("|-%-40s :%-40s|", "version", this->moStorage.GetVersion() );
    AAF_LOG_INFO("|-%-40s :%-40s|", "type", this->moStorage.GetType() );
    AAF_LOG_INFO("|-%-40s :%-40"APL_PRIuINT"|", "slot-capacity", this->moStorage.GetSlotCapacity() );
    AAF_LOG_INFO("|-%-40s :%-40"APL_PRIuINT"|", "segment-count", this->moStorage.GetSegmentCount() );
    AAF_LOG_INFO("|-%-40s :%-40"APL_PRIuINT"|", "segment-capacity", this->moStorage.GetSegmentCapacity() );
    AAF_LOG_INFO("|-%-40s :%-40"APL_PRIuINT"|", "file-count", this->moStorage.GetFileCount() );
    AAF_LOG_INFO("|-%-40s :%-40s|", "create-time", this->moStorage.GetCreateTime() );
    AAF_LOG_INFO("%s", "-------------------------------------------------------------------------------------");
}

void CFLRSegmentQueue::Dump( acl::CIndexDict& aoMessage )
{
    apl_uint64_t lu64WriteTimes(this->mu64WriteTimes);
    apl_uint64_t lu64ReadTimes(this->mu64ReadTimes);

    aoMessage.Set(GFQ_TAG_NODE_WRITE_TIMES, lu64WriteTimes - this->mu64PrevWriteTimes);
    aoMessage.Set(GFQ_TAG_NODE_READ_TIMES, lu64ReadTimes - this->mu64PrevReadTimes);
}

AIBC_GFQ_NAMESPACE_END

