
#ifndef ACL_CODECS_BASE64_H
#define ACL_CODECS_BASE64_H

#include "acl/Utility.h"
#include "acl/MemoryBlock.h"

ACL_NAMESPACE_START

////////////////////////////////////////////////////////////////////////////////////////////////////
class CBase64Encoder
{
public:
    /**
     * Default constuctor
     */
    CBase64Encoder(void);
    
    /**
     * Constuctor
     * Accept user defined buffer, if buffer is no enough and Encoder shall resize it,
     * and the buffer discard
     *
     * @param [out] apcBuffer default buffer pointer
     * @param [in]  auSize default buffer size
     */
    CBase64Encoder( char* apcBuffer, apl_size_t auSize );

    /**
     * Constuctor
     *
     * @a aoMB.GetDataBlock()->Duplicate() function will be called to allocating a CDataBlock for Encoder's moBuffer
     * and @a aoMB will not be modified never.
     *
     * @param [in] aoMB buffer
     */
    CBase64Encoder( CMemoryBlock const& aoMB );

    /**
     * Base64 block update operation. Continues an Base64 encode
     * operation, processing another message block, and updating the context.
     *
     * @param [in] apcInput Encoded Base64 data in string.
     *
     * @return Upon successful completion, shall return 0; otherwise, it shall return -1
     */
    //apl_int_t Update( char const* apcInput );
    
    /**
     * Base64 block update operation. Continues an Base64 encode
     * operation, processing another message block, and updating the context.
     *
     * @param [in] apcInput Encoded Base64 data in string.
     * @param [out] apuOutputLen Length of the binary byte stream.
     *
     * @return Upon successful completion, shall return 0; otherwise, it shall return -1
     */
    apl_int_t Update( void const* apvInput, apl_size_t auInputLen );

    /**
     * Base64 finalization. Padding the end character '=' and zeroizing the context.
     *
     * @return Upon successful completion, shall return 0; otherwise, it shall return -1
     */
    apl_int_t Final(void);
    
    /**
     * Base64 Update and Final operation.
     *
     * @param [in] apvInput Binary data in byte stream.
     * @param [in] auInputLen Length of the byte stream.
     * @return Upon successful completion, shall return 0; otherwise, it shall return -1
     */
    apl_int_t Final( void const* apvInput, apl_size_t auInputLen );
    
    /**
     * @return Base64 output context.
     */
    char const* GetOutput(void);
    
    /**
     * @return Base64 context length.
     */
    apl_size_t GetLength(void);
    
    /**
     * Reset, zeroizing the context.
     */
    void Reset(void);

private:
    unsigned char macEncodeGroup[3];

    apl_size_t muCount;
    
    CMemoryBlock moBuffer;
    
    apl_size_t muLength;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
class CBase64Decoder
{
public:
    /**
     * Default constuctor
     */
    CBase64Decoder(void);

    /**
     * Constuctor
     * Accept user defined buffer, if buffer is no enough and Encoder shall resize it,
     * and the buffer discard
     *
     * @param [out] apcBuffer default buffer pointer
     * @param [in]  auSize default buffer size
     */
    CBase64Decoder( char* apcBuffer, apl_size_t auSize );

    /**
     * Constuctor
     *
     * @a aoMB.GetDataBlock()->Duplicate() function will be called to allocating a CDataBlock for Decoder's moBuffer
     * and @a aoMB will not be modified never.
     *
     * @param [in] aoMB buffer
     */
    CBase64Decoder( CMemoryBlock const& aoMB );

    /**
     * Base64 block update operation. Continues an Base64 decode
     * operation, processing another message block, and updating the context.
     *
     * @return Upon successful completion, shall return 0; otherwise, it shall return -1
     */
    apl_int_t Update( char const* apcInput );
    
    /**
     * Base64 block update operation. Continues an Base64 decode
     * operation, processing another message block, and updating the context.
     *
     * @param [in] apvInput Binary data in string.
     * @param [in] auInputLen Length of the string.
     *
     * @return Upon successful completion, shall return 0; otherwise, it shall return -1
     */
    apl_int_t Update( char const* apcInput, apl_size_t auInputLen );
    
    /**
     * Base64 finalization. zeroizing the context.
     *
     * @return Upon successful completion, shall return 0; otherwise, it shall return -1
     */
    apl_int_t Final(void);
    
    /**
     * Base64 Update and Final operation.
     *
     * @param [in] apvInput Binary data in string.
     * @return Upon successful completion, shall return 0; otherwise, it shall return -1
     */
    apl_int_t Final( char const* apcInput );
    
    /**
     * Base64 Update and Final operation.
     *
     * @param [in] apvInput Binary data in byte stream.
     * @param [in] auInputLen Length of the byte stream.
     * @return Upon successful completion, shall return 0; otherwise, it shall return -1
     */
    apl_int_t Final( char const* apcInput, apl_size_t auInputLen );
    
    /**
     * @return Base64 output context.
     */
    char const* GetOutput(void);

    /**
     * @return Base64 context length.
     */
    apl_size_t GetLength(void);
    
    /**
     * Reset, zeroizing the context.
     */
    void Reset(void);

protected:
    apl_int_t Next4( unsigned char** appcInput, unsigned char apcOutput[], apl_size_t auCurrLen, apl_int_t& aiPadChrs );
    
private:
    unsigned char macDecodeGroup[4];
    
    apl_size_t muCount;
    
    CMemoryBlock moBuffer;
    
    apl_size_t muLength;
    
    apl_size_t mbIsFinished;
};

ACL_NAMESPACE_END

#endif //ACL_CODECS_BASE64_H
