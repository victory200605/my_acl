
#ifndef ACL_HASHS_H
#define ACL_HASHS_H

#include "acl/Utility.h"

ACL_NAMESPACE_START
  
////////////////////////////////////////////////////////////////////////////////////////////////////
class CMD5
{
    class CMD5Digest
    {
        friend class CMD5;
    public:
        unsigned char const* GetUstr(void)
        {
            return this->macDigest;
        }
        
        apl_size_t GetLength(void)
        {
            return sizeof(this->macDigest);
        }
    
    protected:    
        unsigned char macDigest[16];
    };
    
public:
    //MD5 digest type
    typedef CMD5Digest DigestType;
    
public:
    /**
     * Default constuctor
     */
    CMD5(void);

    /**
     * MD5 block update operation. Continues an MD5 message-digest
     * operation, processing another message block, and updating the context.
     *
     * @param [in] apcInput Binary data in byte stream.
     */
    void Update( char const* apcInput );
    
    /**
     * MD5 block update operation. Continues an MD5 message-digest
     * operation, processing another message block, and updating the context.
     *
     * @param [in] apvInput Binary data in byte stream.
     * @param [in] auInputLen Length of the byte stream.
     */
    void Update( void const* apvInput, apl_size_t auInputLen );

    /**
     * MD5 finalization. Ends an MD5 message-digest operation, writing the
     * the message digest and zeroizing the context.
     *
     * @return MD5 message-digest.
     */
    DigestType Final(void);
    
    /**
     * MD5 block update and final operation.
     *
     * @param [in] apcInput Binary data in byte stream.
     * @return MD5 message-digest.
     */
    DigestType Final( char const* apcInput );
    
    /**
     * MD5 block update and final operation.
     *
     * @param [in] apcInput Binary data in byte stream.
     * @param [in] auInputLen Length of the byte stream.
     * @return MD5 message-digest.
     */
    DigestType Final( void const* apcInput, apl_size_t auInputLen );

    /**
     * Reset, zeroizing the context.
     */
    void Reset(void);

protected:
    void MD5Transform( apl_uint32_t* apiState, unsigned char const* apcInput );
    void MD5Memcpy( unsigned char* apcOutput, unsigned char const* apcInput, apl_size_t auLen );
    void MD5Encode( unsigned char* apcOutput, apl_uint32_t const* apuInput, apl_size_t auInputLen );
    void MD5Decode( apl_uint32_t* apuOutput, unsigned char const* apcInput, apl_size_t auInputLen );
    
private:
    //ABCD
    apl_uint32_t mauState[4];
    
    //number of bits, mod 2^64
    apl_uint64_t mi64Count;
    
    unsigned char macBuffer[64];
    
    unsigned char macPadding[64];
};

ACL_NAMESPACE_END

#endif //ACL_HASHS_H
