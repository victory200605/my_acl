
#ifndef ACL_MEMORYSTREAM_H
#define ACL_MEMORYSTREAM_H

#include "acl/MemoryBlock.h"
#include "acl/stl/string.h"

ACL_NAMESPACE_START

class CMemoryStream
{
public:
    /**
     * @brief default constructor
     */
    CMemoryStream(void);
    
    /**
     * @brief constructor
     * @param [in] apoBlock memory-block
     */
    CMemoryStream( CMemoryBlock* apoBlock );
    
    /**
     * @brief destructor
     */
    ~CMemoryStream(void);
    
    /**
     * @brief set memory stream's memory block
     * @param [in] apoBlock memory-block
     */
    void SetMemoryBlock( CMemoryBlock* apoBlock );
    
    /**
     * @brief get memory stream's memory block
     * @return memory-block pointer
     */
    CMemoryBlock* GetMemoryBlock(void);
    
    /**
     * @brief Read \e aiSize bytes from stream into the buffer pointed to by \e apvBuffer.
     *        If the stream is not enough data, stream-length will be return 
     *
     * @param [out] apvBuffer - the data buffer
     * @param [in] aiSize - buffer size
     *
     * @retval >=0 Upon successful completion, the number of bytes actually read shall be returned.
     * @retval -1  Otherwise, -1 shall be returned.
     */
	apl_ssize_t Read( void* apvBuffer, apl_size_t auN );

    /**
     * @brief Read data from stream and place the them into the \e aiN buffers specified by the members of the \e apoIoVec array. 
     *        If the stream is not enough data, stream-length will be return 
     *
     * @param [out] apoIoVec - the array to place the data
     * @param [in] aiN - the array size
     *
     * @retval >=0 Upon successful completion, the number of bytes actually read shall be returned.
     * @retval -1 Otherwise, -1 shall be returned.  
     */
	apl_ssize_t ReadV( apl_iovec_t apoIoVec[], apl_size_t auN );
	
	/**
     * @brief Read a line(end of \n or \r\n) from stream and place the them into the \e aiN buffers
     *        If Eol can't find it, -1 shall be return
     *        If buffer \e apcBuffer is not enough a line, will be truncated to \e auSize
     *
     * @param [out] apvBuffer - the data buffer
     * @param [in] aiSize - buffer size
     *
     * @retval >=0 Upon successful completion, the number of bytes actually read shall be returned
     *             and include null terminal character 
     * @retval -1 Otherwise, -1 shall be returned.  
     */
	apl_ssize_t ReadLine( char* apcBuffer, apl_size_t auSize );
	
	/**
     * @brief Read a line(end of \n or \r\n) from stream and place the them into the \e aiN buffers
     *        If Eol can't find it, -1 shall be return
     *        If EoL can't find it, stream-length will be return
     *
     * @param [out] aoOutput - output string
     *
     * @retval >=0 Upon successful completion, the number of bytes actually read shall be returned
     *             and include null terminal character 
     * @retval -1 Otherwise, -1 shall be returned.  
     */
	apl_ssize_t ReadLine( std::string& aoOutput );
	
	/**
     * @brief Read a c-string(end of \0) from stream and place the them into the \e aiN buffers
     *        If '\0' can't find it, -1 shall be return,
     *        if buffer \e apcBuffer is not enough a line, -1 will be return
     *
     * @param [out] apvBuffer - the data buffer
     * @param [in] aiSize - buffer size
     *
     * @retval >=0 Upon successful completion, the number of bytes actually read shall be returned
     *             and include null terminal character 
     * @retval -1 Otherwise, -1 shall be returned.  
     */
	apl_ssize_t ReadString( char* apcBuffer, apl_size_t auSize );
	
	/**
     * @brief Read a c-string(end of \0) from stream and place the them into the \e aiN buffers
     *        If '\0' can't find it, -1 shall be return,
     *
     * @param [out] aoOutput - output string
     *
     * @retval >=0 Upon successful completion, the number of bytes actually read shall be returned
     *             and include null terminal character 
     * @retval -1 Otherwise, -1 shall be returned.  
     */
	apl_ssize_t ReadString( std::string& aoOutput );
    
