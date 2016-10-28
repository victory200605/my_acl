
#ifndef ACL_CODECS_UNICODE_H
#define ACL_CODECS_UNICODE_H

#include "acl/MemoryBlock.h"

ACL_NAMESPACE_START

enum ETranscoderFromEndian
{
    FROM_BIG_ENDIAN = 0,
    FROM_LITTLE_ENDIAN = 1
};

enum ETranscoderToEndian
{
    TO_BIG_ENDIAN = 0,
    TO_LITTLE_ENDIAN = 1
};

///////////////////////////////////////////////////////////////////////////////////////////////////////
//Unicode Transcoder
template<typename TranscoderType>
class TUnicodeTranscoder
{
public:
    /**
     * Default constuctor
     */
    TUnicodeTranscoder(void);
    
    /**
     * Constuctor
     * Accept user defined buffer, if buffer is no enough and Transcoder shall resize it,
     * and the buffer discard
     *
     * @param [out] apcBuffer default buffer pointer
     * @param [in]  auSize default buffer size
     */
    TUnicodeTranscoder( char* apcBuffer, apl_size_t auSize );

    /**
     * Constuctor
     *
     * @a aoMB.GetDataBlock()->Duplicate() function will be called to allocating a CDataBlock for Transcoder's moBuffer
     * and @a aoMB will not be modified never.
     *
     * @param [in] aoMB buffer
     */
    TUnicodeTranscoder( CMemoryBlock const& aoMB );

    /**
     * Constuctor
     *
     * if @aIsStrict is true and transcoder shall return -1 when invalid data happened, 
     * otherwise REPLACEMENT char will replace it and ignore the error
     *
     * @param [in] aIsStrict default buffer size
     */
    TUnicodeTranscoder( bool aIsStrict );
    
    /**
     * Constuctor
     * if @aIsStrict is true and transcoder shall return -1 when invalid data happened, 
     * otherwise REPLACEMENT char will replace it and ignore the error
     *
     * Accept user defined buffer, if buffer is no enough and Transcoder shall resize it,
     * and the buffer discard
     *
     * @param [out] apcBuffer default buffer pointer
     * @param [in]  auSize default buffer size
     * @param [in] aIsStrict default buffer size
     */
    TUnicodeTranscoder( char* apcBuffer, apl_size_t auSize, bool aIsStrict );

    /**
     * Constuctor
     * if @aIsStrict is true and transcoder shall return -1 when invalid data happened, 
     * otherwise REPLACEMENT char will replace it and ignore the error
     *
     * @a aoMB.GetDataBlock()->Duplicate() function will be called to allocating a CDataBlock for Transcoder's moBuffer
     * and @a aoMB will not be modified never.
     *
     * @param [in] aoMB buffer
     * @param [in] aIsStrict default buffer size
     */
    TUnicodeTranscoder( CMemoryBlock const& aoMB, bool aIsStrict );

    /**
     * Constuctor
     *
     * @param [in]  aeFromEndian input data is big-endian or little-endian
     */
    TUnicodeTranscoder( ETranscoderFromEndian aeFromEndian, bool aIsStrict );
    
    /**
     * Constuctor
     * if @aIsStrict is true and transcoder shall return -1 when invalid data happened, 
     * otherwise REPLACEMENT char will replace it and ignore the error
     *
     * Accept user defined buffer, if buffer is no enough and Transcoder shall resize it,
     * and the buffer discard
     *
     * @param [out] apcBuffer default buffer pointer
     * @param [in]  auSize default buffer size
     * @param [in]  aeFromEndian input data is big-endian or little-endian
     * @param [in] aIsStrict default buffer size
     */
    TUnicodeTranscoder( char* apcBuffer, apl_size_t auSize, ETranscoderFromEndian aeFromEndian, bool aIsStrict );

    /**
     * Constuctor
     * if @aIsStrict is true and transcoder shall return -1 when invalid data happened, 
     * otherwise REPLACEMENT char will replace it and ignore the error
     *
     * @a aoMB.GetDataBlock()->Duplicate() function will be called to allocating a CDataBlock for Transcoder's moBuffer
     * and @a aoMB will not be modified never.
     *
     * @param [in] aoMB buffer
     * @param [in]  aeFromEndian input data is big-endian or little-endian
     * @param [in] aIsStrict default buffer size
     */
    TUnicodeTranscoder( CMemoryBlock const& aoMB, ETranscoderFromEndian aeFromEndian, bool aIsStrict );

