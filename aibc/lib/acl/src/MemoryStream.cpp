
#include "acl/MemoryStream.h"
#include "acl/StrAlgo.h"

ACL_NAMESPACE_START

CMemoryStream::CMemoryStream(void)
    : mpoBlock(NULL)
{
}

CMemoryStream::CMemoryStream( CMemoryBlock* apoBlock )
    : mpoBlock(apoBlock)
{
}

CMemoryStream::~CMemoryStream(void)
{
}

void CMemoryStream::SetMemoryBlock( CMemoryBlock* apoBlock )
{
    this->mpoBlock = apoBlock;
}
    
CMemoryBlock* CMemoryStream::GetMemoryBlock(void)
{
    return this->mpoBlock;
}

apl_ssize_t CMemoryStream::Read( void* apvBuffer, apl_size_t auN )
{
    apl_size_t luLength = this->mpoBlock->GetLength();
    
    if (luLength == 0)
    {
        //EOB
        return -1;
    }
    else if (luLength >= auN)
    {
        apl_memcpy(apvBuffer, this->mpoBlock->GetReadPtr(), auN);
        this->mpoBlock->SetReadPtr(auN);
        return auN;
    }
    else
    {
        //Not enougth content
        apl_memcpy(apvBuffer, this->mpoBlock->GetReadPtr(), luLength);
        this->mpoBlock->SetReadPtr(luLength);
        return luLength;
    }
}

apl_ssize_t CMemoryStream::ReadV( apl_iovec_t apoIoVec[], apl_size_t auN )
{
    apl_size_t  luLength = this->mpoBlock->GetLength();
    char const* lpcReadPtr = this->mpoBlock->GetReadPtr();
    apl_size_t  luReadPos = 0;
    
    for (apl_size_t luIdx = 0; luIdx < auN; luIdx++)
    {
        if (luLength == 0)
        {
            //EOB
            break;
        }
        else if (luLength >= apoIoVec[luIdx].iov_len)
        {
            apl_memcpy(apoIoVec[luIdx].iov_base, lpcReadPtr + luReadPos, apoIoVec[luIdx].iov_len);
            luReadPos += apoIoVec[luIdx].iov_len;
            luLength -= apoIoVec[luIdx].iov_len;
        }
        else
        {
            //No enougth content
            apl_memcpy(apoIoVec[luIdx].iov_base, lpcReadPtr + luReadPos, luLength);
            luReadPos += luLength;
            luLength = 0;
            break;
        }
    }
    
    this->mpoBlock->SetReadPtr(luReadPos);
    
    return luReadPos;
}

apl_ssize_t CMemoryStream::ReadLine( char* apcBuffer, apl_size_t auSize )
{
    char*      lpcReadPtr = this->mpoBlock->GetReadPtr();
    apl_size_t luLength   = this->mpoBlock->GetLength();
    apl_size_t luReadSize = 0;
    apl_size_t luEolLen   = 0;

    if (luLength == 0 || auSize-- == 0)//auSize-- reserve end of character '\0'
    {
        return -1;
    }
    
    //Search EoL character \n
    char* lpcResult = (char*)apl_memchr(lpcReadPtr, '\n', luLength);
    if (lpcResult == NULL)
    {
        return -1;
    }
    else
    {
        if (lpcResult != lpcReadPtr && *(lpcResult - 1) == '\r')
        {
            //Matched \r\n
            luEolLen = 2;
            lpcResult--; //really size
        }
        else
        {
            luEolLen = 1;
        }
        
        if (lpcResult - lpcReadPtr > (apl_ssize_t)auSize)
        {
            //Buffer is not enough a line, will be truncated to auSize
            luReadSize = auSize;
            luEolLen = 0;
        }
        else
        {
            luReadSize = lpcResult - lpcReadPtr;
        }
    }
    
    apl_memcpy(apcBuffer, lpcReadPtr, luReadSize);
    this->mpoBlock->SetReadPtr(luReadSize + luEolLen);
    apcBuffer[luReadSize] = '\0'; //append end character \0
    
    return luReadSize;
}

