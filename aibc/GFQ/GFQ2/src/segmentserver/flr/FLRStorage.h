#ifndef AIBC_GFQ2_FLRSTORAGE_H
#define AIBC_GFQ2_FLRSTORAGE_H

#include "gfq2/Utility.h"
#include "acl/Synch.h"
#include "acl/File.h"
#include "acl/MemMap.h"
#include "acl/IndexDict.h"

AIBC_GFQ_NAMESPACE_START

/** 
 * @brief  Fixed-Length-Record storage space manager
 */
class CFLRStorage
{
public:
    CFLRStorage(void);

    ~CFLRStorage(void);

    static apl_int_t Create(
        char const* apcPath,
        char const* apcVersion,
        apl_size_t auHeaderSize,
        apl_size_t auSlotCapacity,
        apl_size_t auSegmentCapacity,
        apl_size_t auSegmentCount,
        apl_size_t auFileCount );
    
    apl_int_t Open( char const* apcPath );

    void Close(void);

    apl_int_t Write( apl_size_t auIndex, char const* apcBuffer, apl_size_t auLength );
    
    apl_int_t Read( apl_size_t auIndex, acl::CMemoryBlock& aoData, acl::CTimestamp* apoTimestamp = APL_NULL );

    void* GetHeader(void);
    
    apl_size_t GetHeaderLength(void);

    char const* GetVersion(void);

    char const* GetType(void);

    char const* GetCreateTime(void);

    apl_size_t GetSegmentCount(void);

    apl_size_t GetSegmentCapacity(void);

    apl_size_t GetSlotCapacity(void);
    
    apl_size_t GetFileCount(void);

private:
    apl_size_t muSlotCapacity;

    apl_size_t muSegmentCapacity;

    apl_size_t muSegmentCount;

    apl_size_t muFileCount;
    
    apl_size_t muFileCapacity;

    acl::CFile* mpoFiles;

    acl::CLock* mpoLocks;

    acl::CMemMap moMemMap;

    acl::CIndexDict moMetadata;
};

AIBC_GFQ_NAMESPACE_END

#endif