    /**
     * Constuctor
     *
     * @param [in]  aeToEndian output data is big-endian or little-endian
     */
    TUnicodeTranscoder( ETranscoderToEndian aeToEndian, bool aIsStrict );
    
    /**
     * Constuctor
     * if @aIsStrict is true and transcoder shall return -1 when invalid data happened, 
     * otherwise REPLACEMENT char will replace it and ignore the error
     *
     * Accept user defined buffer, if buffer is no enough and Transcoder shall resize it,
     * and the buffer discard
     *
     * @param [out] apcBuffer default buffer pointer
     * @param [in]  auSize default buffer size
     * @param [in]  aeToEndian output data is big-endian or little-endian
     * @param [in] aIsStrict default buffer size
     */
    TUnicodeTranscoder( char* apcBuffer, apl_size_t auSize, ETranscoderToEndian aeToEndian, bool aIsStrict );

    /**
     * Constuctor
     * if @aIsStrict is true and transcoder shall return -1 when invalid data happened, 
     * otherwise REPLACEMENT char will replace it and ignore the error
     *
     * @a aoMB.GetDataBlock()->Duplicate() function will be called to allocating a CDataBlock for Transcoder's moBuffer
     * and @a aoMB will not be modified never.
     *
     * @param [in] aoMB buffer
     * @param [in]  aeToEndian output data is big-endian or little-endian
     * @param [in] aIsStrict default buffer size
     */
    TUnicodeTranscoder( CMemoryBlock const& aoMB, ETranscoderToEndian aeToEndian, bool aIsStrict );

    /**
     * Constuctor
     */
    TUnicodeTranscoder( ETranscoderFromEndian aeFromEndian, ETranscoderToEndian aeToEndian, bool aIsStrict );
    
    /**
     * Constuctor
     * if @aIsStrict is true and transcoder shall return -1 when invalid data happened, 
     * otherwise REPLACEMENT char will replace it and ignore the error
     *
     * Accept user defined buffer, if buffer is no enough and Transcoder shall resize it,
     * and the buffer discard
     *
     * @param [out] apcBuffer default buffer pointer
     * @param [in]  auSize default buffer size
     * @param [in]  aeFromEndian input data is big-endian or little-endian
     * @param [in]  aeToEndian output data is big-endian or little-endian
     * @param [in] aIsStrict default buffer size
     */
    TUnicodeTranscoder( 
        char* apcBuffer, 
        apl_size_t auSize, 
        ETranscoderFromEndian aeFromEndian, 
        ETranscoderToEndian aeToEndian, 
        bool aIsStrict );

    /**
     * Constuctor
     * if @aIsStrict is true and transcoder shall return -1 when invalid data happened, 
     * otherwise REPLACEMENT char will replace it and ignore the error
     *
     * @a aoMB.GetDataBlock()->Duplicate() function will be called to allocating a CDataBlock for Transcoder's moBuffer
     * and @a aoMB will not be modified never.
     *
     * @param [in] aoMB buffer
     * @param [in]  aeFromEndian input data is big-endian or little-endian
     * @param [in]  aeToEndian output data is big-endian or little-endian
     * @param [in] aIsStrict default buffer size
     */
    TUnicodeTranscoder( 
        CMemoryBlock const& aoMB, 
        ETranscoderFromEndian aeFromEndian, 
        ETranscoderToEndian aeToEndian, 
        bool aIsStrict );

    /**
     * UTF8 block update operation. Continues an UTF8 transcode
     * operation, processing another message block, and updating the context.
     *
     * @param [in] apcInput Unicode data in stream.
     * @param [out] apuOutputLen Length of unicode data byte stream.
     *
     * @return Upon successful completion, shall return 0; otherwise, it shall return -1
     */
    apl_int_t Update( void const* apvInput, apl_size_t auInputLen );

    /**
     * UTF8 finalization. zeroizing the context.
     *
     * @return Upon successful completion, shall return 0; otherwise, it shall return -1
     */
    apl_int_t Final(void);
    
