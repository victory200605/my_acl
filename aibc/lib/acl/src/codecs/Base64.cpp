
#include "acl/codecs/Base64.h"

ACL_NAMESPACE_START

////////////////////////////////////////////////////////////////////////////////////////////////////
namespace {
#undef _Encoder
#undef _Padding
#undef _Decoder

////////////////////////////////////////////////////////////////////////////
// Symbols which form the Base64 alphabet (Defined as per RFC 2045)
unsigned char const _Encoder[] =
{ // 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
    'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P', // 0
    'Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f', // 1
    'g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v', // 2
    'w','x','y','z','0','1','2','3','4','5','6','7','8','9','+','/', // 3
};

unsigned char const _Padding = '=';
unsigned char const _Invalid = 0xFF;

unsigned char const _Decoder[] =
{
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xE0,0xF0,0xFF,0xFF,0xF1,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xE0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0x3E,0xFF,0xF2,0xFF,0x3F,
	0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,
	0x3C,0x3D,0xFF,0xFF,0xFF,0x00,0xFF,0xFF,
	0xFF,0x00,0x01,0x02,0x03,0x04,0x05,0x06,
	0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,
	0x0F,0x10,0x11,0x12,0x13,0x14,0x15,0x16,
	0x17,0x18,0x19,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,0x20,
	0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,
	0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,0x30,
	0x31,0x32,0x33,0xFF,0xFF,0xFF,0xFF,0xFF,
};

}

#define __BASE64_ENCODE_3_4(output, input, len)                                 \
    for(apl_size_t liN = 3; liN <= len; liN += 3, output += 4, input += 3)      \
    {                                                                           \
        output[0] = _Encoder[(input[0] >> 2) & 0x3F];                           \
        output[1] = _Encoder[((input[0] << 4) & 0x30) | ((input[1] >> 4) & 0x0F)];  \
        output[2] = _Encoder[((input[1] << 2) & 0x3C) | ((input[2] >> 6) & 0x03)];  \
        output[3] = _Encoder[input[2] & 0x3F];                                  \
    }

CBase64Encoder::CBase64Encoder(void)
    : muCount(0)
    , moBuffer(NULL, 0, CMemoryBlock::DONT_DELETE)
    , muLength(0)
{
}

CBase64Encoder::CBase64Encoder( char* apcBuffer, apl_size_t auSize )
    : muCount(0)
    , moBuffer(apcBuffer, auSize, CMemoryBlock::DONT_DELETE)
    , muLength(0)
{
}

CBase64Encoder::CBase64Encoder( CMemoryBlock const& aoMB )
    : muCount(0)
    , moBuffer(aoMB.GetDataBlock()->Duplicate() )
    , muLength(0)
{
}

//apl_int_t CBase64Encoder::Update( char const* apcInput )
//{
//    return this->Update(apcInput, apl_strlen(apcInput) );
//}

apl_int_t CBase64Encoder::Update( void const* apvInput, apl_size_t auInputLen )
{
    unsigned char* lpcIn      = (unsigned char*)apvInput;
    unsigned char* lpcOut     = NULL;
    apl_size_t     luNeedSize = ((auInputLen + this->muCount) / 3) * 4 + 5;
    
    if (this->moBuffer.GetSpace() < luNeedSize)
    {
        this->moBuffer.Resize( (apl_size_t)( (luNeedSize + this->moBuffer.GetLength() ) * 1.5 ) );
    }
    
    lpcOut = (unsigned char*)this->moBuffer.GetWritePtr();

    if (this->muCount + auInputLen >= 3 )
    {
        apl_memcpy(&this->macEncodeGroup[this->muCount], lpcIn, 3 - this->muCount);
        
        lpcIn = this->macEncodeGroup;
        __BASE64_ENCODE_3_4(lpcOut, lpcIn, 3);
        
        lpcIn      = (unsigned char*)apvInput + 3 - this->muCount;
        auInputLen = auInputLen - 3 + this->muCount;
        __BASE64_ENCODE_3_4(lpcOut, lpcIn, auInputLen);

        this->moBuffer.SetWritePtr((char*)lpcOut);
        this->muCount = 0;
    }

    apl_memcpy(&this->macEncodeGroup[this->muCount], lpcIn, auInputLen % 3);
    this->muCount += auInputLen % 3;
    
    return 0;
}

apl_int_t CBase64Encoder::Final(void)
{
    unsigned char* lpcIn  = this->macEncodeGroup;
    unsigned char* lpcOut = (unsigned char*)this->moBuffer.GetWritePtr();

    if (this->muCount > 0)
    {
        apl_memset(&this->macEncodeGroup[this->muCount], '\0', 3 - this->muCount);
        
        __BASE64_ENCODE_3_4(lpcOut, lpcIn, 3);
        
        this->moBuffer.SetWritePtr(4);
        
        for (apl_size_t liN = this->muCount; liN < 3; liN++)
        {
            *(lpcOut - (3 - liN) ) = _Padding;
        }
    }
    
    *lpcOut = '\0';
    
    this->muLength = this->moBuffer.GetLength();

    this->Reset();
    
    return 0;
}

apl_int_t CBase64Encoder::Final( void const* apvInput, apl_size_t auInputLen )
{
    if ( this->Update(apvInput, auInputLen) != 0 )
    {
        return -1;
    }

    return this->Final();
}

void CBase64Encoder::Reset(void)
{
    this->moBuffer.Reset();

    this->muCount = 0;
}

char const* CBase64Encoder::GetOutput(void)
{
    return this->moBuffer.GetBase();
}
 