apl_ssize_t CMemoryStream::ReadLine( std::string& aoOutput )
{
    char*      lpcReadPtr = this->mpoBlock->GetReadPtr();
    apl_size_t luLength   = this->mpoBlock->GetLength();
    apl_size_t luReadSize = 0;
    apl_size_t luEolLen   = 0;
    
    if (luLength == 0)
    {
        //End of Block
        return -1;
    }
    
    //Search EoL character \n
    char* lpcResult = (char*)apl_memchr(lpcReadPtr, '\n', luLength);
    if (lpcResult == NULL)
    {
        //Can't found it
        return -1;
    }
    else
    {
        if (lpcResult != lpcReadPtr && *(lpcResult - 1) == '\r')
        {
            //Matched \r\n
            luEolLen = 2;
            lpcResult--; //really size
        }
        else
        {
            luEolLen = 1;
        }
        
        luReadSize = lpcResult - lpcReadPtr;
    }
    
    aoOutput.assign(lpcReadPtr, luReadSize);
    this->mpoBlock->SetReadPtr(luReadSize + luEolLen);
    
    return luReadSize;
}

apl_ssize_t CMemoryStream::ReadString( char* apcBuffer, apl_size_t auSize )
{
    char*      lpcReadPtr = this->mpoBlock->GetReadPtr();
    apl_size_t luLength   = this->mpoBlock->GetLength();

    //End of \0
    char* lpcResult = (char*)apl_memchr(lpcReadPtr, '\0', luLength);
    if (lpcResult == NULL)
    {
        return -1;
    }
    else if (lpcResult - lpcReadPtr + 1 > (apl_ssize_t)auSize)
    {
        return -1;
    }

    apl_memcpy(apcBuffer, lpcReadPtr, lpcResult - lpcReadPtr + 1);
    this->mpoBlock->SetReadPtr(lpcResult - lpcReadPtr + 1);
    
    return lpcResult - lpcReadPtr;
}

apl_ssize_t CMemoryStream::ReadString( std::string& aoOutput )
{
    char*      lpcReadPtr = this->mpoBlock->GetReadPtr();
    apl_size_t luLength   = this->mpoBlock->GetLength();

    //End of \0
    char* lpcResult = (char*)apl_memchr(lpcReadPtr, '\0', luLength);
    if (lpcResult == NULL)
    {
        return -1;
    }

    aoOutput.assign(lpcReadPtr, lpcResult - lpcReadPtr);
    this->mpoBlock->SetReadPtr(lpcResult - lpcReadPtr + 1);
    
    return lpcResult - lpcReadPtr;
}

apl_ssize_t CMemoryStream::ReadUntil( char* apcBuffer, apl_size_t auSize, const char* apcTag )
{
    apl_size_t  luTagLen   = apl_strlen(apcTag);
    char*       lpcReadPtr = this->mpoBlock->GetReadPtr();
    char*       lpcLastPtr = lpcReadPtr + this->mpoBlock->GetLength();
    apl_size_t  luLength   = lpcLastPtr - lpcReadPtr;
    apl_size_t  luReadSize = 0;
    apl_ssize_t liResult   = 0;

    if (auSize-- == 0)//auSize-- reserve end of character '\0'
    {
        return -1;
    }

    if (luTagLen > 2)
    {
        //use kmp-find algoritm
        liResult = stralgo::KMPFind(lpcReadPtr, luLength, apcTag, luTagLen);
    }
    else if (luTagLen > 0)
    {
        char* lpcCurrPtr = lpcReadPtr;
        
        while(true)
        {
            //search first character
            char* lpcNext = (char*)apl_memchr(lpcCurrPtr, apcTag[0], lpcLastPtr - lpcCurrPtr);
            if (lpcNext == NULL)
            {
                liResult = -1;
                break;
            }
            //luTagLen must be 1 or 2
            else if (lpcNext[luTagLen - 1] == apcTag[luTagLen - 1])
            {
                liResult = lpcNext - lpcReadPtr;
                break;
            }
            
            lpcCurrPtr = lpcNext + 1;
        }
    }
    else
    {
        return -1;
    }
    
    if (liResult < 0)
    {
        //Can't search the tag
        return -1;
    }
    else if (liResult > (apl_ssize_t)auSize)
    {
        //Successfull, but buffer is no enough
        luReadSize = auSize;
    }
    else if (liResult + luTagLen > auSize)
    {
        //Successfull, but buffer is no enough, and keep the tag complete
        luReadSize = liResult;
    }
    else
    {
        //Successfull
        luReadSize = liResult + luTagLen;
    }
    
    apl_memcpy(apcBuffer, lpcReadPtr, luReadSize);
    this->mpoBlock->SetReadPtr(luReadSize);
    apcBuffer[luReadSize] = '\0';
    
    return luReadSize;
}

