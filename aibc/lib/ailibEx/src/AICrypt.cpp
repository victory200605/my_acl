#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AICrypt.h"

///start namespace
AIBC_NAMESPACE_START

////////////////////////////////////////////////////////////////////////////////////
#define ROTATE_LEFT(x, n) (((x)<<(n)) | ((x)>>(32-(n))))
#define F(X, Y, Z) (((X)&(Y)) | ((~X)&(Z)))
#define G(X, Y, Z) (((X)&(Z)) | ((Y)&(~Z)))
#define I(X, Y, Z) ((Y) ^ ((X)|(~Z)))
#define H(X, Y, Z) ((X)^(Y)^(Z))

#define FF(a, b, c, d, M, s, t) {\
    (a) += F(b, c, d) + (M) + (uint32_t)(t); \
    (a) = ROTATE_LEFT(a, s); \
    (a) += (b); \
    }
#define GG(a, b, c, d, M, s, t) {\
    (a) += G(b, c, d) + (M) + (uint32_t)(t); \
    (a) = ROTATE_LEFT(a, s); \
    (a) += (b); \
    }
#define HH(a, b, c, d, M, s, t) {\
    (a) += H(b, c, d) + (M) + (uint32_t)(t); \
    (a) = ROTATE_LEFT(a, s); \
    (a) += (b); \
    }
#define II(a, b, c, d, M, s, t) {\
    (a) += I(b, c, d) + (M) + (uint32_t)(t); \
    (a) = ROTATE_LEFT(a, s); \
    (a) += (b); \
    }


uint32_t MD5Reverse(uint32_t aiVar)
{
    uint32_t        liVar = 1;
    unsigned char   lsBuf[5] = {0};

    memcpy(lsBuf, &liVar, 4);

    if((uint32_t)lsBuf[3] != 0)
    {
        memcpy(lsBuf, &aiVar, 4);
        
        lsBuf[4] = lsBuf[0];
        lsBuf[0] = lsBuf[3];
        lsBuf[3] = lsBuf[4];

        lsBuf[4] = lsBuf[1];
        lsBuf[1] = lsBuf[2];
        lsBuf[2] = lsBuf[4];

        memcpy(&liVar, lsBuf, 4);
        return  (liVar);
    }

    return  (aiVar);
}


size_t MD5Init(unsigned char* apcData, size_t aiLength)
{
    size_t          liIt = 0;
    size_t          liBits = 0;
    size_t          liLength = 0;
    unsigned char   lcValue = 0;

    liLength = (aiLength%64 < 56) ? (1+aiLength/64) : (2+aiLength/64);
    liLength = 64 * liLength;

    for(liIt = 0; liIt < (liLength-aiLength); liIt++)
    {
        lcValue = (liIt == 0) ? (unsigned char)0x80 : (unsigned char)0x00;
        apcData[aiLength+liIt] = lcValue;
    }

    liBits = aiLength*8;
    apcData[liLength-8] = (unsigned char)(liBits%256);

    liBits = liBits - liBits%256;
    apcData[liLength-7] = (unsigned char)(liBits/256);

    return  (liLength);
}


