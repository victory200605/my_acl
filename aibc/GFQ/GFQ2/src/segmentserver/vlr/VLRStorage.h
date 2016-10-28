#ifndef AIBC_GFQ2_VLRSTORAGE_H
#define AIBC_GFQ2_VLRSTORAGE_H

#include "gfq2/Utility.h"
#include "acl/TimeValue.h"
#include "acl/MemMap.h"
#include "acl/Synch.h"
#include "acl/File.h"
#include "acl/IndexDict.h"
#include "acl/stl/string.h"
#include "acl/stl/vector.h"

AIBC_GFQ_NAMESPACE_START

/** 
 * @brief Create and Open var-length-record storage space on file-system
 */
class CVLRStorage
{
public:
    CVLRStorage(void);

    ~CVLRStorage(void);

    static apl_int_t Create(
        char const* apcPath,
        char const* apcVersion,
        apl_size_t auHeaderSize,
        apl_size_t auSegmentCapacity,
        apl_size_t auSegmentCount,
        apl_size_t auFileCount,
        apl_size_t auPageCount,
        apl_size_t auPageSize );

    apl_int_t Open( char const* apcPath );

    void Close(void);
    
    void* GetHeader(void);

    apl_size_t GetHeaderLength(void);

    apl_ssize_t Read(
        apl_int_t aiPageID,
        apl_int32_t aiOffset,
        void* apvBuffer,
        apl_size_t auLen );
    
    apl_ssize_t Write(
        apl_int_t aiPageID,
        apl_int32_t aiOffset,
        void const* apvBuffer,
        apl_size_t auLen );

    char const* GetVersion(void);

    char const* GetType(void);

    char const* GetCreateTime(void);

    apl_int32_t GetSegmentCount(void);
    
    apl_int32_t GetSegmentCapacity(void);
    
    apl_int32_t GetFileCount(void);
    
    apl_int32_t GetPageCount(void);

    apl_int32_t GetPageSize(void);
    
private:
    acl::CFile* mpoFile;
    
    apl_int32_t miSegmentCount;
    
    apl_int32_t miSegmentCapacity;
    
    apl_int32_t miFileCount;
    
    apl_int32_t miPageCount;
    
    apl_int32_t miPageSize;
    
    acl::CMemMap moHeaderMemMap;
    
    acl::CIndexDict moMetadata;
};

AIBC_GFQ_NAMESPACE_END

#endif

