
#include "acl/Hashs.h"

ACL_NAMESPACE_START

////////////////////////////////////////////////////////////////////////////////////////////////////
#define _MD5_ROTATE_LEFT(x, n) (((x)<<(n)) | ((x)>>(32-(n))))
#define _MD5_F(X, Y, Z) (((X)&(Y)) | ((~X)&(Z)))
#define _MD5_G(X, Y, Z) (((X)&(Z)) | ((Y)&(~Z)))
#define _MD5_I(X, Y, Z) ((Y) ^ ((X)|(~Z)))
#define _MD5_H(X, Y, Z) ((X)^(Y)^(Z))

#define _MD5_FF(a, b, c, d, M, s, t) {\
    (a) += _MD5_F(b, c, d) + (M) + (apl_uint32_t)(t); \
    (a) = _MD5_ROTATE_LEFT(a, s); \
    (a) += (b); \
    }
#define _MD5_GG(a, b, c, d, M, s, t) {\
    (a) += _MD5_G(b, c, d) + (M) + (apl_uint32_t)(t); \
    (a) = _MD5_ROTATE_LEFT(a, s); \
    (a) += (b); \
    }
#define _MD5_HH(a, b, c, d, M, s, t) {\
    (a) += _MD5_H(b, c, d) + (M) + (apl_uint32_t)(t); \
    (a) = _MD5_ROTATE_LEFT(a, s); \
    (a) += (b); \
    }
#define _MD5_II(a, b, c, d, M, s, t) {\
    (a) += _MD5_I(b, c, d) + (M) + (apl_uint32_t)(t); \
    (a) = _MD5_ROTATE_LEFT(a, s); \
    (a) += (b); \
    }

CMD5::CMD5(void)
{
    apl_memset(this->macPadding, 0, sizeof(macPadding));
	this->macPadding[0] = 0x80;
	
    this->Reset();
}

void CMD5::Update( char const* apcInput )
{
    this->Update(apcInput, apl_strlen(apcInput) );
}
   
void CMD5::Update( void const* apvInput, apl_size_t auInputLen )
{
    unsigned char const* lpcInput = (unsigned char const*)apvInput;
    apl_size_t luIndex   = 0;
    apl_size_t luCurrLen = 0;
    apl_size_t luPartLen = 0;

	//Compute number of bytes mod 64
	luCurrLen = (apl_size_t)((this->mi64Count >> 3) & 0x3F);
    
    this->mi64Count += auInputLen << 3;//number of bit, = auInputLen * 8
    
	luPartLen = 64 - luCurrLen;

	if (auInputLen >= luPartLen)
	{
		this->MD5Memcpy(&this->macBuffer[luCurrLen], lpcInput, luPartLen);
		
		this->MD5Transform(this->mauState, this->macBuffer);

		for(luIndex = luPartLen; luIndex + 63 < auInputLen; luIndex += 64)
	    {
	        this->MD5Transform(this->mauState, &lpcInput[luIndex]);
	    }

		luCurrLen = 0;
	}

	// Buffer remaining input
	this->MD5Memcpy(&this->macBuffer[luCurrLen], &lpcInput[luIndex], auInputLen - luIndex);
}
    
CMD5::DigestType CMD5::Final(void)
{
	apl_size_t    luCurrLen = 0;
	apl_size_t    luPadLen = 0;
	apl_uint32_t  lauCount[2];
	unsigned char lacLengthBit[8];
	DigestType    loDigest;
	
	lauCount[0] = this->mi64Count & 0xFFFF;
	lauCount[1] = this->mi64Count >> 32 & 0xFFFF;
    
    this->MD5Encode(lacLengthBit, lauCount, 2);

	luCurrLen = (apl_size_t)((this->mi64Count >> 3) & 0x3F);
	luPadLen = luCurrLen < 56 ? 56 - luCurrLen : 120 - luCurrLen;
	
	this->Update(this->macPadding, luPadLen);

	//Append length 64bit
	this->Update(lacLengthBit, 8);
	
	this->MD5Encode(loDigest.macDigest, this->mauState, 4);

	this->Reset();
	
	return loDigest;
}

CMD5::DigestType CMD5::Final( char const* apcInput )
{
    this->Update(apcInput);
    return this->Final();
}

CMD5::DigestType CMD5::Final( void const* apvInput, apl_size_t auInputLen )
{
    this->Update(apvInput, auInputLen);
    return this->Final();
}

void CMD5::Reset(void)
{
	// Load magic initialization constants.
	this->mauState[0] = 0x67452301;
	this->mauState[1] = 0xefcdab89;
	this->mauState[2] = 0x98badcfe;
	this->mauState[3] = 0x10325476;
	
	//Number of encode bit
	this->mi64Count = 0;
}

