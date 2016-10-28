#ifndef AIBC_GFQ2_STORAGE_H
#define AIBC_GFQ2_STORAGE_H

#include "Utility.h"
#include "Storage.h"
#include "acl/MemMap.h"
#include "acl/Synch.h"
#include "acl/File.h"
#include "acl/IndexDict.h"
#include "acl/stl/string.h"
#include "acl/stl/vector.h"
#include "BufferPool.h"

AIBC_GFQ_NAMESPACE_START

/** 
 * @brief Storage metadata type define
 */
enum EStorageMetadata
{
    METADATA_VERSION          = 0,
    METADATA_SEGMENT_COUNT    = 1,
    METADATA_SEGMENT_CAPACITY = 2,
    METADATA_FILE_COUNT       = 3,
    METADATA_PAGE_COUNT       = 4,
    METADATA_TOTAL_PAGE_COUNT = 5,
    METADATA_PAGE_SIZE        = 6,
    METADATA_HEADER_SIZE      = 7
};

/** 
 * @brief Storage metadata dict class define
 */
struct CStorageMetadata : public acl::CIndexDict
{
    void Dump(void);
};

/** 
 * @brief Create and Open storage space on file-system
 */
class CStorage
{
public:
    CStorage(void);

    ~CStorage(void);

    static apl_int_t Create( char const* apcPath, CStorageMetadata& aoMetadata );

    apl_int_t Open( char const* apcPath );

    void Close(void);

    CStorageMetadata* GetMetadata(void);

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

    apl_int32_t GetPageSize(void);

private:
    acl::CFile* mpoFile;

    apl_int32_t miFileCount;
    
    apl_int32_t miPageCount;
    
    apl_int32_t miPageSize;
    
    acl::CMemMap moHeaderMemMap;
    
    CStorageMetadata moMetadata;
};

AIBC_GFQ_NAMESPACE_END

#endif