apl_ssize_t CMemoryStream::ReadUntil( std::string& aoOutput, const char* apcTag )
{
    apl_size_t  luTagLen   = apl_strlen(apcTag);
    char*       lpcReadPtr = this->mpoBlock->GetReadPtr();
    char*       lpcLastPtr = lpcReadPtr + this->mpoBlock->GetLength();
    apl_size_t  luLength   = lpcLastPtr - lpcReadPtr;
    apl_size_t  luReadSize = 0;
    apl_ssize_t liResult   = 0;

    if (luTagLen > 2)
    {
        //use kmp-find algoritm
        liResult = stralgo::KMPFind(lpcReadPtr, luLength, apcTag, luTagLen);
    }
    else if (luTagLen > 0)
    {
        char* lpcCurrPtr = lpcReadPtr;
        
        while(true)
        {
            //search first character
            char* lpcNext = (char*)apl_memchr(lpcCurrPtr, apcTag[0], lpcLastPtr - lpcCurrPtr);
            if (lpcNext == NULL)
            {
                liResult = -1;
                break;
            }
            //luTagLen must be 1 or 2
            else if (lpcNext[luTagLen - 1] == apcTag[luTagLen - 1])
            {
                liResult = lpcNext - lpcReadPtr;
                break;
            }
            
            lpcCurrPtr = lpcNext + 1;
        }
    }
    else
    {
        return -1;
    }
    
    if (liResult < 0)
    {
        //Can't search the tag
        return -1;
    }
    else
    {
        //Successfull
        luReadSize = liResult + luTagLen;
    }
    
    aoOutput.assign(lpcReadPtr, luReadSize);
    this->mpoBlock->SetReadPtr(luReadSize);
    
    return luReadSize;
}

apl_ssize_t CMemoryStream::ReadToH( apl_uint16_t& au16Val )
{
    if ( this->Read(au16Val) != sizeof(au16Val) )
    {
        return -1;
    }
    
    au16Val = apl_ntoh16(au16Val);
    
    return sizeof(au16Val);
}

apl_ssize_t CMemoryStream::ReadToH( apl_uint32_t& au32Val )
{
    if ( this->Read(au32Val) != sizeof(au32Val) )
    {
        return -1;
    }
    
    au32Val = apl_ntoh32(au32Val);
    
    return sizeof(au32Val);
}

apl_ssize_t CMemoryStream::ReadToH( apl_uint64_t& au64Val )
{
    if ( this->Read(au64Val) != sizeof(au64Val) )
    {
        return -1;
    }
    
    au64Val = apl_ntoh64(au64Val);
    
    return sizeof(au64Val);
}

apl_ssize_t CMemoryStream::ReadToH( apl_int16_t& ai16Val )
{
    if ( this->Read(ai16Val) != sizeof(ai16Val) )
    {
        return -1;
    }
    
    ai16Val = apl_ntoh16(ai16Val);
    
    return sizeof(ai16Val);
}

apl_ssize_t CMemoryStream::ReadToH( apl_int32_t& ai32Val )
{
    if ( this->Read(ai32Val) != sizeof(ai32Val) )
    {
        return -1;
    }
    
    ai32Val = apl_ntoh32(ai32Val);
    
    return sizeof(ai32Val);
}