    /**
     * @brief Read from stream, read size = sizeof(atVal) .
     *
     * @param [out] atVal    store value from the stream.
     *
     * @retval >0    Successful, the number of bytes actually read shall be returned.
     * @retval -1    Fail.
     */
    template<typename ValueType>
    apl_ssize_t Read( ValueType& atVal );
    
    /**
     * @brief read data from stream until the first occurrence of \e apcTag.
     *        If end of tag can't find it, -1 shall be return
     *        If buffer \e apcBuffer is not enough a line, will be truncated to \e auSize
     *
     * @param [out] apcBuffer    the buffer to store data from the stream
     * @param [in]  aiSize       the bytes to read
     * @param [in]  apcTag       the tag string
     *
     * @retval >0  Successful, the number of bytes actually read shall be returned.
     *             and include null terminal character 
     * @retval -1  Fail.
     */
    apl_ssize_t ReadUntil( char* apcBuffer, apl_size_t aiSize, const char* apcTag );
    
    /**
     * @brief read data from stream until the first occurrence of \e apcTag.
     *        If end of tag can't find it, -1 shall be return
     *
     * @param [out] aoOutput    the return result string
     * @param [in]  apcTag      the tag string
     *
     * @retval >0  Successful, the number of bytes actually read shall be returned.
     *             and include null terminal character 
     * @retval -1  Fail.
     */
    apl_ssize_t ReadUntil( std::string& aoOutput, const char* apcTag );
    
    /**
     * @brief Read from stream and convert uint16_t quantities from network byte order to host byte order.
     *
     * @param [out] au16Val    the uint16_t quantities
     *
     * @retval >0    Successful, \e sizeof(au16Val) shall be returned. 
     * @retval -1    Fail.
     */
    apl_ssize_t ReadToH( apl_uint16_t& au16Val );
    
    /**
     * @brief Read from stream and convert uint32_t quantities from network byte order to host byte order.
     *
     * @param [out] au32Val    the uint32_t quantities
     *
     * @retval >0    Successful, \e sizeof(au32Val) shall be returned. 
     * @retval -1    Fail.
     */
    apl_ssize_t ReadToH( apl_uint32_t& au32Val );
    
    /**
     * @brief Read from stream and convert uint64_t quantities from network byte order to host byte order.
     *
     * @param [out] au64Val    the uint64_t quantities
     *
     * @retval >0    Successful, \e sizeof(au64Val) shall be returned. 
     * @retval -1    Fail.
     */
    apl_ssize_t ReadToH( apl_uint64_t& au64Val );
    
    /**
     * @brief Read from stream and convert int16_t quantities from network byte order to host byte order.
     *
     * @param [in] ai16Val    the int16_t quantities
     *
     * @retval >0    Successful, \e sizeof(ai16Val) shall be returned. 
     * @retval -1    Fail.
     */
    apl_ssize_t ReadToH( apl_int16_t& ai16Val );
    
    /**
     * @brief Read from stream and convert int32_t quantities from network byte order to host byte order.
     *
     * @param [out] ai32Val    the int32_t quantities
     *
     * @retval >0    Successful, \e sizeof(ai32Val) shall be returned. 
     * @retval -1    Fail.
     */
    apl_ssize_t ReadToH( apl_int32_t& ai32Val );
    
    /**
     * @brief Read from stream and convert int64_t quantities from network byte order to host byte order.
     *
     * @param [out] ai64Val    the int64_t quantities
     *
     * @retval >0    Successful, \e sizeof(ai64Val) shall be returned. 
     * @retval -1    Fail.
     */
    apl_ssize_t ReadToH( apl_int64_t& ai64Val );
    
    /**
     * @brief Write \e aiSize bytes from the buffer pointed to by \e apvBuffer to the file.
     *
     * @param [in] apvBuffer - the buffer to write to the file
     * @param [in] aiSize - the write bytes
     *
     * @retval >=0 Upon successful completion, the number of bytes actually written to the file shall be returned.This number shall never be greater than nbyte.
     * @retval -1 Otherwise, -1 shall be returned .
     */
	apl_ssize_t Write( void const* apvBuffer, apl_size_t auN );
    
    /**
     * @brief Write to the stream.
     *
     * @param [in] apcPtr    the string to write to the stream.
     *
     * @retval >0    Successful, the number of bytes actually write shall be returned.
     * @retval -1    Fail.
     */
    apl_ssize_t Write( char const* apcPtr );
    
