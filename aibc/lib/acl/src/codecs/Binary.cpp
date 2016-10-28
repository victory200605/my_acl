
#include "acl/codecs/Binary.h"

ACL_NAMESPACE_START

/////////////////////////////////////////////////////////////////////////////////////////////////
//Binary encoder
CBinaryEncoder::CBinaryEncoder(void)
    : moBuffer(NULL, 0, acl::CMemoryBlock::DONT_DELETE)
    , muLength(0)
{
}

CBinaryEncoder::CBinaryEncoder( char* apcBuffer, apl_size_t auSize )
    : moBuffer(apcBuffer, auSize, acl::CMemoryBlock::DONT_DELETE)
    , muLength(0)
{
}

CBinaryEncoder::CBinaryEncoder( CMemoryBlock const& aoMB )
    : moBuffer(aoMB.GetDataBlock()->Duplicate() )
    , muLength(0)
{
}

apl_int_t CBinaryEncoder::Update( void const* apvInput, apl_size_t auInputLen )
{
    apl_size_t luNeedSize   = auInputLen * 8 + 1;//must > 0
    unsigned char* lpcFirst = (unsigned char*)apvInput;
    unsigned char* lpcLast  = lpcFirst + auInputLen;
    char* lpcWritePtr = NULL;
    
    if (this->moBuffer.GetSpace() < luNeedSize)
    {
        this->moBuffer.Resize( (apl_size_t)( (luNeedSize + this->moBuffer.GetLength() ) * 1.5 ) );
    }
    
    lpcWritePtr = this->moBuffer.GetWritePtr();

    for(; lpcFirst != lpcLast; ++lpcFirst)
    {
        *(lpcWritePtr++) = ((*lpcFirst >> 7) & 0x01) + '0';
        *(lpcWritePtr++) = ((*lpcFirst >> 6) & 0x01) + '0';
        *(lpcWritePtr++) = ((*lpcFirst >> 5) & 0x01) + '0';
        *(lpcWritePtr++) = ((*lpcFirst >> 4) & 0x01) + '0';
        *(lpcWritePtr++) = ((*lpcFirst >> 3) & 0x01) + '0';
        *(lpcWritePtr++) = ((*lpcFirst >> 2) & 0x01) + '0';
        *(lpcWritePtr++) = ((*lpcFirst >> 1) & 0x01) + '0';
        *(lpcWritePtr++) = (*lpcFirst & 0x01) + '0';
    }
    
    this->moBuffer.SetWritePtr(lpcWritePtr);
    
    *lpcWritePtr = '\0';
    
    return 0;
}

apl_int_t CBinaryEncoder::Final(void)
{
    this->muLength = this->moBuffer.GetLength();
    this->moBuffer.Reset();
    
    return 0;
}

apl_int_t CBinaryEncoder::Final( void const* apvInput, apl_size_t auInputLen )
{
    if (this->Update(apvInput, auInputLen) != 0)
    {
        return -1;
    }
    
    return this->Final();
}

char const* CBinaryEncoder::GetOutput(void)
{
    return this->moBuffer.GetBase();
}

apl_size_t CBinaryEncoder::GetLength(void)
{
    return this->muLength;
}

void CBinaryEncoder::Reset(void)
{
    this->moBuffer.Reset();
}

//Binary-Decoder
CBinaryDecoder::CBinaryDecoder(void)
    : moBuffer(NULL, 0, acl::CMemoryBlock::DONT_DELETE)
    , muCount(0)
    , muLength(0)
{
}

CBinaryDecoder::CBinaryDecoder( char* apcBuffer, apl_size_t auSize )
    : moBuffer(apcBuffer, auSize, acl::CMemoryBlock::DONT_DELETE)
    , muCount(0)
    , muLength(0)
{
}

CBinaryDecoder::CBinaryDecoder( CMemoryBlock const& aoMB )
    : moBuffer(aoMB.GetDataBlock()->Duplicate() )
    , muCount(0)
    , muLength(0)
{
}

apl_int_t CBinaryDecoder::Update( char const* apcInput )
{
    return this->Update(apcInput, apl_strlen(apcInput) );
}

apl_int_t CBinaryDecoder::Update( char const* apcInput, apl_size_t auInputLen )
{
    apl_size_t  luNeedSize = auInputLen / 8 + 1;//must > 0
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
        if (*lpcFirst == '0' || *lpcFirst == '1')
        {
            this->macGroup[this->muCount++] = *lpcFirst - '0';
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
        
        if (this->muCount == 8)
        {
            *lpcWritePtr = 0;
            *lpcWritePtr |= this->macGroup[0] << 7;
            *lpcWritePtr |= this->macGroup[1] << 6;
            *lpcWritePtr |= this->macGroup[2] << 5;
            *lpcWritePtr |= this->macGroup[3] << 4;
            *lpcWritePtr |= this->macGroup[4] << 3;
            *lpcWritePtr |= this->macGroup[5] << 2;
            *lpcWritePtr |= this->macGroup[6] << 1;
            *lpcWritePtr++ |= this->macGroup[7];
            this->muCount = 0;
        }
    }
    
    this->moBuffer.SetWritePtr((char*)lpcWritePtr);
    
    *lpcWritePtr = '\0';
    
    return 0;
}
    
apl_int_t CBinaryDecoder::Final(void)
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
    
apl_int_t CBinaryDecoder::Final( char const* apcInput )
{
    return this->Final(apcInput, apl_strlen(apcInput) );
}
    
apl_int_t CBinaryDecoder::Final( char const* apcInput, apl_size_t auInputLen )
{
    if (this->Update(apcInput, auInputLen) != 0)
    {
        return -1;
    }
    
    return this->Final();
}
    
char const* CBinaryDecoder::GetOutput(void)
{
    return this->moBuffer.GetBase();
}

apl_size_t CBinaryDecoder::GetLength(void)
{
    return this->muLength;
}

void CBinaryDecoder::Reset(void)
{
    this->moBuffer.Reset();
    this->muCount = 0;
}

ACL_NAMESPACE_END
