#ifndef AIBC_GFQ_PAGE_H
#define AIBC_GFQ_PAGE_H

#include "acl/File.h"
#include "acl/Synch.h"
#include "acl/stl/string.h"
#include "VLRStorage.h"
#include "VLRBufferPool.h"

AIBC_GFQ_NAMESPACE_START

class CVLRPage
{
    //Page header define
    //------------------------------------------------------------------
    //| read_offset (4) | write_offset(4) | ......
    //------------------------------------------------------------------
    struct CPageHeader
    {
        //Read offset value offset in page
        static const apl_int_t ROV_OFFSET = 0;
        
        //Write offset value offset in page
        static const apl_int_t WOV_OFFSET = 4;

        //Page read offset
        apl_int32_t miReadOffset;

        //Page write offset
        apl_int32_t miWriteOffset;
        
        CPageHeader(apl_int32_t aiReadOffset = 0, apl_int32_t aiWriteOffset = 0);
    };

    //Record header define
    //------------------------------------------------------------------
    //| timestamp(4) | length (4) | data | ......
    //------------------------------------------------------------------
    struct CRecordHeader
    {
        //Record insert timestamp
        apl_int32_t miTimestamp;

        //Record length
        apl_int32_t miLength;

        CRecordHeader(apl_int32_t aiTimestamp = 0, apl_int32_t aiLength = 0);
    };

public:
    CVLRPage( CVLRStorage* apoStorage, CVLRBufferPool* apoBufferPool, apl_int_t aiPageID );

    apl_int_t Init(void);
    
    apl_int_t Free(void);
    
    apl_int_t Open(void);

    apl_ssize_t Check(void);

    void Close(void);

    apl_int_t Read( apl_int_t aiStayTime, acl::CMemoryBlock& aoData );

    apl_int_t Write( char const* apcBuffer, apl_size_t auLen );

    bool IsEmpty(void);

    apl_int_t GetPageID(void);

    apl_int32_t GetPageSize(void);

    apl_int32_t GetReadoffset(void);

    apl_int32_t GetWriteOffset(void);

    apl_int32_t GetSpace(void);

private:
    CVLRStorage* mpoStorage;
    
    CVLRBufferPool* mpoBufferPool;

    apl_int_t miPageID;
    
    apl_int32_t miPageSize;
 
    CPageHeader moHeader;
};

AIBC_GFQ_NAMESPACE_END

#endif