    /**
     * @brief Gather output data from the \e aiN buffers specified by the members of the \e apoIoVec array and write them to the file.
     *
     * @param [in] apoIoVec - the array to place output data.
     * @param [in] aiN - the array size
     *
     * @retval >0 Upon successful completion, the number of bytes actually written to the file shall be returned.
     * @retval -1 Otherwise, -1 shall be returned.
     */
	apl_ssize_t WriteV( apl_iovec_t const apoIoVec[], apl_size_t auN );
    
    /**
     * @brief Write to the stream.
     *
     * @param [in] atVal    the value to write to the stream.
     *
     * @retval >0    Successful, the number of bytes actually write shall be returned.
     * @retval -1    Fail.
     */
    template<typename ValueType>
    apl_ssize_t Write( ValueType const& atVal );
    
    /**
     * @brief Write to the stream and convert uint16_t quantities from host byte order to network byte order. 
     *
     * @param [in] au16Val    the uint16_t quantities
     *
     * @retval >0    Successful, \e sizeof(au16Val) shall be returned.
     * @retval -1    Fail.
     */
    apl_ssize_t WriteToN( apl_uint16_t au16Val );
    
    /**
     * @brief Write to the stream and convert uint32_t quantities from host byte order to network byte order. 
     *
     * @param [in] au32Val    the uint32_t quantities
     *
     * @retval >0    Successful, \e sizeof(au32Val) shall be returned.
     * @retval -1    Fail.
     */
    apl_ssize_t WriteToN( apl_uint32_t au32Val );
    
    /**
     * @brief Write to the stream and convert uint64_t quantities from host byte order to network byte order. 
     *
     * @param [in] au64Val    the uint64_t quantities
     *
     * @retval >0    Successful, \e sizeof(au64Val) shall be returned.
     * @retval -1    Fail.
     */
    apl_ssize_t WriteToN( apl_uint64_t au64Val );
    
    /**
     * @brief Write to the stream and convert int16_t quantities from host byte order to network byte order. 
     *
     * @param [in] ai16Val    the int16_t quantities
     *
     * @retval >0    Successful, \e sizeof(ai16Val) shall be returned.
     * @retval -1    Fail.
     */
    apl_ssize_t WriteToN( apl_int16_t ai16Val );
    
    /**
     * @brief Write to the stream and convert int32_t quantities from host byte order to network byte order. 
     *
     * @param [in] ai32Val    the int32_t quantities
     *
     * @retval >0    Successful, \e sizeof(ai32Val) shall be returned.
     * @retval -1    Fail.
     */
    apl_ssize_t WriteToN( apl_int32_t ai32Val );
    
    /**
     * @brief Write to the stream and convert int64_t quantities from host byte order to network byte order. 
     *
     * @param [in] ai64Val    the int64_t quantities
     *
     * @retval >0    Successful, \e sizeof(ai64Val) shall be returned.
     * @retval -1    Fail.
     */
    apl_ssize_t WriteToN( apl_int64_t ai64Val );

private:
    CMemoryBlock* mpoBlock;
};

/////////////////////////////////////////////////////////////////////////////////////
//implement
template<typename ValueType>
apl_ssize_t CMemoryStream::Read( ValueType& atVal )
{
    if (this->mpoBlock->GetLength() >= sizeof(atVal) )
    {
        apl_memcpy(&atVal, this->mpoBlock->GetReadPtr(), sizeof(atVal) );
        this->mpoBlock->SetReadPtr(sizeof(atVal) );
        return sizeof(atVal);
    }
    else
    {
        return -1;
    }
}

template<typename ValueType>
apl_ssize_t CMemoryStream::Write( ValueType const& atVal )
{
    if (this->mpoBlock->GetSpace() >= sizeof(atVal) )
    {
        apl_memcpy(this->mpoBlock->GetWritePtr(), &atVal, sizeof(atVal) );
        this->mpoBlock->SetWritePtr(sizeof(atVal) );
        return sizeof(atVal);
    }
    else
    {
        return -1;
    }
}

ACL_NAMESPACE_END

#endif //ACL_MEMORYSTREAM_H
