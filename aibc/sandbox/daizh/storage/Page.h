#ifndef AIBC_GFQ_PAGE_H
#define AIBC_GFQ_PAGE_H

#include "acl/File.h"
#include "acl/Synch.h"
#include "acl/stl/string.h"
#include "Storage.h"
#include "BufferPool.h"

AIBC_GFQ_NAMESPACE_START

class CPage
{
public:
    CPage( CStorage* apoStorage, CBufferPool* apoBufferPool, apl_int_t aiPageID );

    apl_int_t Init(void);
    
    apl_int_t Free(void);
    
    apl_int_t Open(void);

    apl_int_t Check(void);

    void Close(void);

    apl_int_t Read( apl_int_t aiStayTime, std::string& aoData );

    apl_int_t Write( char const* apcBuffer, apl_size_t auLen );

    bool IsEmpty(void);

    apl_int_t GetPageID(void);

    apl_int32_t GetPageSize(void);

    apl_int32_t GetReadoffset(void);

    apl_int32_t GetWriteOffset(void);

    apl_int32_t GetSpace(void);

private:
    CStorage* mpoStorage;
    
    CBufferPool* mpoBufferPool;

    apl_int_t miPageID;
    
    apl_int32_t miPageSize;
    
    apl_int32_t miReadOffset;
    
    apl_int32_t miWriteOffset;
};

AIBC_GFQ_NAMESPACE_END

#endif