    /**
     * UTF8 Update and Final operation.
     *
     * @param [in] apvInput Unicode data in stream.
     * @param [in] auInputLen unicode data byte stream.
     * @return Upon successful completion, shall return 0; otherwise, it shall return -1
     */
    apl_int_t Final( void const* apvInput, apl_size_t auInputLen );
    
    /**
     * @return UTF8 output context.
     */
    char const* GetOutput(void);
    
    /**
     * @return UTF8 context length.
     */
    apl_size_t GetLength(void);
    
    /**
     * Reset, zeroizing the context.
     */
    void Reset(void);

private:
    CMemoryBlock moBuffer;

    apl_size_t muLength;
    
    unsigned char macGroup[8];
    
    apl_size_t muCount;
    
    apl_size_t muNeedCount;
    
    TranscoderType moTranscoder;
};

///implement
/////////////////////////////////////////////////////////////////////////////////////////////////
template<typename TranscoderType>
TUnicodeTranscoder<TranscoderType>::TUnicodeTranscoder(void)
    : moBuffer(NULL, 0, acl::CMemoryBlock::DONT_DELETE)
    , muLength(0)
    , muCount(0)
    , muNeedCount(0)
{
}

template<typename TranscoderType>
TUnicodeTranscoder<TranscoderType>::TUnicodeTranscoder( char* apcBuffer, apl_size_t auSize )
    : moBuffer(apcBuffer, auSize, acl::CMemoryBlock::DONT_DELETE)
    , muLength(0)
    , muCount(0)
    , muNeedCount(0)
{
}

template<typename TranscoderType>
TUnicodeTranscoder<TranscoderType>::TUnicodeTranscoder( CMemoryBlock const& aoMB )
    : moBuffer(aoMB.GetDataBlock()->Duplicate() )
    , muLength(0)
    , muCount(0)
    , muNeedCount(0)
{
}

template<typename TranscoderType>
TUnicodeTranscoder<TranscoderType>::TUnicodeTranscoder( bool aIsStrict )
    : moBuffer(NULL, 0, acl::CMemoryBlock::DONT_DELETE)
    , muLength(0)
    , muCount(0)
    , muNeedCount(0)
    , moTranscoder(aIsStrict)
{
}

template<typename TranscoderType>
TUnicodeTranscoder<TranscoderType>::TUnicodeTranscoder( char* apcBuffer, apl_size_t auSize, bool aIsStrict )
    : moBuffer(apcBuffer, auSize, acl::CMemoryBlock::DONT_DELETE)
    , muLength(0)
    , muCount(0)
    , muNeedCount(0)
    , moTranscoder(aIsStrict)
{
}

template<typename TranscoderType>
TUnicodeTranscoder<TranscoderType>::TUnicodeTranscoder( CMemoryBlock const& aoMB, bool aIsStrict )
    : moBuffer(aoMB.GetDataBlock()->Duplicate() )
    , muLength(0)
    , muCount(0)
    , muNeedCount(0)
    , moTranscoder(aIsStrict)
{
}

template<typename TranscoderType>
TUnicodeTranscoder<TranscoderType>::TUnicodeTranscoder( ETranscoderFromEndian aeFromEndian, bool aIsStrict )
    : moBuffer(NULL, 0, acl::CMemoryBlock::DONT_DELETE)
    , muLength(0)
    , muCount(0)
    , muNeedCount(0)
    , moTranscoder(aeFromEndian, aIsStrict)
{
}

template<typename TranscoderType>
TUnicodeTranscoder<TranscoderType>::TUnicodeTranscoder( 
    char* apcBuffer, 
    apl_size_t auSize,
    ETranscoderFromEndian aeFromEndian, 
    bool aIsStrict )
    : moBuffer(apcBuffer, auSize, acl::CMemoryBlock::DONT_DELETE)
    , muLength(0)
    , muCount(0)
    , muNeedCount(0)
    , moTranscoder(aeFromEndian, aIsStrict)
{
}

template<typename TranscoderType>
TUnicodeTranscoder<TranscoderType>::TUnicodeTranscoder( 
    CMemoryBlock const& aoMB,
    ETranscoderFromEndian aeFromEndian, 
    bool aIsStrict )
    : moBuffer(aoMB.GetDataBlock()->Duplicate() )
    , muLength(0)
    , muCount(0)
    , muNeedCount(0)
    , moTranscoder(aeFromEndian, aIsStrict)
{
}