apl_size_t CBase64Encoder::GetLength(void)
{
    return this->muLength;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
CBase64Decoder::CBase64Decoder(void)
    : muCount(0)
    , moBuffer(NULL, 0, CMemoryBlock::DONT_DELETE)
    , muLength(0)
    , mbIsFinished(false)
{
}

CBase64Decoder::CBase64Decoder( char* apcBuffer, apl_size_t auSize )
    : muCount(0)
    , moBuffer(apcBuffer, auSize, CMemoryBlock::DONT_DELETE)
    , muLength(0)
    , mbIsFinished(false)
{
}

CBase64Decoder::CBase64Decoder( CMemoryBlock const& aoMB )
    : muCount(0)
    , moBuffer(aoMB.GetDataBlock()->Duplicate() )
    , muLength(0)
    , mbIsFinished(false)
{
}

apl_int_t CBase64Decoder::Update( char const* apcInput )
{
    return this->Update(apcInput, apl_strlen(apcInput) );
}

apl_int_t CBase64Decoder::Update( char const* apcInput, apl_size_t auInputLen )
{
    unsigned char* lpcIn      = (unsigned char*)apcInput;
    unsigned char* lpcOut     = NULL;
    apl_int_t      liRetCode  = 0;
    apl_int_t      liPadChrs  = 0;
    apl_size_t     luNeedSize = ((this->muCount + auInputLen) / 4) * 3 + 1;
    
    if (this->moBuffer.GetSpace() < luNeedSize)
    {
        this->moBuffer.Resize( (apl_size_t)( (luNeedSize + this->moBuffer.GetLength() ) * 1.5 ) );
    }
    
    lpcOut = (unsigned char*)this->moBuffer.GetWritePtr();
    
    while(true)
    {
        apl_int_t liResult = this->Next4(&lpcIn, this->macDecodeGroup, this->muCount, liPadChrs);
        if (liResult == 4)
        {
            if (!this->mbIsFinished)
            {
                lpcOut[0] = ((_Decoder[this->macDecodeGroup[0]] << 2) & 0xFC) 
                    | ((_Decoder[this->macDecodeGroup[1]] >> 4) & 0x03);
                lpcOut[1] = ((_Decoder[this->macDecodeGroup[1]] << 4) & 0xF0) 
                    | ((_Decoder[this->macDecodeGroup[2]] >> 2) & 0x0F);
                lpcOut[2] = ((_Decoder[this->macDecodeGroup[2]] << 6) & 0xC0) 
                    | ((_Decoder[this->macDecodeGroup[3]] & 0x3F));
            
                lpcOut += (3 - liPadChrs);
                this->mbIsFinished = liPadChrs > 0 ? true : false;
                this->muCount = 0;
            }
            else
            {
                liRetCode = -1;
                break;
            }
        }
        else if (liResult >= 0)
        {
            this->muCount = liResult;
            break;
        }
        else
        {
            liRetCode = -1;
            break;
        }
    }
    
    if (liRetCode == 0)
    {
        this->moBuffer.SetWritePtr((char*)lpcOut);
    }
    else
    {
        this->moBuffer.Reset();
    }
    
    return liRetCode;
}

apl_int_t CBase64Decoder::Final(void)
{
    if (this->muCount > 0)
    {
        this->Reset();
        return -1;
    }
    else
    {
        *(this->moBuffer.GetWritePtr() ) = '\0';
        this->muLength = this->moBuffer.GetLength();
        this->Reset();
        return 0;
    }
}

apl_int_t CBase64Decoder::Final( char const* apcInput )
{
    return this->Final(apcInput, apl_strlen(apcInput) );
}

apl_int_t CBase64Decoder::Final( char const* apcInput, apl_size_t auInputLen )
{
    if (this->Update(apcInput, auInputLen) != 0)
    {
        return -1;
    }

    return this->Final();
}

apl_int_t CBase64Decoder::Next4(
    unsigned char** appcInput, unsigned char apcOutput[], apl_size_t auCurrLen, apl_int_t& aiPadChrs )
{
    unsigned char*& lpcInput = *appcInput;
    apl_int_t   liPadHit = 0;
    apl_int_t   liResult = auCurrLen;
    
    if (liResult >= 2 && apcOutput[liResult-1] == _Padding)
    {
        liPadHit++;
    }
    
    while(*lpcInput != '\0' && liResult < 4)
    {
        while(*lpcInput != '\0' && liResult < 4)
        {
            if ( apl_isspace(*lpcInput) )
            {
                lpcInput++;
                continue;
            }
            else if (_Invalid == _Decoder[(apl_int_t)*lpcInput])
            {
                return -1;
            }
            else if (_Padding == *lpcInput)
            {
                if (liResult < 2)
                {
                    return -1;
                }
                
                apcOutput[liResult++] = *lpcInput++;
                liPadHit++;
                break;
            }
            else
            {
                apcOutput[liResult++] = *lpcInput++;
            }
        }
    }
    
    aiPadChrs = liPadHit;

    return (liPadHit == 1 && apcOutput[liResult - 1] != _Padding) ? -1 : liResult;
}

char const* CBase64Decoder::GetOutput(void)
{
    return this->moBuffer.GetBase();
}
 
apl_size_t CBase64Decoder::GetLength(void)
{
    return this->muLength;
}
    
void CBase64Decoder::Reset(void)
{
    this->moBuffer.Reset();

    this->muCount = 0;
    
    this->mbIsFinished = false;
}

#undef __BASE64_ENCODE_3_4

ACL_NAMESPACE_END