void CMD5::MD5Transform( apl_uint32_t* apiState, unsigned char const* apvInput )
{
    apl_uint32_t a = apiState[0];
    apl_uint32_t b = apiState[1];
    apl_uint32_t c = apiState[2];
    apl_uint32_t d = apiState[3];
    apl_uint32_t luTemp[16];
    
    this->MD5Decode(luTemp, apvInput, 64);

    _MD5_FF(a, b, c, d, luTemp[ 0],  7, 0xD76AA478); /* 01 */
    _MD5_FF(d, a, b, c, luTemp[ 1], 12, 0xE8C7B756); /* 02 */
    _MD5_FF(c, d, a, b, luTemp[ 2], 17, 0x242070DB); /* 03 */
    _MD5_FF(b, c, d, a, luTemp[ 3], 22, 0xC1BDCEEE); /* 04 */
    _MD5_FF(a, b, c, d, luTemp[ 4],  7, 0xF57C0FAF); /* 05 */
    _MD5_FF(d, a, b, c, luTemp[ 5], 12, 0x4787C62A); /* 06 */
    _MD5_FF(c, d, a, b, luTemp[ 6], 17, 0xA8304613); /* 07 */
    _MD5_FF(b, c, d, a, luTemp[ 7], 22, 0xFD469501); /* 08 */
    _MD5_FF(a, b, c, d, luTemp[ 8],  7, 0x698098D8); /* 09 */
    _MD5_FF(d, a, b, c, luTemp[ 9], 12, 0x8B44F7AF); /* 10 */
    _MD5_FF(c, d, a, b, luTemp[10], 17, 0xFFFF5BB1); /* 11 */
    _MD5_FF(b, c, d, a, luTemp[11], 22, 0x895CD7BE); /* 12 */
    _MD5_FF(a, b, c, d, luTemp[12],  7, 0x6B901122); /* 13 */
    _MD5_FF(d, a, b, c, luTemp[13], 12, 0xFD987193); /* 14 */
    _MD5_FF(c, d, a, b, luTemp[14], 17, 0xA679438E); /* 15 */
    _MD5_FF(b, c, d, a, luTemp[15], 22, 0x49B40821); /* 16 */

    _MD5_GG(a, b, c, d, luTemp[ 1],  5, 0xF61E2562); /* 17 */
    _MD5_GG(d, a, b, c, luTemp[ 6],  9, 0xC040B340); /* 18 */
    _MD5_GG(c, d, a, b, luTemp[11], 14, 0x265E5A51); /* 19 */
    _MD5_GG(b, c, d, a, luTemp[ 0], 20, 0xE9B6C7AA); /* 20 */
    _MD5_GG(a, b, c, d, luTemp[ 5],  5, 0xD62F105D); /* 21 */
    _MD5_GG(d, a, b, c, luTemp[10],  9, 0x02441453); /* 22 */
    _MD5_GG(c, d, a, b, luTemp[15], 14, 0xD8A1E681); /* 23 */
    _MD5_GG(b, c, d, a, luTemp[ 4], 20, 0xE7D3FBC8); /* 24 */
    _MD5_GG(a, b, c, d, luTemp[ 9],  5, 0x21E1CDE6); /* 25 */
    _MD5_GG(d, a, b, c, luTemp[14],  9, 0xC33707D6); /* 26 */
    _MD5_GG(c, d, a, b, luTemp[ 3], 14, 0xF4D50D87); /* 27 */
    _MD5_GG(b, c, d, a, luTemp[ 8], 20, 0x455A14ED); /* 28 */
    _MD5_GG(a, b, c, d, luTemp[13],  5, 0xA9E3E905); /* 29 */
    _MD5_GG(d, a, b, c, luTemp[ 2],  9, 0xFCEFA3F8); /* 30 */
    _MD5_GG(c, d, a, b, luTemp[ 7], 14, 0x676F02D9); /* 31 */
    _MD5_GG(b, c, d, a, luTemp[12], 20, 0x8D2A4C8A); /* 32 */

    _MD5_HH(a, b, c, d, luTemp[ 5],  4, 0xFFFA3942); /* 33 */
    _MD5_HH(d, a, b, c, luTemp[ 8], 11, 0x8771F681); /* 34 */
    _MD5_HH(c, d, a, b, luTemp[11], 16, 0x6D9D6122); /* 35 */
    _MD5_HH(b, c, d, a, luTemp[14], 23, 0xFDE5380C); /* 36 */
    _MD5_HH(a, b, c, d, luTemp[ 1],  4, 0xA4BEEA44); /* 37 */
    _MD5_HH(d, a, b, c, luTemp[ 4], 11, 0x4BDECFA9); /* 38 */
    _MD5_HH(c, d, a, b, luTemp[ 7], 16, 0xF6BB4B60); /* 39 */
    _MD5_HH(b, c, d, a, luTemp[10], 23, 0xBEBFBC70); /* 40 */
    _MD5_HH(a, b, c, d, luTemp[13],  4, 0x289B7EC6); /* 41 */
    _MD5_HH(d, a, b, c, luTemp[ 0], 11, 0xEAA127FA); /* 42 */
    _MD5_HH(c, d, a, b, luTemp[ 3], 16, 0xD4EF3085); /* 43 */
    _MD5_HH(b, c, d, a, luTemp[ 6], 23, 0x04881D05); /* 44 */
    _MD5_HH(a, b, c, d, luTemp[ 9],  4, 0xD9D4D039); /* 45 */
    _MD5_HH(d, a, b, c, luTemp[12], 11, 0xE6DB99E5); /* 46 */
    _MD5_HH(c, d, a, b, luTemp[15], 16, 0x1FA27CF8); /* 47 */
    _MD5_HH(b, c, d, a, luTemp[ 2], 23, 0xC4AC5665); /* 48 */

    _MD5_II(a, b, c, d, luTemp[ 0],  6, 0xF4292244); /* 49 */
    _MD5_II(d, a, b, c, luTemp[ 7], 10, 0x432AFF97); /* 50 */
    _MD5_II(c, d, a, b, luTemp[14], 15, 0xAB9423A7); /* 51 */
    _MD5_II(b, c, d, a, luTemp[ 5], 21, 0xFC93A039); /* 52 */
    _MD5_II(a, b, c, d, luTemp[12],  6, 0x655B59C3); /* 53 */
    _MD5_II(d, a, b, c, luTemp[ 3], 10, 0x8F0CCC92); /* 54 */
    _MD5_II(c, d, a, b, luTemp[10], 15, 0xFFEFF47D); /* 55 */
    _MD5_II(b, c, d, a, luTemp[ 1], 21, 0x85845DD1); /* 56 */
    _MD5_II(a, b, c, d, luTemp[ 8],  6, 0x6FA87E4F); /* 57 */
    _MD5_II(d, a, b, c, luTemp[15], 10, 0xFE2CE6E0); /* 58 */
    _MD5_II(c, d, a, b, luTemp[ 6], 15, 0xA3014314); /* 59 */
    _MD5_II(b, c, d, a, luTemp[13], 21, 0x4E0811A1); /* 60 */
    _MD5_II(a, b, c, d, luTemp[ 4],  6, 0xF7537E82); /* 61 */
    _MD5_II(d, a, b, c, luTemp[11], 10, 0xBD3AF235); /* 62 */
    _MD5_II(c, d, a, b, luTemp[ 2], 15, 0x2AD7D2BB); /* 63 */
    _MD5_II(b, c, d, a, luTemp[ 9], 21, 0xEB86D391); /* 64 */

    apiState[0] += a;
    apiState[1] += b;
    apiState[2] += c;
    apiState[3] += d;
}