void MD5Transform(uint32_t* apiVarABCD, uint32_t* apiInData)
{
    uint32_t    a = apiVarABCD[0];
    uint32_t    b = apiVarABCD[1];
    uint32_t    c = apiVarABCD[2];
    uint32_t    d = apiVarABCD[3];

    FF(a, b, c, d, apiInData[ 0],  7, 0xD76AA478); /* 01 */
    FF(d, a, b, c, apiInData[ 1], 12, 0xE8C7B756); /* 02 */
    FF(c, d, a, b, apiInData[ 2], 17, 0x242070DB); /* 03 */
    FF(b, c, d, a, apiInData[ 3], 22, 0xC1BDCEEE); /* 04 */
    FF(a, b, c, d, apiInData[ 4],  7, 0xF57C0FAF); /* 05 */
    FF(d, a, b, c, apiInData[ 5], 12, 0x4787C62A); /* 06 */
    FF(c, d, a, b, apiInData[ 6], 17, 0xA8304613); /* 07 */
    FF(b, c, d, a, apiInData[ 7], 22, 0xFD469501); /* 08 */
    FF(a, b, c, d, apiInData[ 8],  7, 0x698098D8); /* 09 */
    FF(d, a, b, c, apiInData[ 9], 12, 0x8B44F7AF); /* 10 */
    FF(c, d, a, b, apiInData[10], 17, 0xFFFF5BB1); /* 11 */
    FF(b, c, d, a, apiInData[11], 22, 0x895CD7BE); /* 12 */
    FF(a, b, c, d, apiInData[12],  7, 0x6B901122); /* 13 */
    FF(d, a, b, c, apiInData[13], 12, 0xFD987193); /* 14 */
    FF(c, d, a, b, apiInData[14], 17, 0xA679438E); /* 15 */
    FF(b, c, d, a, apiInData[15], 22, 0x49B40821); /* 16 */

    GG(a, b, c, d, apiInData[ 1],  5, 0xF61E2562); /* 17 */
    GG(d, a, b, c, apiInData[ 6],  9, 0xC040B340); /* 18 */
    GG(c, d, a, b, apiInData[11], 14, 0x265E5A51); /* 19 */
    GG(b, c, d, a, apiInData[ 0], 20, 0xE9B6C7AA); /* 20 */
    GG(a, b, c, d, apiInData[ 5],  5, 0xD62F105D); /* 21 */
    GG(d, a, b, c, apiInData[10],  9, 0x02441453); /* 22 */
    GG(c, d, a, b, apiInData[15], 14, 0xD8A1E681); /* 23 */
    GG(b, c, d, a, apiInData[ 4], 20, 0xE7D3FBC8); /* 24 */
    GG(a, b, c, d, apiInData[ 9],  5, 0x21E1CDE6); /* 25 */
    GG(d, a, b, c, apiInData[14],  9, 0xC33707D6); /* 26 */
    GG(c, d, a, b, apiInData[ 3], 14, 0xF4D50D87); /* 27 */
    GG(b, c, d, a, apiInData[ 8], 20, 0x455A14ED); /* 28 */
    GG(a, b, c, d, apiInData[13],  5, 0xA9E3E905); /* 29 */
    GG(d, a, b, c, apiInData[ 2],  9, 0xFCEFA3F8); /* 30 */
    GG(c, d, a, b, apiInData[ 7], 14, 0x676F02D9); /* 31 */
    GG(b, c, d, a, apiInData[12], 20, 0x8D2A4C8A); /* 32 */

    HH(a, b, c, d, apiInData[ 5],  4, 0xFFFA3942); /* 33 */
    HH(d, a, b, c, apiInData[ 8], 11, 0x8771F681); /* 34 */
    HH(c, d, a, b, apiInData[11], 16, 0x6D9D6122); /* 35 */
    HH(b, c, d, a, apiInData[14], 23, 0xFDE5380C); /* 36 */
    HH(a, b, c, d, apiInData[ 1],  4, 0xA4BEEA44); /* 37 */
    HH(d, a, b, c, apiInData[ 4], 11, 0x4BDECFA9); /* 38 */
    HH(c, d, a, b, apiInData[ 7], 16, 0xF6BB4B60); /* 39 */
    HH(b, c, d, a, apiInData[10], 23, 0xBEBFBC70); /* 40 */
    HH(a, b, c, d, apiInData[13],  4, 0x289B7EC6); /* 41 */
    HH(d, a, b, c, apiInData[ 0], 11, 0xEAA127FA); /* 42 */
    HH(c, d, a, b, apiInData[ 3], 16, 0xD4EF3085); /* 43 */
    HH(b, c, d, a, apiInData[ 6], 23, 0x04881D05); /* 44 */
    HH(a, b, c, d, apiInData[ 9],  4, 0xD9D4D039); /* 45 */
    HH(d, a, b, c, apiInData[12], 11, 0xE6DB99E5); /* 46 */
    HH(c, d, a, b, apiInData[15], 16, 0x1FA27CF8); /* 47 */
    HH(b, c, d, a, apiInData[ 2], 23, 0xC4AC5665); /* 48 */

    II(a, b, c, d, apiInData[ 0],  6, 0xF4292244); /* 49 */
    II(d, a, b, c, apiInData[ 7], 10, 0x432AFF97); /* 50 */
    II(c, d, a, b, apiInData[14], 15, 0xAB9423A7); /* 51 */
    II(b, c, d, a, apiInData[ 5], 21, 0xFC93A039); /* 52 */
    II(a, b, c, d, apiInData[12],  6, 0x655B59C3); /* 53 */
    II(d, a, b, c, apiInData[ 3], 10, 0x8F0CCC92); /* 54 */
    II(c, d, a, b, apiInData[10], 15, 0xFFEFF47D); /* 55 */
    II(b, c, d, a, apiInData[ 1], 21, 0x85845DD1); /* 56 */
    II(a, b, c, d, apiInData[ 8],  6, 0x6FA87E4F); /* 57 */
    II(d, a, b, c, apiInData[15], 10, 0xFE2CE6E0); /* 58 */
    II(c, d, a, b, apiInData[ 6], 15, 0xA3014314); /* 59 */
    II(b, c, d, a, apiInData[13], 21, 0x4E0811A1); /* 60 */
    II(a, b, c, d, apiInData[ 4],  6, 0xF7537E82); /* 61 */
    II(d, a, b, c, apiInData[11], 10, 0xBD3AF235); /* 62 */
    II(c, d, a, b, apiInData[ 2], 15, 0x2AD7D2BB); /* 63 */
    II(b, c, d, a, apiInData[ 9], 21, 0xEB86D391); /* 64 */

    apiVarABCD[0] += a;
    apiVarABCD[1] += b;
    apiVarABCD[2] += c;
    apiVarABCD[3] += d;
}