template<typename TranscoderType>
TUnicodeTranscoder<TranscoderType>::TUnicodeTranscoder( ETranscoderToEndian aeToEndian, bool aIsStrict )
    : moBuffer(NULL, 0, acl::CMemoryBlock::DONT_DELETE)
    , muLength(0)
    , muCount(0)
    , muNeedCount(0)
    , moTranscoder(aeToEndian, aIsStrict)
{
}

template<typename TranscoderType>
TUnicodeTranscoder<TranscoderType>::TUnicodeTranscoder( char* apcBuffer, apl_size_t auSize, ETranscoderToEndian aeToEndian, bool aIsStrict )
    : moBuffer(apcBuffer, auSize, acl::CMemoryBlock::DONT_DELETE)
    , muLength(0)
    , muCount(0)
    , muNeedCount(0)
    , moTranscoder(aeToEndian, aIsStrict)
{
}

template<typename TranscoderType>
TUnicodeTranscoder<TranscoderType>::TUnicodeTranscoder( CMemoryBlock const& aoMB, ETranscoderToEndian aeToEndian, bool aIsStrict )
    : moBuffer(aoMB.GetDataBlock()->Duplicate() )
    , muLength(0)
    , muCount(0)
    , muNeedCount(0)
    , moTranscoder(aeToEndian, aIsStrict)
{
}

template<typename TranscoderType>
TUnicodeTranscoder<TranscoderType>::TUnicodeTranscoder( ETranscoderFromEndian aeFromEndian, ETranscoderToEndian aeToEndian, bool aIsStrict )
    : moBuffer(NULL, 0, acl::CMemoryBlock::DONT_DELETE)
    , muLength(0)
    , muCount(0)
    , muNeedCount(0)
    , moTranscoder(aeFromEndian, aeToEndian, aIsStrict)
{
}

template<typename TranscoderType>
TUnicodeTranscoder<TranscoderType>::TUnicodeTranscoder( 
    char* apcBuffer,
    apl_size_t auSize,
    ETranscoderFromEndian aeFromEndian, 
    ETranscoderToEndian aeToEndian, 
    bool aIsStrict )
    : moBuffer(apcBuffer, auSize, acl::CMemoryBlock::DONT_DELETE)
    , muLength(0)
    , muCount(0)
    , muNeedCount(0)
    , moTranscoder(aeFromEndian, aeToEndian, aIsStrict)
{
}

template<typename TranscoderType>
TUnicodeTranscoder<TranscoderType>::TUnicodeTranscoder( 
    CMemoryBlock const& aoMB,
    ETranscoderFromEndian aeFromEndian, 
    ETranscoderToEndian aeToEndian, 
    bool aIsStrict )
    : moBuffer(aoMB.GetDataBlock()->Duplicate() )
    , muLength(0)
    , muCount(0)
    , muNeedCount(0)
    , moTranscoder(aeFromEndian, aeToEndian, aIsStrict)
{
}

template<typename TranscoderType>
apl_int_t TUnicodeTranscoder<TranscoderType>::Update( void const* apvInput, apl_size_t auInputLen )
{
    apl_size_t luNeedSize = this->moTranscoder.CalcNeedSize(auInputLen);
    unsigned char const* lpcInput = (unsigned char const*)apvInput;
    unsigned char const* lpcLast  = lpcInput + auInputLen;
    apl_ssize_t    liResult = 0;
    unsigned char* lpcWritePtr = NULL;
    
    if (this->moBuffer.GetSpace() < luNeedSize)
    {
        this->moBuffer.Resize(luNeedSize + this->moBuffer.GetLength() + 1);
    }
    
    lpcWritePtr = (unsigned char*)this->moBuffer.GetWritePtr();
    
    if (this->muCount > 0)
    {
        while(true)
        {
            if (auInputLen >= this->muNeedCount)
            {
                unsigned char const* lpcTemp = this->macGroup;
                apl_memcpy(this->macGroup + this->muCount, lpcInput, this->muNeedCount);
                liResult = this->moTranscoder.Transcode(&lpcTemp, this->muCount + this->muNeedCount,&lpcWritePtr);
                if (liResult < 0)
                {
                    return -1;
                }
                else if (liResult > 0)
                {
                    this->muNeedCount = liResult - this->muCount;
                    continue;
                }
    
                lpcInput += this->muNeedCount;
                this->muCount = 0;
                this->muNeedCount = 0;
                break;
            }
            else
            {
                apl_memcpy(this->macGroup + this->muCount, lpcInput, auInputLen);
                this->muCount += auInputLen;
                this->muNeedCount -= auInputLen;
                return 0;
            }
        }
    }
    
    liResult = this->moTranscoder.Transcode(&lpcInput, lpcLast - lpcInput, &lpcWritePtr);
    if (liResult < 0)
    {
        return -1;
    }
    else if (liResult > 0)
    {
        ACL_ASSERT(liResult < (apl_ssize_t)sizeof(this->macGroup) );
        
        apl_memcpy(this->macGroup, lpcInput, lpcLast - lpcInput);
        this->muCount = (lpcLast - lpcInput);
        this->muNeedCount = liResult - this->muCount;
    }

    this->moBuffer.SetWritePtr((char*)lpcWritePtr);
    
    *lpcWritePtr = '\0';

    return 0;
}