apl_ssize_t CMemoryStream::ReadToH( apl_int64_t& ai64Val )
{
    if ( this->Read(ai64Val) != sizeof(ai64Val) )
    {
        return -1;
    }
    
    ai64Val = apl_ntoh64(ai64Val);
    
    return sizeof(ai64Val);
}

apl_ssize_t CMemoryStream::Write( void const* apvBuffer, apl_size_t auN )
{
    apl_size_t luSpace  = 0;
    apl_size_t luWriteSize = 0;
        
    if ( (luSpace = this->mpoBlock->GetSpace() ) == 0)
    {
        return -1;
    }
    
    luWriteSize = luSpace >= auN ? auN : luSpace;
    
    apl_memcpy(this->mpoBlock->GetWritePtr(), apvBuffer, luWriteSize);
    
    this->mpoBlock->SetWritePtr(luWriteSize);
    
    return luWriteSize;
}

apl_ssize_t CMemoryStream::Write( char const* apcPtr )
{
    return this->Write(apcPtr, apl_strlen(apcPtr) + 1 );
}

apl_ssize_t CMemoryStream::WriteV( apl_iovec_t const apoIoVec[], apl_size_t auN )
{
    apl_size_t  luSpace = this->mpoBlock->GetSpace();
    char*       lpcWritePtr = this->mpoBlock->GetWritePtr();
    apl_size_t  luWritePos = 0;
    
    for (apl_size_t luIdx = 0; luIdx < auN; luIdx++)
    {
        if (luSpace == 0)
        {
            //No enough space
            break;
        }
        else if (luSpace >= apoIoVec[luIdx].iov_len)
        {
            apl_memcpy(lpcWritePtr + luWritePos, apoIoVec[luIdx].iov_base, apoIoVec[luIdx].iov_len);
            luWritePos += apoIoVec[luIdx].iov_len;
            luSpace -= apoIoVec[luIdx].iov_len;
        }
        else
        {
            //No enough space
            apl_memcpy(lpcWritePtr + luWritePos, apoIoVec[luIdx].iov_base, luSpace);
            luWritePos += luSpace;
            luSpace = 0;
            break;
        }
    }
    
    this->mpoBlock->SetWritePtr(luWritePos);
    
    return luWritePos;
}

apl_ssize_t CMemoryStream::WriteToN( apl_uint16_t au16Val )
{
    apl_uint16_t lu16Val = apl_hton16(au16Val);
    
    if ( this->Write(lu16Val) != sizeof(lu16Val) )
    {
        return -1;
    }

    return sizeof(lu16Val);
}

apl_ssize_t CMemoryStream::WriteToN( apl_uint32_t au32Val )
{
    apl_uint32_t lu32Val = apl_hton32(au32Val);
    
    if ( this->Write(lu32Val) != sizeof(lu32Val) )
    {
        return -1;
    }

    return sizeof(lu32Val);
}

apl_ssize_t CMemoryStream::WriteToN( apl_uint64_t au64Val )
{
    apl_uint64_t lu64Val = apl_hton64(au64Val);
    
    if ( this->Write(lu64Val) != sizeof(lu64Val) )
    {
        return -1;
    }

    return sizeof(lu64Val);
}

apl_ssize_t CMemoryStream::WriteToN( apl_int16_t ai16Val )
{
    apl_int16_t li16Val = apl_hton16(ai16Val);
    
    if ( this->Write(li16Val) != sizeof(li16Val) )
    {
        return -1;
    }

    return sizeof(li16Val);
}

apl_ssize_t CMemoryStream::WriteToN( apl_int32_t ai32Val )
{
    apl_int32_t li32Val = apl_hton32(ai32Val);
    
    if ( this->Write(li32Val) != sizeof(li32Val) )
    {
        return -1;
    }

    return sizeof(li32Val);
}

apl_ssize_t CMemoryStream::WriteToN( apl_int64_t ai64Val )
{
    apl_int64_t li64Val = apl_hton64(ai64Val);
    
    if ( this->Write(li64Val) != sizeof(li64Val) )
    {
        return -1;
    }

    return sizeof(li64Val);
}

ACL_NAMESPACE_END