size_t MD5Encode(void const* apInData, size_t aiLength, void* apOutData)
{
    size_t          liLength = 0;
    uint32_t        liVarSub = 0;
    uint32_t        liVarABCD[4+1];
    uint32_t        liVarSubArray[16+1];
    unsigned char   lsInData[128+64+1];
    
    liVarABCD[0] = 0x67452301;
    liVarABCD[1] = 0xEFCDAB89;
    liVarABCD[2] = 0x98BADCFE;
    liVarABCD[3] = 0x10325476;

    if(apInData == NULL || apOutData == NULL || aiLength < 0)
    {
        return (0);
    }

    liLength = (aiLength <= 128) ? aiLength : 128;

    memcpy(lsInData, apInData, liLength);
    liLength = MD5Init(lsInData, aiLength);

    for(size_t liLoop = 0; liLoop < liLength; liLoop+=64)
    {
        for(size_t liIk = 0; liIk < 64; liIk+=4)
        {
            memcpy(&liVarSub, &lsInData[liLoop+liIk], 4);
            liVarSubArray[liIk/4] = MD5Reverse(liVarSub);
        }
        
        MD5Transform(liVarABCD, liVarSubArray);
    }

    for(size_t liIj = 0; liIj < 16; liIj+=4)
    {
        liVarABCD[liIj/4] = MD5Reverse(liVarABCD[liIj/4]);
        memcpy(((char*)apOutData)+liIj, &liVarABCD[liIj/4], 4);
    }

    return  (aiLength);
}

////////////////////////////////////////////////////////////////////////////////////

size_t BCDEncode(void const* apInData, size_t aiLength, void* apOutData)
{
    unsigned char  lcChar = 0;
    unsigned char* lpcOut = (unsigned char*)apOutData;
    unsigned char const*    lpcIn = (unsigned char const*)apInData;
    unsigned char const*    lpcInEnd = lpcIn + aiLength;

    for(; lpcIn < lpcInEnd; lpcIn += 2)
    {
        lcChar = ((*lpcIn - '0')<<4) & 0x0F0;
        
        if(lpcIn+1 < lpcInEnd)
        {
            lcChar |= (*(lpcIn+1) - '0') & 0x0F;
        }

        *(lpcOut++) = lcChar;
    }
    *lpcOut= '\0';

    return  (lpcOut-(unsigned char*)apOutData);
}