template<typename TranscoderType>
apl_int_t TUnicodeTranscoder<TranscoderType>::Final(void)
{
    if (this->muCount > 0)
    {
        unsigned char* lpcWritePtr = (unsigned char*)this->moBuffer.GetWritePtr();
        if (this->moTranscoder.FillReplacementChar(&lpcWritePtr) != 0)
        {
            return -1;
        }
        else
        {
            this->moBuffer.SetWritePtr((char*)lpcWritePtr);
    
            *lpcWritePtr = '\0';
        }
    }
    
    this->muLength = this->moBuffer.GetLength();
    this->moBuffer.Reset();
    
    return 0;
}

template<typename TranscoderType>
apl_int_t TUnicodeTranscoder<TranscoderType>::Final( void const* apvInput, apl_size_t auInputLen )
{
    if (this->Update(apvInput, auInputLen) != 0)
    {
        return -1;
    }
    
    return this->Final();
}

template<typename TranscoderType>
char const* TUnicodeTranscoder<TranscoderType>::GetOutput(void)
{
    return this->moBuffer.GetBase();
}

template<typename TranscoderType>
apl_size_t TUnicodeTranscoder<TranscoderType>::GetLength(void)
{
    return this->muLength;
}

template<typename TranscoderType>
void TUnicodeTranscoder<TranscoderType>::Reset(void)
{
    this->moBuffer.Reset();
}

