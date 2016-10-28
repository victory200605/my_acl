
#include "acl/codecs/Unicode.h"
#include "Iconv.inl"

ACL_NAMESPACE_START

//Some fundamental constants
const apl_uint32_t UNI_REPLACEMENT_CHAR = APL_UINT32_C(0x0000003F);
const apl_uint32_t UNI_MAX_BMP          = APL_UINT32_C(0x0000FFFF);
const apl_uint32_t UNI_MAX_UTF16        = APL_UINT32_C(0x0010FFFF);
const apl_uint32_t UNI_MAX_UTF32        = APL_UINT32_C(0x7FFFFFFF);
const apl_uint32_t UNI_MAX_LEGAL_UTF32  = APL_UINT32_C(0x0010FFFF);

const apl_uint32_t UNI_SUR_HIGH_START   = APL_UINT32_C(0xD800);
const apl_uint32_t UNI_SUR_HIGH_END     = APL_UINT32_C(0xDBFF);
const apl_uint32_t UNI_SUR_LOW_START    = APL_UINT32_C(0xDC00);
const apl_uint32_t UNI_SUR_LOW_END      = APL_UINT32_C(0xDFFF);

/* used for shifting by 10 bits */
const apl_int_t HALF_SHIFT    = 10;
const apl_uint32_t HALF_BASE  = APL_UINT32_C(0x0010000);
const apl_uint32_t HALF_MASK   = APL_UINT32_C(0x3FF);

/*
 * Once the bits are split out into bytes of UTF-8, this is a mask OR-ed
 * into the first byte, depending on how many bytes follow.  There are
 * as many entries in this table as there are UTF-8 sequence types.
 * (I.e., one byte sequence, two byte... etc.). Remember that sequencs
 * for *legal* UTF-8 will be 4 or fewer bytes total.
 */
