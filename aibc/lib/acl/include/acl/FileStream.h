
#ifndef ACL_FILESTREAM_H
#define ACL_FILESTREAM_H

#include "acl/File.h"
#include "acl/MemoryBlock.h"
#include "acl/stl/string.h"

ACL_NAMESPACE_START

class CFileStream : public CFile
{
public:
    /**
     * @brief default constructor
     */
    CFileStream(void);
    
    /**
     * @brief constructor
     * @param [in] auBufferSize - readline buffer size 
     */
    CFileStream( apl_size_t auBufferSize );
    
    /**
     * @brief destructor
     */
    ~CFileStream(void);
    
    /**
     * @brief set readline buffer size
     *
     * @param [in] auBufferSize - readline buffer size 
     */
    void SetBufferSize( apl_size_t auBufferSize );
    
    /**
     * @brief clear readline buffer
     */
    void ClearBuffer(void);
    
    /**
     * @brief Read a line(end of \n or \r\n) from file into the buffer pointed to by \e apvBuffer.
     *        If EoL can't find it, all will be return if buffer is enough,
     *        if buffer \e apcBuffer is not enough a line, will be truncated to \e auSize
     *
     * @param [out] apvBuffer - the buffer to store data from file
     * @param [in] aiSize - the bytes to read
     *
     * @retval >=0 Upon successful completion, the number of bytes actually read shall be returned.
     * @retval -1  Otherwise, -1 shall be returned.
     */
    apl_ssize_t ReadLine( char* apcBuffer, apl_size_t auSize );
    
    /**
     * @brief Read a line(end of \n or \r\n) from file into the buffer pointed to by \e apvBuffer.
     *        If EoL can't find it, all will be return if buffer is enough,
     *        if buffer \e apcBuffer is not enough a line, will be truncated to \e auSize
     *
     * @param [out] apvBuffer - the buffer to store data from file
     * @param [in] aiSize - the bytes to read
     *
     * @retval >=0 Upon successful completion, the number of bytes actually read shall be returned.
     * @retval -1  Otherwise, -1 shall be returned.
     */
    apl_ssize_t ReadLine( std::string& aoOutput );
    
    /**
     * @brief Read \e aiSize bytes from the file into the buffer pointed to by \e apvBuffer.
     *
     * @param [out] apvBuffer - the buffer to store data from file
     * @param [in] aiSize - the bytes to read
     *
     * @retval >=0 Upon successful completion, the number of bytes actually read shall be returned.
     * @retval -1  Otherwise, -1 shall be returned.
     */
	apl_ssize_t Read( void* apvBuffer, apl_size_t auSize );

    /**
     * @brief Read \e aiSize bytes from a given position in the file without changing the file pointer into the buffer pointed to by \e apvBuffer.
     *
     * @param [in] ai64Offset - the desired position inside the file
     * @param [out] apvBuffer - the buffer to store data from file
     * @param [in] aiSize - the bytes to read
     *
     * @retval >=0 Upon successful completion, the number of bytes actually read shall be returned.
     * @retval -1  Otherwise, -1 shall be returned.
     */
	apl_ssize_t Read( apl_int64_t ai64Offset, void* apvBuffer, apl_size_t auSize );
	
    /**
     * @brief Read data from a file and place the them into the \e aiN buffers specified by the members of the \e apoIoVec array. 
     *
     * @param [out] apoIoVec - the array to place the data
     * @param [in] aiN - the array size
     *
     * @retval >=0 Upon successful completion, the number of bytes actually read shall be returned.
     * @retval -1 Otherwise, -1 shall be returned.  
     */
	apl_ssize_t ReadV( apl_iovec_t apoIoVec[], apl_size_t auN );

    /**
     * @brief Read data from from a given position in a file and place the them into the \e aiN buffers specified by the members of the \e apoIoVec array. 
     *
     * @param [in] ai64Offset - the desired position inside the file
     * @param [out] apoIoVec - the array to place the data
     * @param [in] aiN - the array size
     *
     * @retval >=0 Upon successful completion, the number of bytes actually read shall be returned.
     * @retval -1 Otherwise, -1 shall be returned.  
     */
	apl_ssize_t ReadV( apl_int64_t ai64Offset, apl_iovec_t apoIoVec[], apl_size_t auN );

    /**
     * @brief Write \e aiSize bytes from the buffer pointed to by \e apvBuffer to the file.
     *
     * @param [in] apvBuffer - the buffer to write to the file
     * @param [in] aiSize - the write bytes
     *
     * @retval >=0 Upon successful completion, the number of bytes actually written to the file shall be returned.This number shall never be greater than nbyte.
     * @retval -1 Otherwise, -1 shall be returned .
     */
	apl_ssize_t Write( void const* apvBuffer, apl_size_t auSize);
	
    /**
     * @brief Write \e aiSize bytes from the buffer pointed to by \e apvBuffer to the given position of the file.
     *
     * @param [in] ai64Offset - the desired position inside the file
     * @param [in] apvBuffer - the buffer to write to the file
     * @param [in] aiSize - the write bytes
     *
     * @retval >=0 Upon successful completion, the number of bytes actually written to the file shall be returned.This number shall never be greater than nbyte.
     * @retval -1 Otherwise, -1 shall be returned .
     */
	apl_ssize_t Write( apl_int64_t ai64Offset, void const* apvBuffer, apl_size_t auSize );
	
    /**
     * @brief Gather output data from the \e aiN buffers specified by the members of the \e apoIoVec array and write them to the file.
     *
     * @param [in] apoIoVec - the array to place output data.
     * @param [in] aiN - the array size
     *
     * @retval >0 Upon successful completion, the number of bytes actually written to the file shall be returned.
     * @retval -1 Otherwise, -1 shall be returned.
     */
	apl_ssize_t WriteV( apl_iovec_t const apoIoVec[], apl_size_t auN);
	
    /**
     * @brief Gather output data from the \e aiN buffers specified by the members of the \e apoIoVec array and write them to the given position in the file.
     *
     * @param [in] ai64Offset - the desired position inside the file
     * @param [in] apoIoVec - the array to place output data.
     * @param [in] aiN - the array size
     *
     * @retval >=0 Upon successful completion, the number of bytes actually written to the file shall be returned.
     * @retval -1 Otherwise, -1 shall be returned.
     */
	apl_ssize_t WriteV( apl_int64_t ai64Offset, apl_iovec_t const apoIoVec[], apl_size_t auN );
    
	/**
     * @brief The tell() function obtains the current value of  the  file-position indicator for the file stream
     *
     * @retval >=0 Upon successful completion, the resulting offset, as measured in bytes from the beginning of the file, shall be returned. 
     * @retval -1 Otherwise, -1 shall be returned, errno shall be set to indicate the error, and the file offset shall remain unchanged.
     */
	apl_int64_t Tell(void);

    /**
     * @brief IsEof function shall test the end-of-file indicator for the file stream
     *        NOTE: If there is a read buffer, the end-of-file may be brought forward, exp ReadLine function
     *
     * return true if and only if the end-of-file indicator is set for file stream, otherwise return false
     */
     bool IsEof(void);
     
private:
    acl::CMemoryBlock moBuffer;
};

ACL_NAMESPACE_END

#endif //ACL_FILESTREAM_H