size_t BCDDecode(void const* apInData, size_t aiLength, void* apOutData)
{
    unsigned char   lcChar = 0;
    unsigned char*  lpcOut = (unsigned char*)apOutData;
    unsigned char const* lpcIn = (unsigned char const*)apInData;
    unsigned char const* lpcInEnd = lpcIn + aiLength;

    for(; lpcIn < lpcInEnd; lpcIn++)
    {
        lcChar = *lpcIn;
        *(lpcOut++) = ((lcChar&0xF0)>>4) + '0';
        *(lpcOut++) = (lcChar&0x0F) + '0';
    }
    *lpcOut= '\0';

    return  (lpcOut - (unsigned char*)apOutData);
}

////////////////////////////////////////////////////////////////////////////

static unsigned char const _B64E_TAB[] =
{ // 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
    'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P', // 0
    'Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f', // 1
    'g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v', // 2
    'w','x','y','z','0','1','2','3','4','5','6','7','8','9','+','/', // 3
};
#define _B64_PADDING  ((unsigned char)'=')
#define _B64_INVALID  ((unsigned char)0xFF)

#define NL _B64_INVALID
static unsigned char const _B64D_TAB[] =
{ // 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
    NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL, // 0
    NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL, // 1
    NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,62,NL,NL,NL,63, // 2
    52,53,54,55,56,57,58,59,60,61,NL,NL,NL, 0,NL,NL, // 3
    NL, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14, // 4
    15,16,17,18,19,20,21,22,23,24,25,NL,NL,NL,NL,NL, // 5
    NL,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40, // 6
    41,42,43,44,45,46,47,48,49,50,51,NL,NL,NL,NL,NL, // 7
    NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL, // 8
    NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL, // 9
    NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL, // A
    NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL, // B
    NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL, // C
    NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL, // D
    NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL, // E
    NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL,NL, // F
};
#undef NL


#define _B64Encode3To4(apc3Bytes, apc4Chars) \
    apc4Chars[0] = _B64E_TAB[(apc3Bytes[0] >> 2) & 0x3F]; \
    apc4Chars[1] = _B64E_TAB[((apc3Bytes[0] << 4) & 0x30) | ((apc3Bytes[1] >> 4) & 0x0F)]; \
    apc4Chars[2] = _B64E_TAB[((apc3Bytes[1] << 2) & 0x3C) | ((apc3Bytes[2] >> 6) & 0x03)]; \
    apc4Chars[3] = _B64E_TAB[apc3Bytes[2] & 0x3F]; 


ssize_t BASE64Encode(void const* apInData, size_t aiInLength, void* apOutData, size_t aiOutLength)
{
    unsigned char*  lpcIn   = (unsigned char*)apInData;
    unsigned char*  lpcOut  = (unsigned char*)apOutData;

    size_t          liNeedSize      = BASE64_CALC_ENCODE_SIZE(aiInLength);
    size_t          liChunkCount    = aiInLength / 3;
    size_t          liDelta         = aiInLength % 3;

    assert(apInData != NULL && apOutData != NULL);

    AI_RETURN_IF(AI_B64_ERR_OUTSIZE, (aiOutLength < liNeedSize));

    for(size_t liIt = 0; liIt < liChunkCount; ++liIt)
    {
        _B64Encode3To4(lpcIn, lpcOut);
        lpcIn += 3;
        lpcOut += 4;
    }

    if (liDelta > 0)
    {
        unsigned char   lsLastBytes[3];

        memcpy(lsLastBytes, lpcIn, liDelta);

        if (1 == liDelta)
        {
            lsLastBytes[1] = '\0';
            lsLastBytes[2] = '\0';

            _B64Encode3To4(lsLastBytes, lpcOut);

            lpcOut[2] = _B64_PADDING;
            lpcOut[3] = _B64_PADDING;
            
        }
        else if (2 == liDelta)
        {
            lsLastBytes[2] = '\0';

            _B64Encode3To4(lsLastBytes, lpcOut);

            lpcOut[3] = _B64_PADDING;
        }
        else
        {
            assert(false && "shit happens!");
        }

        lpcIn += liDelta;
        lpcOut += 4;
    }

    *lpcOut++ = '\0';
    assert(lpcOut - (unsigned char*)apOutData == (ptrdiff_t)liNeedSize);

    return liNeedSize - 1;
}


