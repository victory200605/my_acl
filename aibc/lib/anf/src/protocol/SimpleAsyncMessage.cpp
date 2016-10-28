
#include "anf/protocol/SimpleAsyncMessage.h"

ANF_NAMESPACE_START

CSimpleAsyncMessage::CSimpleAsyncMessage(void)
    : miMessageID(0)
    , miResult(0)
{
}
    
CSimpleAsyncMessage::~CSimpleAsyncMessage(void)
{
}

apl_ssize_t CSimpleAsyncMessage::Encode( acl::CMemoryBlock& aoBuffer )
{
    char*       lpcWritePtr  = aoBuffer.GetWritePtr();
    apl_int32_t liBodyLength = this->moBody.GetLength();
    apl_int32_t liMessageID  = apl_hton32(this->miMessageID);
    apl_int32_t liResult     = apl_hton32(this->miResult);
    apl_int32_t liLength     = apl_hton32(liBodyLength);
    
    if (aoBuffer.GetSpace() < sizeof(apl_int32_t) * 3 + liBodyLength )
    {
        return sizeof(apl_int32_t) * 3 + liBodyLength;
    }
    
    apl_memcpy(lpcWritePtr + 0,  &liMessageID, 4);
    apl_memcpy(lpcWritePtr + 4,  &liResult, 4);
    apl_memcpy(lpcWritePtr + 8,  &liLength, 4);
    if (liBodyLength > 0)
    {
        apl_memcpy(lpcWritePtr + 12, this->moBody.GetReadPtr(), liBodyLength);
    }
    
    aoBuffer.SetWritePtr(sizeof(apl_int32_t) * 3 + liBodyLength);
    
    return 0;
}
    
apl_ssize_t CSimpleAsyncMessage::Decode( acl::CMemoryBlock& aoBuffer )
{
    char*       lpcReadPtr = aoBuffer.GetReadPtr();
    apl_size_t  luLength = aoBuffer.GetLength();
    apl_int32_t liBodyLength = 0; 

    if (luLength < sizeof(apl_int32_t) * 3)
    {
        return sizeof(apl_int32_t) * 3;
    }
    
    apl_memcpy(&this->miMessageID, lpcReadPtr + 0,  4);
    apl_memcpy(&this->miResult, lpcReadPtr + 4,  4);
    apl_memcpy(&liBodyLength, lpcReadPtr + 8,  4);
    
    this->miMessageID = apl_ntoh32(this->miMessageID);
    this->miResult    = apl_ntoh32(this->miResult);
    liBodyLength      = apl_ntoh32(liBodyLength);
    
    if (liBodyLength > 0)
    {
        if (luLength < sizeof(apl_int32_t) * 3 + liBodyLength)
        {
            return sizeof(apl_int32_t) * 3 + liBodyLength;
        }
        
        this->moBody.Reset();
        this->moBody.Resize(liBodyLength + 1);
        char* lpcWritePtr = this->moBody.GetWritePtr();

        apl_memcpy(lpcWritePtr, lpcReadPtr + 12, liBodyLength);
        *(lpcWritePtr + liBodyLength) = '\0';
        this->moBody.SetWritePtr(liBodyLength);
    }
    
    aoBuffer.SetReadPtr(sizeof(apl_int32_t) * 3 + liBodyLength);
    
    return 0;
}

void CSimpleAsyncMessage::SetMessageID( apl_size_t auMessageID )
{
    this->miMessageID = auMessageID;
}
    
apl_size_t CSimpleAsyncMessage::GetMessageID(void) const
{
    return this->miMessageID;
}
    
acl::CMemoryBlock& CSimpleAsyncMessage::GetBody(void)
{
    return this->moBody;
}
    
ANF_NAMESPACE_END
