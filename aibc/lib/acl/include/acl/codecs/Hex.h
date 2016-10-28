
#ifndef ACL_CODECS_HEX_H
#define ACL_CODECS_HEX_H

#include "acl/Utility.h"
#include "acl/MemoryBlock.h"

ACL_NAMESPACE_START

///////////////////////////////////////////////////////////////////////////////////////////////////////
//Hex Encoder
class CHexEncoder
{
public:
    enum EOption
    {
        OPT_UPPER = 1,
        OPT_LOWER = 2
    };
    
public:
    /**
     * Default constuctor
     */
    CHexEncoder( EOption aeOpt = OPT_UPPER );
 
    /**
     * Constuctor
     * Accept user defined buffer, if buffer is no enough and Encoder shall resize it,
     * and the buffer discard
     *
     * @param [out] apcBuffer default buffer pointer
     * @param [in]  auSize default buffer size
     */
    CHexEncoder( char* apcBuffer, apl_size_t auSize, EOption aeOpt = OPT_UPPER );

    /**
     * Constuctor
     *
     * @a aoMB.GetDataBlock()->Duplicate() function will be called to allocating a CDataBlock for Encoder's moBuffer
     * and @a aoMB will not be modified never.
     *
     * @param [in] aoMB buffer
     */
    CHexEncoder( CMemoryBlock const& aoMB, EOption aeOpt = OPT_UPPER );

    /**
     * Hex block update operation. Continues an Hex encode
     * operation, processing another message block, and updating the context.
     *
     * @param [in] apcInput Encoded Hex data in string.
     * @param [out] apuOutputLen Length of the binary byte stream.
     *
     * @return Upon successful completion, shall return 0; otherwise, it shall return -1
     */
    apl_int_t Update( void const* apvInput, apl_size_t auInputLen );

    /**
     * Hex finalization. zeroizing the context.
     *
     * @return Upon successful completion, shall return 0; otherwise, it shall return -1
     */
    apl_int_t Final(void);
    
    /**
     * Hex Update and Final operation.
     *
     * @param [in] apvInput Binary data in byte stream.
     * @param [in] auInputLen Length of the byte stream.
     * @return Upon successful completion, shall return 0; otherwise, it shall return -1
     */
    apl_int_t Final( void const* apvInput, apl_size_t auInputLen );
    
    /**
     * @return Hex output context.
     */
    char const* GetOutput(void);
    
    /**
     * @return Hex context length.
     */
    apl_size_t GetLength(void);
    
    /**
     * Reset, zeroizing the context.
     */
    void Reset(void);
    
private:
    CMemoryBlock moBuffer;

    apl_size_t muLength;
    
    EOption meOption;
};

//Hex Decoder
class CHexDecoder
{
public:
    /**
     * Default constuctor
     */
    CHexDecoder(void);
 
    /**
     * Constuctor
     * Accept user defined buffer, if buffer is no enough and Decoder shall resize it,
     * and the buffer discard
     *
     * @param [out] apcBuffer default buffer pointer
     * @param [in]  auSize default buffer size
     */
    CHexDecoder( char* apcBuffer, apl_size_t auSize );

    /**
     * Constuctor
     *
     * @a aoMB.GetDataBlock()->Duplicate() function will be called to allocating a CDataBlock for Decoder's moBuffer
     * and @a aoMB will not be modified never.
     *
     * @param [in] aoMB buffer
     */
    CHexDecoder( CMemoryBlock const& aoMB );

    /**
     * Hex block update operation. Continues an Hex decode
     * operation, processing another message block, and updating the context.
     *
     * @return Upon successful completion, shall return 0; otherwise, it shall return -1
     */
    apl_int_t Update( char const* apcInput );
    
    /**
     * Hex block update operation. Continues an Hex decode
     * operation, processing another message block, and updating the context.
     *
     * @param [in] apvInput Hex data in string.
     * @param [in] auInputLen Length of the string.
     *
     * @return Upon successful completion, shall return 0; otherwise, it shall return -1
     */
    apl_int_t Update( char const* apcInput, apl_size_t auInputLen );
    
    /**
     * Hex finalization. zeroizing the context.
     *
     * @return Upon successful completion, shall return 0; otherwise, it shall return -1
     */
    apl_int_t Final(void);
    
    /**
     * Hex Update and Final operation.
     *
     * @param [in] apvInput Hex data in string.
     * @return Upon successful completion, shall return 0; otherwise, it shall return -1
     */
    apl_int_t Final( char const* apcInput );
    
    /**
     * Hex Update and Final operation.
     *
     * @param [in] apvInput Hex data in byte stream.
     * @param [in] auInputLen Length of the byte stream.
     * @return Upon successful completion, shall return 0; otherwise, it shall return -1
     */
    apl_int_t Final( char const* apcInput, apl_size_t auInputLen );
    
    /**
     * @return Hex output context.
     */
    char const* GetOutput(void);

    /**
     * @return Hex context length.
     */
    apl_size_t GetLength(void);
    
    /**
     * Reset, zeroizing the context.
     */
    void Reset(void);
    
private:
    CMemoryBlock moBuffer;
    
    unsigned char macGroup[2];
    
    apl_size_t muCount;
    
    apl_size_t muLength;
};

ACL_NAMESPACE_END

#endif //ACL_CODECS_HEX_H
