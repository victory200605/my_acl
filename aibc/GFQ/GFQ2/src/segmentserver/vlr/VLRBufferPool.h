#ifndef AIBC_GFQ2_BUFFER_POOL_H
#define AIBC_GFQ2_BUFFER_POOL_H

#include "gfq2/Utility.h"
#include "acl/Ring.h"
#include "acl/MemMap.h"
#include "acl/IndexDict.h"

AIBC_GFQ_NAMESPACE_START

class CVLRStorage;

/** 
 * @brief Storage page buffer pool, use to cache reading or writing page
 */
class CVLRBufferPool
{
    enum EMetadata
    {
        METADATA_TYPE            = 0,
        METADATA_BUCKET_NUM      = 1,
        METADATA_PAGE_SIZE       = 2,
        METADATA_HEADER_SIZE     = 3,
        METADATA_CHECKPOINT_SIZE = 4,
        METADATA_BUFFER_SIZE     = 5
    };

    /** 
     * @brief Buffer pool checkpoint for writing operation atomicity
     */
    struct CCheckpoint
    {
        char miEnable;

        apl_int32_t miPageID;
        
        apl_int32_t miOffset;
        
        apl_int32_t miLength;

        char macBuffer[1];
    };

    /** 
     * @brief Cache page link entry node define
     */
    struct CPageEntry
    {
        CPageEntry(void);

        apl_int_t miPageID;
        
        apl_int_t miIndex;

        char* mpcBuffer;

        ACL_RING_ENTRY(CPageEntry) moEntry;
    };

public:
    enum EWriteOption
    {
        OPT_DEFAULT = 0,

        OPT_ATOMIC = 1
    };

public:
    CVLRBufferPool(void);

    ~CVLRBufferPool(void);

    apl_int_t Initialize(
        CVLRStorage* apoStorage,
        char const* apcPath,
        char const* apcType,
        apl_int_t aiBucketNum, 
        apl_int_t aiPageSize, 
        apl_int_t aiBufferSize 
    );

    void Close(void);

    apl_ssize_t Read(
        apl_int_t aiPageID,
        apl_int32_t aiOffset,
        void* apvBuffer,
        apl_size_t auLen );
    
    apl_ssize_t Write(
        apl_int_t aiPageID,
        apl_int32_t aiOffset,
        void const* apvBuffer,
        apl_size_t auLen,
        apl_int_t aiOpt = OPT_DEFAULT);

    apl_int_t Preallocate( apl_int_t aiPageID, void const* apvBuffer, apl_size_t auLen );

    apl_int_t Hot( apl_int_t aiPageID );

    apl_int_t Erase( apl_int_t aiPageID );
    
    apl_int_t Sync( apl_int_t aiPageID );
    
    apl_int_t Sync(void);

    void Dump( acl::CIndexDict& aoMessage );

protected:
    apl_int_t ReflushBufferPool( char const* apcPath );
    
    apl_int_t Create(
        char const* apcPath,
        char const* apcType,
        apl_int_t aiBucketNum, 
        apl_int_t aiPageSize, 
        apl_int_t aiBufferSize );

    apl_int_t Open(
        acl::CIndexDict const& aoMetadata,
        void* apvBuffer,
        apl_int_t aiSize );
    
    apl_int_t Get(
        apl_int_t aiPageID,
        void const* apcInitBuffer,
        apl_size_t auInitLen,
        apl_uint64_t* apu64MissedTimes,
        char** apcPage );

    apl_int_t ClearExpirePage(void);

    apl_int_t CheckCheckpointAndRecover(void);

private:
    CVLRStorage* mpoStorage;
    
    acl::CMemMap moMemMap;

    //Buffer pool storage struct
    apl_int32_t* mpiIndex;

    CCheckpoint* mpoCheckpoint;

    char* mpcPage;
    
    //Buffer pool description variable
    std::string moPath;

    std::string moType;
    
    apl_int_t miBucketNum;
    
    apl_int32_t miPageCount;

    //Buffer pool page link
    CPageEntry* mpoEntry;

    ACL_RING_HEAD(PageHeaderType, CPageEntry);
    
    PageHeaderType moFree;

    apl_int_t miFreePageCount;
    
    PageHeaderType moUsed;

    apl_int_t miUsedPageCount;
    
    //STAT.
    struct CStatInfo
    {
        apl_uint64_t mu64ReadTimes;

        apl_uint64_t mu64ReadMissedTimes;

        apl_uint64_t mu64WriteTimes;

        apl_uint64_t mu64WriteMissedTimes;
        
        apl_uint64_t mu64SyncTimes;
    };

    CStatInfo moStatInfo;

    CStatInfo moPrevStatInfo;
};

AIBC_GFQ_NAMESPACE_END

#endif