typedef void (*Transform2FuncType)(unsigned char ch1, unsigned char ch2, unsigned char* apcOutput);
typedef void (*Transform4FuncType)(unsigned char ch1, unsigned char ch2, unsigned char ch3, unsigned char ch4, unsigned char* apcOutput);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CUTF32ToUTF16
{
public:
    CUTF32ToUTF16( ETranscoderFromEndian aeFromEndian, ETranscoderToEndian aeToEndian, bool abIsStrict );
    
    apl_size_t CalcNeedSize( apl_size_t auLength );
    
    apl_ssize_t Transcode( unsigned char const** apcInput, apl_size_t auLength, unsigned char** apcOutput );
    
    apl_int_t FillReplacementChar( unsigned char** apcOutput );
    
public:
    bool mbIsStrictTranscode;
    
    Transform2FuncType mfFromTransform2;
    
    Transform4FuncType mfFromTransform4;
    
    Transform2FuncType mfToTransform2;
    
    Transform4FuncType mfToTransform4;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CUTF16ToUTF32
{
public:
    CUTF16ToUTF32( ETranscoderFromEndian aeFromEndian, ETranscoderToEndian aeToEndian, bool abIsStrict );
    
    apl_size_t CalcNeedSize( apl_size_t auLength );
    
    apl_ssize_t Transcode( unsigned char const** apcInput, apl_size_t auLength, unsigned char** apcOutput );
    
    apl_int_t FillReplacementChar( unsigned char** apcOutput );
    
public:
    bool mbIsStrictTranscode;
    
    Transform2FuncType mfFromTransform2;
    
    Transform4FuncType mfFromTransform4;
    
    Transform2FuncType mfToTransform2;
    
    Transform4FuncType mfToTransform4;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CUTF16ToUTF8
{
public:
    CUTF16ToUTF8( ETranscoderFromEndian aeFromEndian, bool abIsStrict );
    
    apl_size_t CalcNeedSize( apl_size_t auLength );
    
    apl_ssize_t Transcode( unsigned char const** apcInput, apl_size_t auLength, unsigned char** apcOutput );
    
    apl_int_t FillReplacementChar( unsigned char** apcOutput );
    
public:
    bool mbIsStrictTranscode;
    
    Transform2FuncType mfFromTransform2;
    
    Transform4FuncType mfFromTransform4;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CUTF8ToUTF16
{
public:
    CUTF8ToUTF16( ETranscoderToEndian aeToEndian, bool abIsStrict );
    
    apl_size_t CalcNeedSize( apl_size_t auLength );
    
    apl_ssize_t Transcode( unsigned char const** apcInput, apl_size_t auLength, unsigned char** apcOutput );
    
    apl_int_t FillReplacementChar( unsigned char** apcOutput );
    
public:
    bool mbIsStrictTranscode;

    Transform2FuncType mfToTransform2;
    
    Transform4FuncType mfToTransform4;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CUTF32ToUTF8
{
public:
    CUTF32ToUTF8( ETranscoderFromEndian aeFromEndian, bool abIsStrict );
    
    apl_size_t CalcNeedSize( apl_size_t auLength );
    
    apl_ssize_t Transcode( unsigned char const** apcInput, apl_size_t auLength, unsigned char** apcOutput );
    
    apl_int_t FillReplacementChar( unsigned char** apcOutput );
    
public:
    bool mbIsStrictTranscode;
    
    Transform2FuncType mfFromTransform2;
    
    Transform4FuncType mfFromTransform4;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CUTF8ToUTF32
{
public:
    CUTF8ToUTF32( ETranscoderToEndian aeToEndian, bool abIsStrict );
    
    apl_size_t CalcNeedSize( apl_size_t auLength );
    
    apl_ssize_t Transcode( unsigned char const** apcInput, apl_size_t auLength, unsigned char** apcOutput );
    
    apl_int_t FillReplacementChar( unsigned char** apcOutput );
    
public:
    bool mbIsStrictTranscode;
    
    Transform2FuncType mfToTransform2;
    
    Transform4FuncType mfToTransform4;
};

//////////////////////////////////////////////////////////////////////////////////////////////
class CGBKToUTF16
{
public:
    CGBKToUTF16( ETranscoderToEndian aeToEndian, bool abIsStrict );
    
    apl_size_t CalcNeedSize( apl_size_t auLength );
    
    apl_ssize_t Transcode( unsigned char const** apcInput, apl_size_t auLength, unsigned char** apcOutput );
    
    apl_int_t FillReplacementChar(unsigned char** apcOutput);

public:
    bool mbIsStrictTranscode;

    Transform2FuncType mfToTransform2;
    
    Transform4FuncType mfToTransform4;
};

//////////////////////////////////////////////////////////////////////////////////////////////
class CUTF16ToGBK
{
public:
    CUTF16ToGBK( ETranscoderFromEndian aeFromEndian, bool abIsStrict );
    
    apl_size_t CalcNeedSize( apl_size_t auLength );
    
    apl_ssize_t Transcode( unsigned char const** apcInput, apl_size_t auLength, unsigned char** apcOutput );
    
    apl_int_t FillReplacementChar( unsigned char** apcOutput );
    
public:
    bool mbIsStrictTranscode;
    
    Transform2FuncType mfFromTransform2;
    
    Transform4FuncType mfFromTransform4;
};

//////////////////////////////////////////////////////////////////////////////////////////////
class CGBKToUTF8
{
public:
    CGBKToUTF8( bool abIsStrict );
    
    apl_size_t CalcNeedSize( apl_size_t auLength );
    
    apl_ssize_t Transcode( unsigned char const** apcInput, apl_size_t auLength, unsigned char** apcOutput );
    
    apl_int_t FillReplacementChar(unsigned char** apcOutput);

public:
    bool mbIsStrictTranscode;
};

//////////////////////////////////////////////////////////////////////////////////////////////
class CUTF8ToGBK
{
public:
    CUTF8ToGBK( bool abIsStrict );
    
    apl_size_t CalcNeedSize( apl_size_t auLength );
    
    apl_ssize_t Transcode( unsigned char const** apcInput, apl_size_t auLength, unsigned char** apcOutput );
    
    apl_int_t FillReplacementChar( unsigned char** apcOutput );
    
public:
    bool mbIsStrictTranscode;
};

//////////////////////////////////////////////////////////////////////////////////////////////
class CBIG5ToUTF16
{
public:
    CBIG5ToUTF16( ETranscoderToEndian aeToEndian, bool abIsStrict );
    
    apl_size_t CalcNeedSize( apl_size_t auLength );
    
    apl_ssize_t Transcode( unsigned char const** apcInput, apl_size_t auLength, unsigned char** apcOutput );
    
    apl_int_t FillReplacementChar( unsigned char** apcOutput );
    
public:
    bool mbIsStrictTranscode;
    
    Transform2FuncType mfToTransform2;
    
    Transform4FuncType mfToTransform4;
};

//////////////////////////////////////////////////////////////////////////////////////////////
class CUTF16ToBIG5
{ 
public:
    CUTF16ToBIG5( ETranscoderFromEndian aeFromEndian, bool abIsStrict );

    apl_size_t CalcNeedSize( apl_size_t auLength );
    
    apl_ssize_t Transcode( unsigned char const** apcInput, apl_size_t auLength, unsigned char** apcOutput );
    
    apl_int_t FillReplacementChar( unsigned char** apcOutput );
    
public:
    bool mbIsStrictTranscode;
    
    Transform2FuncType mfFromTransform2;
    
    Transform4FuncType mfFromTransform4;
};

//////////////////////////////////////////////////////////////////////////////////////////////
class CBIG5ToUTF8
{
public:
    CBIG5ToUTF8( bool abIsStrict );
    
    apl_size_t CalcNeedSize( apl_size_t auLength );
    
    apl_ssize_t Transcode( unsigned char const** apcInput, apl_size_t auLength, unsigned char** apcOutput );
    
    apl_int_t FillReplacementChar( unsigned char** apcOutput );
    
public:
    bool mbIsStrictTranscode;
};

//////////////////////////////////////////////////////////////////////////////////////////////
class CUTF8ToBIG5
{ 
public:
    CUTF8ToBIG5( bool abIsStrict );

    apl_size_t CalcNeedSize( apl_size_t auLength );
    
    apl_ssize_t Transcode( unsigned char const** apcInput, apl_size_t auLength, unsigned char** apcOutput );
    
    apl_int_t FillReplacementChar( unsigned char** apcOutput );
    
public:
    bool mbIsStrictTranscode;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef TUnicodeTranscoder<CUTF32ToUTF16> CUTF32ToUTF16Transcoder;
typedef TUnicodeTranscoder<CUTF16ToUTF32> CUTF16ToUTF32Transcoder;
typedef TUnicodeTranscoder<CUTF32ToUTF8>  CUTF32ToUTF8Transcoder;
typedef TUnicodeTranscoder<CUTF8ToUTF32>  CUTF8ToUTF32Transcoder;
typedef TUnicodeTranscoder<CUTF16ToUTF8>  CUTF16ToUTF8Transcoder;
typedef TUnicodeTranscoder<CUTF8ToUTF16>  CUTF8ToUTF16Transcoder;
typedef TUnicodeTranscoder<CGBKToUTF16>   CGBKToUTF16Transcoder;
typedef TUnicodeTranscoder<CUTF16ToGBK>   CUTF16ToGBKTranscoder;
typedef TUnicodeTranscoder<CGBKToUTF8>    CGBKToUTF8Transcoder;
typedef TUnicodeTranscoder<CUTF8ToGBK>    CUTF8ToGBKTranscoder;
typedef TUnicodeTranscoder<CUTF16ToBIG5>  CUTF16ToBIG5Transcoder;
typedef TUnicodeTranscoder<CBIG5ToUTF16>  CBIG5ToUTF16Transcoder;
typedef TUnicodeTranscoder<CUTF8ToBIG5>   CUTF8ToBIG5Transcoder;
typedef TUnicodeTranscoder<CBIG5ToUTF8>   CBIG5ToUTF8Transcoder;

ACL_NAMESPACE_END

#endif //ACL_CODECS_UNICODE_H
