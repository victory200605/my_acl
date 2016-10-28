
#ifndef ACL_CODECS_BINARY_H
#define ACL_CODECS_BINARY_H

#include "acl/Utility.h"
#include "acl/MemoryBlock.h"

ACL_NAMESPACE_START

///////////////////////////////////////////////////////////////////////////////////////////////////////
//Binary Encoder
class CBinaryEncoder
{
public:
    /**
     * Default constuctor
     */
    CBinaryEncoder(void);
 
    /**
     * Constuctor
     * Accept user defined buffer, if buffer is no enough and Encoder shall resize it,
     * and the buffer discard
     *
     * @param [out] apcBuffer default buffer pointer
     * @param [in]  auSize default buffer size
     */
    CBinaryEncoder( char* apcBuffer, apl_size_t auSize );

    /**
     * Constuctor
     *
     * @a aoMB.GetDataBlock()->Duplicate() function will be called to allocating a CDataBlock for Encoder's moBuffer
     * and @a aoMB will not be modified never.
     *
     * @param [in] aoMB buffer
     */
    CBinaryEncoder( CMemoryBlock const& aoMB );

    /**
     * Binary block update operation. Continues an Binary encode
     * operation, processing another message block, and updating the context.
     *
     * @param [in] apcInput Encoded Binary data in string.
     * @param [out] apuOutputLen Length of the binary byte stream.
     *
     * @return Upon successful completion, shall return 0; otherwise, it shall return -1
     */
    apl_int_t Update( void const* apvInput, apl_size_t auInputLen );

    /**
     * Binary finalization. zeroizing the context.
     *
     * @return Upon successful completion, shall return 0; otherwise, it shall return -1
     */
    apl_int_t Final(void);
    
    /**
     * Binary Update and Final operation.
     *
     * @param [in] apvInput Binary data in byte stream.
     * @param [in] auInputLen Length of the byte stream.
     * @return Upon successful completion, shall return 0; otherwise, it shall return -1
     */
    apl_int_t Final( void const* apvInput, apl_size_t auInputLen );
    
    /**
     * @return Binary output context.
     */
    char const* GetOutput(void);
    
    /**
     * @return Binary context length.
     */
    apl_size_t GetLength(void);
    
    /**
     * Reset, zeroizing the context.
     */
    void Reset(void);
    
private:
    CMemoryBlock moBuffer;

    apl_size_t muLength;
};

//Binary Decoder
class CBinaryDecoder
{
public:
    /**
     * Default constuctor
     */
    CBinaryDecoder(void);
 
    /**
     * Constuctor
     * Accept user defined buffer, if buffer is no enough and Encoder shall resize it,
     * and the buffer discard
     *
     * @param [out] apcBuffer default buffer pointer
     * @param [in]  auSize default buffer size
     */
    CBinaryDecoder( char* apcBuffer, apl_size_t auSize );

    /**
     * Constuctor
     *
     * @a aoMB.GetDataBlock()->Duplicate() function will be called to allocating a CDataBlock for Decoder's moBuffer
     * and @a aoMB will not be modified never.
     *
     * @param [in] aoMB buffer
     */
    CBinaryDecoder( CMemoryBlock const& aoMB );

    /**
     * Binary block update operation. Continues an Binary decode
     * operation, processing another message block, and updating the context.
     *
     * @return Upon successful completion, shall return 0; otherwise, it shall return -1
     */
    apl_int_t Update( char const* apcInput );
    
    /**
     * Binary block update operation. Continues an Binary decode
     * operation, processing another message block, and updating the context.
     *
     * @param [in] apvInput Binary data in string.
     * @param [in] auInputLen Length of the string.
     *
     * @return Upon successful completion, shall return 0; otherwise, it shall return -1
     */
    apl_int_t Update( char const* apcInput, apl_size_t auInputLen );
    
    /**
     * Binary finalization. zeroizing the context.
     *
     * @return Upon successful completion, shall return 0; otherwise, it shall return -1
     */
    apl_int_t Final(void);
    
    /**
     * Binary Update and Final operation.
     *
     * @param [in] apvInput Binary data in string.
     * @return Upon successful completion, shall return 0; otherwise, it shall return -1
     */
    apl_int_t Final( char const* apcInput );
    
    /**
     * Binary Update and Final operation.
     *
     * @param [in] apvInput Binary data in byte stream.
     * @param [in] auInputLen Length of the byte stream.
     * @return Upon successful completion, shall return 0; otherwise, it shall return -1
     */
    apl_int_t Final( char const* apcInput, apl_size_t auInputLen );
    
    /**
     * @return Binary output context.
     */
    char const* GetOutput(void);

    /**
     * @return Binary context length.
     */
    apl_size_t GetLength(void);
    
    /**
     * Reset, zeroizing the context.
     */
    void Reset(void);
    
private:
    CMemoryBlock moBuffer;
    
    unsigned char macGroup[8];
    
    apl_size_t muCount;
    
    apl_size_t muLength;
};

ACL_NAMESPACE_END

#endif //ACL_CODECS_BINARY_H
