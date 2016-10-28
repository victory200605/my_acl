
#include "acl/codecs/Hex.h"

ACL_NAMESPACE_START

/////////////////////////////////////////////////////////////////////////////////////////////////
//Hex encoder
CHexEncoder::CHexEncoder( EOption aeOpt )
    : moBuffer(NULL, 0, acl::CMemoryBlock::DONT_DELETE)
    , muLength(0)
    , meOption(aeOpt)
{
}

CHexEncoder::CHexEncoder( char* apcBuffer, apl_size_t auSize, EOption aeOpt )
    : moBuffer(apcBuffer, auSize, acl::CMemoryBlock::DONT_DELETE)
    , muLength(0)
    , meOption(aeOpt)
{
}

CHexEncoder::CHexEncoder( CMemoryBlock const& aoMB, EOption aeOpt )
    : moBuffer(aoMB.GetDataBlock()->Duplicate() )
    , muLength(0)
    , meOption(aeOpt)
{
}

apl_int_t CHexEncoder::Update( void const* apvInput, apl_size_t auInputLen )
{
    apl_size_t luNeedSize = auInputLen * 2 + 1;//inputlen must > 0
    unsigned char* lpcFirst = (unsigned char*)apvInput;
    unsigned char* lpcLast  = lpcFirst + auInputLen;
    char*          lpcWritePtr = NULL;
    
    if (this->moBuffer.GetSpace() < luNeedSize)
    {
        this->moBuffer.Resize( (apl_size_t)( (luNeedSize + this->moBuffer.GetLength() ) * 1.5 ) );
    }
    
    lpcWritePtr = this->moBuffer.GetWritePtr();

    for(; lpcFirst != lpcLast; ++lpcFirst)
    {
        unsigned char lcChLow  = *lpcFirst & 0x0F;
    	unsigned char lcChHigh = *lpcFirst >> 4;
        
        if (this->meOption == OPT_UPPER)
        {
    	    lcChLow  += lcChLow  < 10 ? '0' : ('A' - 10);
            lcChHigh += lcChHigh < 10 ? '0' : ('A' - 10);
        }
        else
        {
            lcChLow  += lcChLow  < 10 ? '0' : ('a' - 10);
            lcChHigh += lcChHigh < 10 ? '0' : ('a' - 10);
        }
        
        *(lpcWritePtr++) = lcChHigh;
        *(lpcWritePtr++) = lcChLow;
    }
    
    this->moBuffer.SetWritePtr(lpcWritePtr);
    
    *lpcWritePtr = '\0';
    
    return 0;
}

apl_int_t CHexEncoder::Final(void)
{
    this->muLength = this->moBuffer.GetLength();
    this->moBuffer.Reset();
    
    return 0;
}

apl_int_t CHexEncoder::Final( void const* apvInput, apl_size_t auInputLen )
{
    if (this->Update(apvInput, auInputLen) != 0)
    {
        return -1;
    }
    
    return this->Final();
}

char const* CHexEncoder::GetOutput(void)
{
    return this->moBuffer.GetBase();
}

apl_size_t CHexEncoder::GetLength(void)
{
    return this->muLength;
}

void CHexEncoder::Reset(void)
{
    this->moBuffer.Reset();
}

//Hex-Decoder
CHexDecoder::CHexDecoder(void)
    : moBuffer(NULL, 0, acl::CMemoryBlock::DONT_DELETE)
    , muCount(0)
    , muLength(0)
{
}

CHexDecoder::CHexDecoder( char* apcBuffer, apl_size_t auSize )
    : moBuffer(apcBuffer, auSize, acl::CMemoryBlock::DONT_DELETE)
    , muCount(0)
    , muLength(0)
{
}

CHexDecoder::CHexDecoder( CMemoryBlock const& aoMB )
    : moBuffer(aoMB.GetDataBlock()->Duplicate() )
    , muCount(0)
    , muLength(0)
{
}

apl_int_t CHexDecoder::Update( char const* apcInput )
{
    return this->Update(apcInput, apl_strlen(apcInput) );
}

apl_int_t CHexDecoder::Update( char const* apcInput, apl_size_t auInputLen )
{
    apl_size_t  luNeedSize = auInputLen / 2 + 1;//inputlen must > 0
    char const* lpcFirst   = apcInput;
    char const* lpcLast    = lpcFirst + auInputLen;
    unsigned char* lpcWritePtr = NULL;
    
    if (this->moBuffer.GetSpace() < luNeedSize)
    {
        this->moBuffer.Resize( (apl_size_t)( (luNeedSize + this->moBuffer.GetLength() ) * 1.5 ) );
    }
    
    lpcWritePtr = (unsigned char*)this->moBuffer.GetWritePtr();
    
    for (; lpcFirst != lpcLast; ++lpcFirst)
    {
        if (*lpcFirst >= '0' && *lpcFirst <= '9')
        {
            this->macGroup[this->muCount++] = *lpcFirst - '0';
        }
        else if (*lpcFirst >= 'A' && *lpcFirst <= 'F')
        {
            this->macGroup[this->muCount++] = *lpcFirst - 'A' + 10;
        }
        else if (*lpcFirst >= 'a' && *lpcFirst <= 'f')
        {
            this->macGroup[this->muCount++] = *lpcFirst - 'a' + 10;
        }
        else if (
            *lpcFirst == ' ' 
            || *lpcFirst == '\t' 
            || *lpcFirst == '\r' 
            || *lpcFirst == '\n' )
        {
            continue;
        }
        else
        {
            return -1;
        }
        
        if (this->muCount == 2)
        {
            *lpcWritePtr = (this->macGroup[0] & 0x0F) << 4;
            *lpcWritePtr++ |= this->macGroup[1] & 0x0F;
            this->muCount = 0;
        }
    }
    
    this->moBuffer.SetWritePtr((char*)lpcWritePtr);
    
    *lpcWritePtr = '\0';
    
    return 0;
}
    
apl_int_t CHexDecoder::Final(void)
{
    if (this->muCount > 0)
    {
        this->Reset();
        return -1;
    }
    else
    {
        this->muLength = this->moBuffer.GetLength();
        this->Reset();
        return 0;
    }
}
    
apl_int_t CHexDecoder::Final( char const* apcInput )
{
    return this->Final(apcInput, apl_strlen(apcInput) );
}
    
apl_int_t CHexDecoder::Final( char const* apcInput, apl_size_t auInputLen )
{
    if (this->Update(apcInput, auInputLen) != 0)
    {
        return -1;
    }
    
    return this->Final();
}
    
char const* CHexDecoder::GetOutput(void)
{
    return this->moBuffer.GetBase();
}

apl_size_t CHexDecoder::GetLength(void)
{
    return this->muLength;
}

void CHexDecoder::Reset(void)
{
    this->moBuffer.Reset();
    this->muCount = 0;
}

ACL_NAMESPACE_END