#define _B64Decode4To3(apc4Chars, apc3Bytes) \
    apc3Bytes[0]    = ((_B64D_TAB[apc4Chars[0]] << 2) & 0xFC) | ((_B64D_TAB[apc4Chars[1]] >> 4) & 0x03); \
    apc3Bytes[1]    = ((_B64D_TAB[apc4Chars[1]] << 4) & 0xF0) | ((_B64D_TAB[apc4Chars[2]] >> 2) & 0x0F); \
    apc3Bytes[2]    = ((_B64D_TAB[apc4Chars[2]] << 6) & 0xC0) | ((_B64D_TAB[apc4Chars[3]] & 0x3F)); 

#define _B64_GET_NEXT_CHAR(c, p) do { c = *p++; }while(isspace(c))

ssize_t BASE64Decode(void const* apInData, size_t aiInLength, void* apOutData, size_t aiOutLength)
{
    unsigned char const*    lpcIn   = (unsigned char const*)apInData;
    unsigned char*          lpcOut  = (unsigned char*)apOutData;
    unsigned char   ls4Chars[4];
    size_t          liChunkCount;
    size_t          liNeedSize;
    ssize_t         liRet;

    assert(apInData != NULL && apOutData != NULL);

    liRet = BASE64Check(apInData, aiInLength);

    AI_RETURN_IF(liRet, (liRet < 0));
    AI_RETURN_IF(AI_B64_ERR_INSIZE, (liRet % 4));

    liChunkCount = liRet / 4;
    liNeedSize = liChunkCount * 3;

    AI_RETURN_IF(AI_B64_ERR_OUTSIZE, (liNeedSize > aiOutLength));

    for (size_t i = 0; i < liChunkCount; ++i)
    {

        _B64_GET_NEXT_CHAR(ls4Chars[0], lpcIn); 
        _B64_GET_NEXT_CHAR(ls4Chars[1], lpcIn); 
        _B64_GET_NEXT_CHAR(ls4Chars[2], lpcIn); 
        _B64_GET_NEXT_CHAR(ls4Chars[3], lpcIn); 

        _B64Decode4To3(ls4Chars, lpcOut);
        lpcOut += 3;
    } 

    if (_B64_PADDING == ls4Chars[2])
    {
        lpcOut -= 2;
    }
    else if (_B64_PADDING == ls4Chars[3])
    {
        lpcOut -= 1;
    }

    return  lpcOut - (unsigned char*)apOutData;
}


ssize_t BASE64Check(void const* apData, size_t aiLength)
{
    char const* lpcIn = (char const*)apData;
    char const* lpcInEnd = lpcIn + aiLength;
    size_t  liWSCount = 0; 
    size_t  liPaddingCount = 0;
    size_t  liDataLength = 0;

    assert(apData != NULL);

    for(; lpcIn < lpcInEnd; ++lpcIn)
    {
        if (isspace(*lpcIn))
        {
            ++liWSCount;
            continue;
        }
        else if (_B64_INVALID == _B64D_TAB[(int)*lpcIn])
        {
            return AI_B64_ERR_INPUT;
        }
        else if ('=' == *lpcIn)
        {
            break;
        }
    }
    
    for (; lpcIn < lpcInEnd; ++lpcIn)
    {
        if (isspace(*lpcIn))
        {
            ++liWSCount;
            continue;
        }
        else if ('=' != *lpcIn)
        {
            return AI_B64_ERR_PADDING;
        }
        ++liPaddingCount;
    }

    if (liPaddingCount > 2)
    {
        return AI_B64_ERR_PADDING;
    }
    
    liDataLength = (lpcIn - (char const*)apData) - liWSCount;

    if (liDataLength % 4)
    {
        return AI_B64_ERR_INPUT;
    }

    return liDataLength;
}

///end namespace
AIBC_NAMESPACE_END