void CMD5::MD5Encode(unsigned char* apcOutput, apl_uint32_t const* apuInput, apl_size_t auInputLen)
{
	for ( apl_size_t luI = 0, luO = 0; luI < auInputLen; luI++, luO += 4)
	{
		apcOutput[luO]   = (unsigned char)(apuInput[luI] & 0xFF);
		apcOutput[luO+1] = (unsigned char)((apuInput[luI] >> 8) & 0xFF);
		apcOutput[luO+2] = (unsigned char)((apuInput[luI] >> 16) & 0xFF);
		apcOutput[luO+3] = (unsigned char)((apuInput[luI] >> 24) & 0xFF);
	}
}

void CMD5::MD5Decode(apl_uint32_t* apuOutput, unsigned char const* apcInput, apl_size_t auInputLen)
{
	for ( apl_size_t luI = 0, luO = 0; luI < auInputLen; luO++, luI += 4)
    {
		apuOutput[luO]  = ((apl_uint32_t)apcInput[luI]);
		apuOutput[luO] |= (((apl_uint32_t)apcInput[luI+1]) << 8);
		apuOutput[luO] |= (((apl_uint32_t)apcInput[luI+2]) << 16);
		apuOutput[luO] |= (((apl_uint32_t)apcInput[luI+3]) << 24);
    }
}

void CMD5::MD5Memcpy (unsigned char* apcOutput, unsigned char const* apcInput, apl_size_t auLen)
{
	for (apl_size_t luN = 0; luN < auLen; luN++)
	{
	    apcOutput[luN] = apcInput[luN];
    }
}

ACL_NAMESPACE_END