unsigned char const FIRST_BYTE_MARK[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

/*
 * Index into the table below with the first byte of a UTF-8 sequence to
 * get the number of trailing bytes that are supposed to follow it.
 * Note that *legal* UTF-8 values can't have 4 or 5-bytes. The table is
 * left as-is for anyone who may want to do such conversion, which was
 * allowed in earlier algorithms.
 */
const char TRAILING_BYTES_FOR_UTF8[256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};

/*
 * Magic values subtracted from a buffer value during UTF8 conversion.
 * This table contains as many values as there might be trailing bytes
 * in a UTF-8 sequence.
 */
const apl_uint32_t OFFSETS_FROM_UTF8[6] = { 0x00000000UL, 0x00003080UL, 0x000E2080UL, 
    0x03C82080UL, 0xFA082080UL, 0x82082080UL };

/*
 * Utility routine to tell whether a sequence of bytes is legal UTF-8.
 * This must be called with the length pre-determined by the first byte.
 * If not calling this from ConvertUTF8to*, then the length can be set by:
 *  length = TRAILING_BYTES_FOR_UTF8[*source]+1;
 * and the sequence is illegal right away if there aren't that many bytes
 * available.
 * If presented with a length > 4, this returns false.  The Unicode
 * definition of UTF-8 goes up to 4-byte sequences.
 */
bool IsLegalUTF8( unsigned char const* apcInput, apl_size_t auLength )
{
    unsigned char a;
    unsigned char const* lpcSrcPtr = apcInput+auLength;
    
    switch (auLength) {
        default: return false;
    	// Everything else falls through when "true"...
        case 4: if ((a = (*--lpcSrcPtr)) < 0x80 || a > 0xBF) return false;
        case 3: if ((a = (*--lpcSrcPtr)) < 0x80 || a > 0xBF) return false;
        case 2: if ((a = (*--lpcSrcPtr)) > 0xBF) return false;
    
    	switch (*apcInput) {
    	    // no fall-through in this inner switch
    	    case 0xE0: if (a < 0xA0) return false; break;//not require three bytes
    	    case 0xED: if (a > 0x9F) return false; break;//(>0xD800)
    	    case 0xF0: if (a < 0x90) return false; break;//not require four bytes
    	    case 0xF4: if (a > 0x8F) return false; break;//(>0x10FF)
    	    default:   if (a < 0x80) return false;
    	}
    
        case 1: if (*apcInput >= 0x80 && *apcInput < 0xC2) return false;
    }
    
    if (*apcInput > 0xF4) return false;
    
    return true;
}

apl_int_t WriteUTF8( apl_uint32_t auUnicode, unsigned char* apcOutput, bool abIsStrict )
{
    apl_uint32_t       luUnicode = auUnicode;
    unsigned char*     lpcOutput = apcOutput;
    apl_size_t         luBytesToWrite = 0;
    const apl_uint32_t luByteMask = 0xBF;
    const apl_uint32_t luByteMark = 0x80;

    // Figure out how many bytes the result will require
    if (luUnicode < APL_UINT32_C(0x80) )
    {
        luBytesToWrite = 1;
    }
    else if (luUnicode < APL_UINT32_C(0x800) )
    {
        luBytesToWrite = 2;
    }
    else if (luUnicode < APL_UINT32_C(0x10000) )
    {
        luBytesToWrite = 3;
    }
    else if (luUnicode < APL_UINT32_C(0x110000) )
    {
        luBytesToWrite = 4;
    }
    else 
    {
        if (!abIsStrict)
        {
            luBytesToWrite = 1;
            luUnicode = UNI_REPLACEMENT_CHAR;
        }
        else
        {
            return -1;
        }
    }

    lpcOutput += luBytesToWrite;

    switch (luBytesToWrite) { //Note: everything falls through.
        case 4: *--lpcOutput = (unsigned char)((luUnicode | luByteMark) & luByteMask); luUnicode >>= 6;
        case 3: *--lpcOutput = (unsigned char)((luUnicode | luByteMark) & luByteMask); luUnicode >>= 6;
        case 2: *--lpcOutput = (unsigned char)((luUnicode | luByteMark) & luByteMask); luUnicode >>= 6;
        case 1: *--lpcOutput = (unsigned char)(luUnicode | FIRST_BYTE_MARK[luBytesToWrite]);
    }

    return luBytesToWrite;
}

apl_int_t ReadUTF8( unsigned char const** apcInput, unsigned char const* apcLast, apl_uint32_t* apuUnicode, bool abIsStrict )
{
    unsigned char const*& lpcInput = *apcInput;
    apl_uint32_t& luUnicode = *apuUnicode;
    apl_size_t    luExtBytesToRead = TRAILING_BYTES_FOR_UTF8[*lpcInput];
    
    luUnicode = 0;

    if ( (apl_size_t)(apcLast - lpcInput) < luExtBytesToRead + 1)
    {
        return luExtBytesToRead + 1;
    }
    
    if (!IsLegalUTF8(lpcInput, luExtBytesToRead + 1) )
    {
        if (abIsStrict)
        {
            return -1;
        }
        else
        {
            luUnicode = UNI_REPLACEMENT_CHAR;
            lpcInput += luExtBytesToRead + 1;
            
            return 0;
        }
    }
    
    switch (luExtBytesToRead)
    {
        case 5: luUnicode += *lpcInput++; luUnicode <<= 6; /* remember, illegal UTF-8 */
        case 4: luUnicode += *lpcInput++; luUnicode <<= 6; /* remember, illegal UTF-8 */
        case 3: luUnicode += *lpcInput++; luUnicode <<= 6;
        case 2: luUnicode += *lpcInput++; luUnicode <<= 6;
        case 1: luUnicode += *lpcInput++; luUnicode <<= 6;
        case 0: luUnicode += *lpcInput++;
    }
    
    luUnicode -= OFFSETS_FROM_UTF8[luExtBytesToRead];

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CBigEndian
{
public:
    static void Transform2( unsigned char ch1, unsigned char ch2, unsigned char* apcOutput )
    {
        apcOutput[0] = ch1;
        apcOutput[1] = ch2;
    }
    static void Transform4( unsigned char ch1, unsigned char ch2, unsigned char ch3, unsigned char ch4, unsigned char* apcOutput )
    {
        apcOutput[0] = ch1;
        apcOutput[1] = ch2;
        apcOutput[2] = ch3;
        apcOutput[3] = ch4;
    }
};

class CLittleEndian
{
public:
    static void Transform2( unsigned char ch1, unsigned char ch2, unsigned char* apcOutput )
    {
        apcOutput[1] = ch1;
        apcOutput[0] = ch2;
    }
    static void Transform4( unsigned char ch1, unsigned char ch2, unsigned char ch3, unsigned char ch4, unsigned char* apcOutput )
    {
        apcOutput[3] = ch1;
        apcOutput[2] = ch2;
        apcOutput[1] = ch3;
        apcOutput[0] = ch4;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///UTF32 to UTF16
CUTF32ToUTF16::CUTF32ToUTF16( ETranscoderFromEndian aeFromEndian, ETranscoderToEndian aeToEndian, bool abIsStrict )
    : mbIsStrictTranscode(abIsStrict)
    , mfFromTransform2(CBigEndian::Transform2)
    , mfFromTransform4(CBigEndian::Transform4)
    , mfToTransform2(CBigEndian::Transform2)
    , mfToTransform4(CBigEndian::Transform4)
{
    if (aeFromEndian == FROM_LITTLE_ENDIAN)
    {
        this->mfFromTransform2 = CLittleEndian::Transform2;
        this->mfFromTransform4 = CLittleEndian::Transform4;
    }

    if (aeToEndian == TO_LITTLE_ENDIAN)
    {
        this->mfToTransform2 = CLittleEndian::Transform2;
        this->mfToTransform4 = CLittleEndian::Transform4;
    }
}
    
apl_size_t CUTF32ToUTF16::CalcNeedSize( apl_size_t auLength )
{
    return auLength + 2/*fill replacemen char*/ + 1;
}

apl_ssize_t CUTF32ToUTF16::Transcode( unsigned char const** apcInput, apl_size_t auLength, unsigned char** apcOutput )
{
    unsigned char const*& lpcInput  = *apcInput;
    unsigned char const*  lpcLast   = lpcInput + auLength;
    unsigned char*&       lpcOutput = *apcOutput;
    unsigned char lacCode[4];
    apl_uint32_t  luUnicode = 0;
    
    while(lpcInput < lpcLast)
    {
        if (lpcLast - lpcInput < 4)
        {
            return 4;
        }

        (*this->mfFromTransform4)(lpcInput[0], lpcInput[1], lpcInput[2], lpcInput[3], lacCode);
        
        lpcInput += 4;
        
        luUnicode = lacCode[0] << 24 | lacCode[1] << 16 | lacCode[2] << 8 | lacCode[3];
    
        if (luUnicode <= UNI_MAX_BMP)//BMP
        {
            //Target is a character <= 0xFFFF
            if(luUnicode >= UNI_SUR_HIGH_START && luUnicode <= UNI_SUR_LOW_END )
            {
                if (this->FillReplacementChar(&lpcOutput) != 0)
                {
                    return -1;
                }
            }
            else
            {
                (*this->mfToTransform2)(lacCode[2], lacCode[3], lpcOutput);
                lpcOutput += 2;
            }
        }
        else if (luUnicode > APL_UINT32_C(0x0010FFFF) )
        {
            if (this->FillReplacementChar(&lpcOutput) != 0)
            {
                return -1;
            }
        }
        else
        {
            // target is a character in range 0xFFFF - 0x10FFFF.
    	    luUnicode -= HALF_BASE;
    	    apl_uint16_t luHigh = (apl_uint16_t)((luUnicode >> HALF_SHIFT) + UNI_SUR_HIGH_START);
    	    apl_uint16_t luLow  = (apl_uint16_t)((luUnicode & HALF_MASK) + UNI_SUR_LOW_START);
    	    
    	    (*this->mfToTransform2)(luHigh >> 8, luHigh & 0xFF, lpcOutput);
            lpcOutput += 2;
            
            (*this->mfToTransform2)(luLow >> 8, luLow & 0xFF, lpcOutput);     
            lpcOutput += 2;
        }
    }
    
    return 0;
}

apl_int_t CUTF32ToUTF16::FillReplacementChar(unsigned char** apcOutput)
{
    if (!this->mbIsStrictTranscode)
    {
        unsigned char*& lpcOutput = *apcOutput;
        
        //UNI_REPLACEMENT_CHAR = 0x0000FFFD;
        (*this->mfToTransform2)(
            (UNI_REPLACEMENT_CHAR >> 8) & 0xFF,
            (UNI_REPLACEMENT_CHAR) & 0xFF,
            lpcOutput );
        lpcOutput += 2;
        
        return 0;
    }
    else
    {
        return -1;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///UTF32 to UTF16
CUTF16ToUTF32::CUTF16ToUTF32( ETranscoderFromEndian aeFromEndian, ETranscoderToEndian aeToEndian, bool abIsStrict )
    : mbIsStrictTranscode(abIsStrict)
    , mfFromTransform2(CBigEndian::Transform2)
    , mfFromTransform4(CBigEndian::Transform4)
    , mfToTransform2(CBigEndian::Transform2)
    , mfToTransform4(CBigEndian::Transform4)
{
    if (aeFromEndian == FROM_LITTLE_ENDIAN)
    {
        this->mfFromTransform2 = CLittleEndian::Transform2;
        this->mfFromTransform4 = CLittleEndian::Transform4;
    }

    if (aeToEndian == TO_LITTLE_ENDIAN)
    {
        this->mfToTransform2 = CLittleEndian::Transform2;
        this->mfToTransform4 = CLittleEndian::Transform4;
    }
}

apl_size_t CUTF16ToUTF32::CalcNeedSize( apl_size_t auLength )
{
    return auLength * 2 + 4/*fill replacemen char*/ + 1;
}

apl_ssize_t CUTF16ToUTF32::Transcode( unsigned char const** apcInput, apl_size_t auLength, unsigned char** apcOutput )
{
    unsigned char const*& lpcInput  = *apcInput;
    unsigned char const*  lpcLast   = lpcInput + auLength;
    unsigned char*&       lpcOutput = *apcOutput;
    unsigned char         lacCode[2];
    apl_uint32_t          luUnicode = 0;

    while(lpcInput < lpcLast)
    {
        if (lpcLast - lpcInput < 2)
        {
            return 2;
        }
        
        (*this->mfFromTransform2)(lpcInput[0], lpcInput[1], lacCode);
            
        luUnicode = lacCode[0] << 8 | lacCode[1];

    	if (luUnicode >= UNI_SUR_HIGH_START && luUnicode <= UNI_SUR_HIGH_END)
    	{
    	    unsigned char lacTemp[2];
    	    apl_uint32_t  luCh2 = 0;
    	    
            // If we have a surrogate pair, convert to UTF32 first.
            if (lpcLast - lpcInput < 4)
            {
                return 4;
            }
            
            (*this->mfFromTransform2)(lpcInput[2], lpcInput[3], lacTemp);
            luCh2 = lacTemp[0] << 8 | lacTemp[1];

            if (luCh2 >= UNI_SUR_LOW_START && luCh2 <= UNI_SUR_LOW_END)
            {
                luUnicode = ((luUnicode - UNI_SUR_HIGH_START) << HALF_SHIFT)
        		    + (luCh2 - UNI_SUR_LOW_START) + HALF_BASE;
        			
                lpcInput += 2;
            }
            else
            {
                if (!this->mbIsStrictTranscode)
                {
                    luUnicode = UNI_REPLACEMENT_CHAR;
                }
                else
                {
                    return -1;
                }
            }
        }
        else if (luUnicode >= UNI_SUR_LOW_START && luUnicode <= UNI_SUR_LOW_END)
        {
            if (!this->mbIsStrictTranscode)
            {
                luUnicode = UNI_REPLACEMENT_CHAR;
            }
            else
            {
                return -1;
            }
        }
        
        lpcInput += 2;
        
        (*this->mfToTransform4)(
            (luUnicode >> 24) & 0xFF,
            (luUnicode >> 16) & 0xFF,
            (luUnicode >> 8) & 0xFF,
            (luUnicode) & 0xFF,
            lpcOutput );
        lpcOutput += 4;
    }
    
    return 0;
}

apl_int_t CUTF16ToUTF32::FillReplacementChar(unsigned char** apcOutput)
{
    if (!this->mbIsStrictTranscode)
    {
        unsigned char*& lpcOutput = *apcOutput;
        
        //UNI_REPLACEMENT_CHAR = 0x0000FFFD;
        (*this->mfToTransform4)(
            (UNI_REPLACEMENT_CHAR >> 24) & 0xFF,
            (UNI_REPLACEMENT_CHAR >> 16) & 0xFF,
            (UNI_REPLACEMENT_CHAR >> 8) & 0xFF,
            (UNI_REPLACEMENT_CHAR) & 0xFF,
            lpcOutput );
        lpcOutput += 4;
        
        return 0;
    }
    else
    {
        return -1;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///UTF16 to UTF8
CUTF16ToUTF8::CUTF16ToUTF8( ETranscoderFromEndian aeFromEndian, bool abIsStrict )
    : mbIsStrictTranscode(abIsStrict)
    , mfFromTransform2(CBigEndian::Transform2)
    , mfFromTransform4(CBigEndian::Transform4)
{
    if (aeFromEndian == FROM_LITTLE_ENDIAN)
    {
        this->mfFromTransform2 = CLittleEndian::Transform2;
        this->mfFromTransform4 = CLittleEndian::Transform4;
    }
}

apl_size_t CUTF16ToUTF8::CalcNeedSize( apl_size_t auLength )
{
    return (auLength / 2 + 1) * 5 + 5/*fill replacement char*/ + 1;
}

apl_ssize_t CUTF16ToUTF8::Transcode( unsigned char const** apcInput, apl_size_t auLength, unsigned char** apcOutput )
{
    unsigned char const*& lpcInput  = *apcInput;
    unsigned char const*  lpcLast   = lpcInput + auLength;
    unsigned char*&       lpcOutput = *apcOutput;
    unsigned char         lacCode[2];
    apl_uint32_t          luUnicode = 0;
    apl_int_t             liBytesToWrite = 0;
    
    while(lpcInput < lpcLast)
    {
        if (lpcLast - lpcInput < 2)
        {
            return 2;
        }
        
        //Trans to big endian
        (*this->mfFromTransform2)(lpcInput[0], lpcInput[1], lacCode);

        luUnicode = lacCode[0] << 8 | lacCode[1];

    	if (luUnicode >= UNI_SUR_HIGH_START && luUnicode <= UNI_SUR_HIGH_END)
    	{
    	    unsigned char lacTemp[2];
    	    apl_uint32_t  luCh2 = 0;
    	    
            //If we have a surrogate pair, convert to UTF32 first.
            if (lpcLast - lpcInput < 4)
            {
                return 4;
            }
            
            (*this->mfFromTransform2)(lpcInput[2], lpcInput[3], lacTemp);
            luCh2 = lacTemp[0] << 8 | lacTemp[1];
 
            if (luCh2 >= UNI_SUR_LOW_START && luCh2 <= UNI_SUR_LOW_END)
            {
                //Target is a character in range 0xFFFF - 0x10FFFF
                //0001xxxx 11111111 11111111
                luUnicode = ((luUnicode - UNI_SUR_HIGH_START) << 10)
    			    + (luCh2 - UNI_SUR_LOW_START) + APL_UINT32_C(0x0010000);
    			
    			lpcInput += 2;
            }
            else
            {
                if (!this->mbIsStrictTranscode)//it's an unpaired high surrogate
                {
                    luUnicode = UNI_REPLACEMENT_CHAR;
                }
                else
                {
                    return -1;
                }
            }
        }
        else if (luUnicode >= UNI_SUR_LOW_START && luUnicode <= UNI_SUR_LOW_END)
        {
            if (!this->mbIsStrictTranscode)//it's an unpaired high surrogate
            {
                luUnicode = UNI_REPLACEMENT_CHAR;
            }
            else
            {
                return -1;
            }
        }

        if ( (liBytesToWrite = WriteUTF8(luUnicode, lpcOutput, this->mbIsStrictTranscode) ) < 0 )
        {
            return -1;
        }
        
        lpcInput += 2;
    	lpcOutput += liBytesToWrite;
    }
    
    return 0;
}

apl_int_t CUTF16ToUTF8::FillReplacementChar(unsigned char** apcOutput)
{   
    if (!this->mbIsStrictTranscode)
    {
        unsigned char*& lpcOutput = *apcOutput;

        apl_int_t liBytesToWrite = WriteUTF8(UNI_REPLACEMENT_CHAR, lpcOutput, this->mbIsStrictTranscode);
        ACL_ASSERT(liBytesToWrite >= 0);

        lpcOutput += liBytesToWrite;
         
        return 0;
    }
    else
    {
        return -1;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///UTF32 to UTF8
CUTF32ToUTF8::CUTF32ToUTF8( ETranscoderFromEndian aeFromEndian, bool abIsStrict )
    : mbIsStrictTranscode(abIsStrict)
    , mfFromTransform2(CBigEndian::Transform2)
    , mfFromTransform4(CBigEndian::Transform4)
{
    if (aeFromEndian == FROM_LITTLE_ENDIAN)
    {
        this->mfFromTransform2 = CLittleEndian::Transform2;
        this->mfFromTransform4 = CLittleEndian::Transform4;
    }
}

apl_size_t CUTF32ToUTF8::CalcNeedSize( apl_size_t auLength )
{
    return (auLength / 4 + 1) * 5 + 5 /*fill replacement char*/ + 1;
}

apl_ssize_t CUTF32ToUTF8::Transcode( unsigned char const** apcInput, apl_size_t auLength, unsigned char** apcOutput )
{
    unsigned char const*& lpcInput  = *apcInput;
    unsigned char const*  lpcLast   = lpcInput + auLength;
    unsigned char*&       lpcOutput = *apcOutput;
    unsigned char         lacCode[4];
    apl_uint32_t          luUnicode = 0;
    apl_int_t             liBytesToWrite = 0;
    
    while(lpcInput < lpcLast)
    {
        if (lpcLast - lpcInput < 4)
        {
            return 4;
        }
        
        (*this->mfFromTransform4)(lpcInput[0], lpcInput[1], lpcInput[2], lpcInput[3], lacCode);
        
        luUnicode = lacCode[0] << 24 | lacCode[1] << 16 | lacCode[2] << 8 | lacCode[3];
        
        if (luUnicode >= UNI_SUR_HIGH_START && luUnicode <= UNI_SUR_LOW_END)
        {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            if (!this->mbIsStrictTranscode)
            {
                luUnicode = UNI_REPLACEMENT_CHAR;
            }
            else
            {
                return -1;
            }
        }

        if ( (liBytesToWrite = WriteUTF8(luUnicode, lpcOutput, this->mbIsStrictTranscode) ) < 0 )
        {
            return -1;
        }
        
        lpcInput += 4;
    	lpcOutput += liBytesToWrite;
    }
    
    return 0;
}

apl_int_t CUTF32ToUTF8::FillReplacementChar(unsigned char** apcOutput)
{   
    if (!this->mbIsStrictTranscode)
    {
        unsigned char*& lpcOutput = *apcOutput;

        apl_int_t liBytesToWrite = WriteUTF8(UNI_REPLACEMENT_CHAR, lpcOutput, this->mbIsStrictTranscode);
        ACL_ASSERT(liBytesToWrite >= 0);

        lpcOutput += liBytesToWrite;
        
        return 0;
    }
    else
    {
        return -1;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///UTF8 to UTF16
CUTF8ToUTF16::CUTF8ToUTF16( ETranscoderToEndian aeToEndian, bool abIsStrict )
    : mbIsStrictTranscode(abIsStrict)
    , mfToTransform2(CBigEndian::Transform2)
    , mfToTransform4(CBigEndian::Transform4)
{
    if (aeToEndian == TO_LITTLE_ENDIAN)
    {
        this->mfToTransform2 = CLittleEndian::Transform2;
        this->mfToTransform4 = CLittleEndian::Transform4;
    }
}

apl_size_t CUTF8ToUTF16::CalcNeedSize( apl_size_t auLength )
{
    return auLength * 2 + 2/*fill replacement char*/ + 1;
}

apl_ssize_t CUTF8ToUTF16::Transcode( unsigned char const** apcInput, apl_size_t auLength, unsigned char** apcOutput )
{
    unsigned char const*& lpcInput  = *apcInput;
    unsigned char const*  lpcLast   = lpcInput + auLength;
    unsigned char*&       lpcOutput = *apcOutput;
    apl_uint32_t          luUnicode = 0;
    
    while(lpcInput < lpcLast)
    {
        apl_int_t liResult = ReadUTF8(&lpcInput, lpcLast, &luUnicode, this->mbIsStrictTranscode);
        if (liResult < 0)
        {
            return -1;
        }
        else if (liResult > 0)
        {
            return liResult;
        }

        if (luUnicode <= UNI_MAX_BMP)
        {
            // Target is a character <= 0xFFFF (BMP)
    	    // UTF-16 surrogate values are illegal in UTF-32
    	    if (luUnicode >= UNI_SUR_HIGH_START && luUnicode <= UNI_SUR_LOW_END)
    	    {
                if (this->FillReplacementChar(&lpcOutput) != 0)
                {
                    return -1;
                }
    	    }
    	    else
    	    {
    	        (*this->mfToTransform2)(luUnicode >> 8, luUnicode & 0xFF, lpcOutput);
                lpcOutput += 2;
    	    }
    	}
    	else if (luUnicode > UNI_MAX_UTF16)
    	{
            if (this->FillReplacementChar(&lpcOutput) != 0)
            {
                return -1;
            }
    	}
    	else
    	{
    	    // target is a character in range 0xFFFF - 0x10FFFF.
    	    luUnicode -= HALF_BASE;
    	    apl_uint16_t luHigh = (apl_uint16_t)((luUnicode >> HALF_SHIFT) + UNI_SUR_HIGH_START);
    	    apl_uint16_t luLow  = (apl_uint16_t)((luUnicode & HALF_MASK) + UNI_SUR_LOW_START);
    	    
    	    (*this->mfToTransform2)(luHigh >> 8, luHigh & 0xFF, lpcOutput);
            lpcOutput += 2;
            
            (*this->mfToTransform2)(luLow >> 8, luLow & 0xFF, lpcOutput);
            lpcOutput += 2;
    	}
    }
    
    return 0;
}

apl_int_t CUTF8ToUTF16::FillReplacementChar(unsigned char** apcOutput)
{
    if (!this->mbIsStrictTranscode)
    {
        unsigned char*& lpcOutput = *apcOutput;
        
        (*this->mfToTransform2)(
            (UNI_REPLACEMENT_CHAR >> 8) & 0xFF,
            (UNI_REPLACEMENT_CHAR) & 0xFF,
            lpcOutput );
            
        lpcOutput += 2;
        
        return 0;
    }
    else
    {
        return -1;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///UTF8 to UTF32
CUTF8ToUTF32::CUTF8ToUTF32( ETranscoderToEndian aeToEndian, bool abIsStrict )
    : mbIsStrictTranscode(abIsStrict)
    , mfToTransform2(CBigEndian::Transform2)
    , mfToTransform4(CBigEndian::Transform4)
{
    if (aeToEndian == TO_LITTLE_ENDIAN)
    {
        this->mfToTransform2 = CLittleEndian::Transform2;
        this->mfToTransform4 = CLittleEndian::Transform4;
    }
}

apl_size_t CUTF8ToUTF32::CalcNeedSize( apl_size_t auLength )
{
    return auLength * 4 + 4/*fill replacement char*/ + 1;
}

apl_ssize_t CUTF8ToUTF32::Transcode( unsigned char const** apcInput, apl_size_t auLength, unsigned char** apcOutput )
{
    unsigned char const*& lpcInput  = *apcInput;
    unsigned char const*  lpcLast   = lpcInput + auLength;
    unsigned char*&       lpcOutput = *apcOutput;
    apl_uint32_t          luUnicode = 0;
    
    while(lpcInput < lpcLast)
    {
        apl_int_t liResult = ReadUTF8(&lpcInput, lpcLast, &luUnicode, this->mbIsStrictTranscode);
        if (liResult < 0)
        {
            return -1;
        }
        else if (liResult > 0)
        {
            return liResult;
        }
    	
    	if (luUnicode <= UNI_MAX_LEGAL_UTF32 )
    	{
    	    /*
    	     * UTF-16 surrogate values are illegal in UTF-32, and anything
    	     * over Plane 17 (> 0x10FFFF) is illegal.
    	     */
    	    if (luUnicode >= UNI_SUR_HIGH_START && luUnicode <= UNI_SUR_LOW_END)
    	    {
                // i.e., ch > UNI_MAX_LEGAL_UTF32
                if (this->FillReplacementChar(&lpcOutput) != 0)
                {
                    return -1;
                }
    	    }
    	    else
    	    {
    	        (*this->mfToTransform4)(
    	            (luUnicode >> 24) & 0xFF,
                    (luUnicode >> 16) & 0xFF,
                    (luUnicode >> 8) & 0xFF,
                    (luUnicode) & 0xFF,
                    lpcOutput );
                lpcOutput += 4;
    	    }
    	}
    	else
    	{ 
    	    // i.e., ch > UNI_MAX_LEGAL_UTF32
            if (this->FillReplacementChar(&lpcOutput) != 0)
            {
                return -1;
            }
    	}
    }
    
    return 0;
}

apl_int_t CUTF8ToUTF32::FillReplacementChar(unsigned char** apcOutput)
{
    if (!this->mbIsStrictTranscode)
    {
        unsigned char*& lpcOutput = *apcOutput;
        
        (*this->mfToTransform4)(
            (UNI_REPLACEMENT_CHAR >> 24) & 0xFF,
            (UNI_REPLACEMENT_CHAR >> 16) & 0xFF,
            (UNI_REPLACEMENT_CHAR >> 8) & 0xFF,
            (UNI_REPLACEMENT_CHAR) & 0xFF,
            lpcOutput );
            
        lpcOutput += 4;
        
        return 0;
    }
    else
    {
        return -1;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
CGBKToUTF16::CGBKToUTF16( ETranscoderToEndian aeToEndian, bool abIsStrict )
    : mbIsStrictTranscode(abIsStrict)
    , mfToTransform2(CBigEndian::Transform2)
    , mfToTransform4(CBigEndian::Transform4)
{
    if (aeToEndian == TO_LITTLE_ENDIAN)
    {
        this->mfToTransform2 = CLittleEndian::Transform2;
        this->mfToTransform4 = CLittleEndian::Transform4;
    }
}

apl_size_t CGBKToUTF16::CalcNeedSize( apl_size_t auLength )
{
    return auLength * 2 + 1;
}
    
apl_ssize_t CGBKToUTF16::Transcode( unsigned char const** apcInput, apl_size_t auLength, unsigned char** apcOutput )
{
    apl_int_t liRetCode = 0;
    unsigned char const*& lpcInput  = *apcInput;
    unsigned char const*  lpcLast   = lpcInput + auLength;
    unsigned char*&       lpcOutput = *apcOutput;
    apl_uint32_t luUnicode = 0;

    while(lpcInput < lpcLast)
    {
        liRetCode = ces_gbk_mbtowc(&luUnicode, lpcInput, lpcLast - lpcInput);
        
        if(liRetCode > 0)
        {
            (*this->mfToTransform2)( (luUnicode >> 8) & 0xFF, (luUnicode) & 0xFF, lpcOutput );
            lpcOutput += 2;
            lpcInput += liRetCode;
        }
        else if (liRetCode == -2 && lpcLast - lpcInput < 2)
        {
            return 2;
        }
        else
        {
            if (this->FillReplacementChar(&lpcOutput) != 0)
            {
                return -1;
            }
            
            lpcInput += 2;//*lpcInput > 0x80
        }
    } 
    
    return 0;
}

apl_int_t CGBKToUTF16::FillReplacementChar(unsigned char** apcOutput)
{
    if (!this->mbIsStrictTranscode)
    {
        unsigned char*& lpcOutput = *apcOutput;
        
        //UNI_REPLACEMENT_CHAR = 0x0000FFFD;
        (*this->mfToTransform2)(
            (UNI_REPLACEMENT_CHAR >> 8) & 0xFF,
            (UNI_REPLACEMENT_CHAR) & 0xFF,
            lpcOutput );
        lpcOutput += 2;
        
        return 0;
    }
    else
    {
        return -1;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
CUTF16ToGBK::CUTF16ToGBK( ETranscoderFromEndian aeFromEndian, bool abIsStrict )
    : mbIsStrictTranscode(abIsStrict)
    , mfFromTransform2(CBigEndian::Transform2)
    , mfFromTransform4(CBigEndian::Transform4)
{
    if (aeFromEndian == FROM_LITTLE_ENDIAN)
    {
        this->mfFromTransform2 = CLittleEndian::Transform2;
        this->mfFromTransform4 = CLittleEndian::Transform4;
    }
}

apl_size_t CUTF16ToGBK::CalcNeedSize( apl_size_t auLength )
{
    return auLength + 1;
}
    
apl_ssize_t CUTF16ToGBK::Transcode( unsigned char const** apcInput, apl_size_t auLength, unsigned char** apcOutput )
{
    apl_int_t liRetCode = 0;
    unsigned char const*& lpcInput  = *apcInput;
    unsigned char const*  lpcLast   = lpcInput + auLength;
    unsigned char*&       lpcOutput = *apcOutput;
    unsigned char         lacCode[2];
    apl_uint32_t luUnicode = 0;

    while(lpcInput < lpcLast)
    {
        if (lpcLast - lpcInput < 2)
        {
            return 2;
        }
        
        (*this->mfFromTransform2)(lpcInput[0], lpcInput[1], lacCode);
        
        luUnicode = (lacCode[0] << 8) | lacCode[1];
        
        liRetCode = ces_gbk_wctomb(lpcOutput, luUnicode, 2);
        if(liRetCode > 0)
        {
            lpcOutput += liRetCode;
            lpcInput += 2;
        }
        else
        {
            if (this->FillReplacementChar(&lpcOutput) != 0)
            {
                return -1;
            }
            
            lpcInput += 2;//*lpcInput > 0x80
        }
    } 
    
    return 0;
}

apl_int_t CUTF16ToGBK::FillReplacementChar(unsigned char** apcOutput)
{
    if (!this->mbIsStrictTranscode)
    {
        unsigned char*& lpcOutput = *apcOutput;

        apl_int_t liRetCode = ces_gbk_wctomb(lpcOutput, UNI_REPLACEMENT_CHAR, 2);
        ACL_ASSERT(liRetCode > 0);

        lpcOutput += liRetCode;

        return 0;
    }
    else
    {
        return -1;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
CGBKToUTF8::CGBKToUTF8( bool abIsStrict )
    : mbIsStrictTranscode(abIsStrict)
{
}

apl_size_t CGBKToUTF8::CalcNeedSize( apl_size_t auLength )
{
    return (auLength / 2 + 1) * 5 + 5/*fill replacement char*/ + 1;
}
    
apl_ssize_t CGBKToUTF8::Transcode( unsigned char const** apcInput, apl_size_t auLength, unsigned char** apcOutput )
{
    apl_int_t liRetCode = 0;
    unsigned char const*& lpcInput  = *apcInput;
    unsigned char const*  lpcLast   = lpcInput + auLength;
    unsigned char*&       lpcOutput = *apcOutput;
    apl_uint32_t luUnicode = 0;
    apl_int_t    liBytesToWrite = 0;

    while(lpcInput < lpcLast)
    {
        liRetCode = ces_gbk_mbtowc(&luUnicode, lpcInput, lpcLast - lpcInput);
        
        if(liRetCode > 0)
        {
            if ( (liBytesToWrite = WriteUTF8(luUnicode, lpcOutput, this->mbIsStrictTranscode) ) < 0)
            {
                return -1;
            }

            lpcOutput += liBytesToWrite;
            lpcInput += liRetCode;
        }
        else if (liRetCode == -2 && lpcLast - lpcInput < 2)
        {
            return 2;
        }
        else
        {
            if (this->FillReplacementChar(&lpcOutput) != 0)
            {
                return -1;
            }
            
            lpcInput += 2;//*lpcInput > 0x80
        }
    } 
    
    return 0;
}

apl_int_t CGBKToUTF8::FillReplacementChar(unsigned char** apcOutput)
{
    if (!this->mbIsStrictTranscode)
    {
        unsigned char*& lpcOutput = *apcOutput;

        apl_int_t liBytesToWrite = WriteUTF8(UNI_REPLACEMENT_CHAR, lpcOutput, this->mbIsStrictTranscode);
        ACL_ASSERT(liBytesToWrite >= 0);

        lpcOutput += liBytesToWrite;

        return 0;
    }
    else
    {
        return -1;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
CUTF8ToGBK::CUTF8ToGBK( bool abIsStrict )
    : mbIsStrictTranscode(abIsStrict)
{
}

apl_size_t CUTF8ToGBK::CalcNeedSize( apl_size_t auLength )
{
    return auLength + 1;
}
    
apl_ssize_t CUTF8ToGBK::Transcode( unsigned char const** apcInput, apl_size_t auLength, unsigned char** apcOutput )
{
    apl_int_t liRetCode = 0;
    unsigned char const*& lpcInput  = *apcInput;
    unsigned char const*  lpcLast   = lpcInput + auLength;
    unsigned char*&       lpcOutput = *apcOutput;
    apl_uint32_t luUnicode = 0;

    while(lpcInput < lpcLast)
    {
        liRetCode = ReadUTF8(&lpcInput, lpcLast, &luUnicode, this->mbIsStrictTranscode);
        if (liRetCode < 0)
        {
            return -1;
        }
        else if (liRetCode > 0)
        {
            return liRetCode;
        }
        
        liRetCode = ces_gbk_wctomb(lpcOutput, luUnicode, 2);
        if(liRetCode > 0)
        {
            lpcOutput += liRetCode;
            lpcInput += 2;
        }
        else
        {
            if (this->FillReplacementChar(&lpcOutput) != 0)
            {
                return -1;
            }
            
            lpcInput += 2;//*lpcInput > 0x80
        }
    } 
    
    return 0;
}

apl_int_t CUTF8ToGBK::FillReplacementChar(unsigned char** apcOutput)
{
    if (!this->mbIsStrictTranscode)
    {
        unsigned char*& lpcOutput = *apcOutput;

        apl_int_t liRetCode = ces_gbk_wctomb(lpcOutput, UNI_REPLACEMENT_CHAR, 2);
        ACL_ASSERT(liRetCode > 0);
        
        lpcOutput += liRetCode;

        return 0;
    }
    else
    {
        return -1;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CBIG5ToUTF16::CBIG5ToUTF16( ETranscoderToEndian aeToEndian, bool abIsStrict )
    : mbIsStrictTranscode(abIsStrict)
    , mfToTransform2(CBigEndian::Transform2)
    , mfToTransform4(CBigEndian::Transform4)
{
    if (aeToEndian == TO_LITTLE_ENDIAN)
    {
        this->mfToTransform2 = CLittleEndian::Transform2;
        this->mfToTransform4 = CLittleEndian::Transform4;
    }
}

apl_size_t CBIG5ToUTF16::CalcNeedSize( apl_size_t auLength )
{
    return auLength * 2 + 1;
}
    
apl_ssize_t CBIG5ToUTF16::Transcode( unsigned char const** apcInput, apl_size_t auLength, unsigned char** apcOutput )
{
    apl_int_t liRetCode = 0;
    unsigned char const*& lpcInput  = *apcInput;
    unsigned char const*  lpcLast   = lpcInput + auLength;
    unsigned char*&       lpcOutput = *apcOutput;
    apl_uint32_t luUnicode = 0;

    while(lpcInput < lpcLast)
    {
        liRetCode = ces_big5_mbtowc(&luUnicode, lpcInput, lpcLast - lpcInput);
        
        if(liRetCode > 0)
        {
            (*this->mfToTransform2)( (luUnicode >> 8) & 0xFF, (luUnicode) & 0xFF, lpcOutput );
            lpcOutput += 2;
            lpcInput += liRetCode;
        }
        else if (liRetCode == -2 && lpcLast - lpcInput < 2)
        {
            return 2;
        }
        else
        {
            if (this->FillReplacementChar(&lpcOutput) != 0)
            {
                return -1;
            }
            
            lpcInput += 2;//*lpcInput > 0x80
        }
    } 
    
    return 0;
}

apl_int_t CBIG5ToUTF16::FillReplacementChar(unsigned char** apcOutput)
{
    if (!this->mbIsStrictTranscode)
    {
        unsigned char*& lpcOutput = *apcOutput;

        (*this->mfToTransform2)( (UNI_REPLACEMENT_CHAR >> 8) & 0xFF, (UNI_REPLACEMENT_CHAR) & 0xFF, lpcOutput);
        lpcOutput += 2;

        return 0;
    }
    else
    {
        return -1;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
CUTF16ToBIG5::CUTF16ToBIG5( ETranscoderFromEndian aeFromEndian, bool abIsStrict )
    : mbIsStrictTranscode(abIsStrict)
    , mfFromTransform2(CBigEndian::Transform2)
    , mfFromTransform4(CBigEndian::Transform4)
{
    if (aeFromEndian == FROM_LITTLE_ENDIAN)
    {
        this->mfFromTransform2 = CLittleEndian::Transform2;
        this->mfFromTransform4 = CLittleEndian::Transform4;
    }
}

apl_size_t CUTF16ToBIG5::CalcNeedSize( apl_size_t auLength )
{
    return auLength + 1;
}
    
apl_ssize_t CUTF16ToBIG5::Transcode( unsigned char const** apcInput, apl_size_t auLength, unsigned char** apcOutput )
{
    apl_int_t liRetCode = 0;
    unsigned char const*& lpcInput  = *apcInput;
    unsigned char const*  lpcLast   = lpcInput + auLength;
    unsigned char*&       lpcOutput = *apcOutput;
    unsigned char         lacCode[2];
    apl_uint32_t luUnicode = 0;

    while(lpcInput < lpcLast)
    {
        if (lpcLast - lpcInput < 2)
        {
            return 2;
        }
        
        (*this->mfFromTransform2)(lpcInput[0], lpcInput[1], lacCode);
        
        luUnicode = (lacCode[0] << 8) | lacCode[1];
        
        liRetCode = ces_big5_wctomb(lpcOutput, luUnicode, 2);
        if(liRetCode > 0)
        {
            lpcOutput += liRetCode;
            lpcInput += 2;
        }
        else
        {
            if (this->FillReplacementChar(&lpcOutput) != 0)
            {
                return -1;
            }
            
            lpcInput += 2;//*lpcInput > 0x80
        }
    } 
    
    return 0;
}

inline apl_int_t CUTF16ToBIG5::FillReplacementChar(unsigned char** apcOutput)
{
    if (!this->mbIsStrictTranscode)
    {
        unsigned char*& lpcOutput = *apcOutput;
        
        apl_int_t liRetCode = ces_big5_wctomb(lpcOutput, UNI_REPLACEMENT_CHAR, 2);
        ACL_ASSERT(liRetCode > 0);

        lpcOutput += liRetCode;

        return 0;
    }
    else
    {
        return -1;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CBIG5ToUTF8::CBIG5ToUTF8( bool abIsStrict )
    : mbIsStrictTranscode(abIsStrict)
{
}

apl_size_t CBIG5ToUTF8::CalcNeedSize( apl_size_t auLength )
{
    return (auLength / 2 + 1) * 5 + 5/*fill replacement char*/ + 1;
}
    
apl_ssize_t CBIG5ToUTF8::Transcode( unsigned char const** apcInput, apl_size_t auLength, unsigned char** apcOutput )
{
    apl_int_t liRetCode = 0;
    unsigned char const*& lpcInput  = *apcInput;
    unsigned char const*  lpcLast   = lpcInput + auLength;
    unsigned char*&       lpcOutput = *apcOutput;
    apl_uint32_t luUnicode = 0;
    apl_int_t    liBytesToWrite = 0;

    while(lpcInput < lpcLast)
    {
        liRetCode = ces_big5_mbtowc(&luUnicode, lpcInput, lpcLast - lpcInput);
        
        if(liRetCode > 0)
        {
            if ( (liBytesToWrite = WriteUTF8(luUnicode, lpcOutput, this->mbIsStrictTranscode) ) < 0)
            {
                return -1;
            }

            lpcOutput += liBytesToWrite;
            lpcInput += liRetCode;
        }
        else if (liRetCode == -2 && lpcLast - lpcInput < 2)
        {
            return 2;
        }
        else
        {
            if (this->FillReplacementChar(&lpcOutput) != 0)
            {
                return -1;
            }
            
            lpcInput += 2;//*lpcInput > 0x80
        }
    } 
    
    return 0;
}

apl_int_t CBIG5ToUTF8::FillReplacementChar(unsigned char** apcOutput)
{
    if (!this->mbIsStrictTranscode)
    {
        unsigned char*& lpcOutput = *apcOutput;

        apl_int_t liBytesToWrite = WriteUTF8(UNI_REPLACEMENT_CHAR, lpcOutput, this->mbIsStrictTranscode);
        ACL_ASSERT(liBytesToWrite >= 0);

        lpcOutput += liBytesToWrite;

        return 0;
    }
    else
    {
        return -1;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
CUTF8ToBIG5::CUTF8ToBIG5( bool abIsStrict )
    : mbIsStrictTranscode(abIsStrict)
{
}

apl_size_t CUTF8ToBIG5::CalcNeedSize( apl_size_t auLength )
{
    return auLength + 1;
}
    
apl_ssize_t CUTF8ToBIG5::Transcode( unsigned char const** apcInput, apl_size_t auLength, unsigned char** apcOutput )
{
    apl_int_t liRetCode = 0;
    unsigned char const*& lpcInput  = *apcInput;
    unsigned char const*  lpcLast   = lpcInput + auLength;
    unsigned char*&       lpcOutput = *apcOutput;
    unsigned char         lacCode[2];
    apl_uint32_t luUnicode = 0;

    while(lpcInput < lpcLast)
    {
        liRetCode = ReadUTF8(&lpcInput, lpcLast, &luUnicode, this->mbIsStrictTranscode);
        if (liRetCode < 0)
        {
            return -1;
        }
        else if (liRetCode > 0)
        {
            return liRetCode;
        }
        
        liRetCode = ces_big5_wctomb(lpcOutput, luUnicode, 2);
        if(liRetCode > 0)
        {
            lpcOutput += liRetCode;
            lpcInput += 2;
        }
        else
        {
            if (this->FillReplacementChar(&lpcOutput) != 0)
            {
                return -1;
            }
            
            lpcInput += 2;//*lpcInput > 0x80
        }
    } 
    
    return 0;
}

inline apl_int_t CUTF8ToBIG5::FillReplacementChar(unsigned char** apcOutput)
{
    if (!this->mbIsStrictTranscode)
    {
        unsigned char*& lpcOutput = *apcOutput;
        
        apl_int_t liRetCode = ces_big5_wctomb(lpcOutput, UNI_REPLACEMENT_CHAR, 2);
        ACL_ASSERT(liRetCode > 0);

        lpcOutput += liRetCode;

        return 0;
    }
    else
    {
        return -1;
    }
}

ACL_NAMESPACE_END
