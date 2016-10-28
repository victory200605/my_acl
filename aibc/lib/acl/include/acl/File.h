/**
 * @file File.h
 */

#ifndef ACL_FILE_H
#define ACL_FILE_H

#include "acl/Utility.h"
#include "acl/IOHandle.h"

ACL_NAMESPACE_START
 
/**
 * class CFileInfo
 */
class CFileInfo
{
    friend class CFile;
public:
    /**
     * @brief Get the file size in bytes for regular files.\n
     * Get the length in bytes of the pathname contained in the symbolic link for symbolic links.
     *
     * @return The file size in bytes or the length in bytes of the pathname contained in the symbolic link.
     */
    apl_size_t GetSize(void) const;
    
    /**
     * @brief Get device ID of device containing file.
     *
     * @return The device ID.
     */
    apl_int_t GetDev(void) const;

    /**
     * @brief Get file serial number.
     *
     * @return The file serial number.
     */
    apl_int_t GetIno(void) const;

    /**
     * @brief Get mode of file.
     *
     * @return The mode of file.
     */
    apl_int_t GetMode(void) const;

    /**
     * @brief Get the number of hard links to the file.
     *
     * @return The number of hard links to the file.
     */
    apl_int_t GetNLink(void) const;

    /**
     * @brief Get the user ID of file.
     *
     * @return  The user ID of file.
     */
    apl_int_t GetUid(void) const;

    /**
     * @brief Get group ID of file.
     *
     * @return The group ID of file.
     */
    apl_int_t GetGid(void) const;

    /**
     * @brief Get the time of last access.
     *
     * @return The time of last access.
     */
    apl_time_t GetATime(void) const;

    /**
     * @brief Get the time of last data modification.
     *
     * @return The time of last data modification. 
     */
    apl_time_t GetMTime(void) const;

    /**
     * @brief Get the time of last status change.
     *
     * @return The time of last status change.
     */
    apl_time_t GetCTime(void) const;
    
protected:
    apl_size_t muSize;
    apl_int_t  miDev;
    apl_int_t  miIno;
    apl_int_t  miMode;
    apl_int_t  miNLink;
    apl_int_t  miUid;
    apl_int_t  miGid;
    apl_time_t mi64ATime;
    apl_time_t mi64MTime;
    apl_time_t mi64CTime;
};

/**
 * class CFile
 */
class CFile : public CIOHandle
{
public:
    typedef CFileInfo FileInfoType;
    
public:

    /**
     * A constructor.
     */
    CFile(void);
    
    /**
     * A destructor.
     */
    ~CFile(void);
    
    /**
     * @brief Open a file.
     *
     * @param [in] apcFileName - the file pathname
     * @param [in] aiFlag - the file status flags 
     * @param [in] aiMode - the access permission bits of the file mode,it does not affect whether the file is open for reading, writing, or for both.
     * 
     * @retval 0 Open a file successfully.
     * @retval -1 Open a file failed.
     */
	apl_int_t Open( char const* apcFileName, apl_int_t aiFlag = APL_O_RDONLY, apl_int_t aiMode = 0 );
	
    /**
     * @brief Open a file in /dev/shm file system.
     *
     * @param [in] apcFileName - the file pathname
     * @param [in] aiFlag - the file status flags 
     * @param [in] aiMode - the access permission bits of the file mode,it does not affect whether the file is open for reading, writing, or for both.
     * 
     * @retval 0 Open a file successfully.
     * @retval -1 Open a file failed.
     */
	apl_int_t ShmOpen( char const* apcFileName, apl_int_t aiFlag = APL_O_RDONLY, apl_int_t aiMode = 0 );
	
    /**
     * @brief Truncate a file to a specified length.
     *
     * @param [in] ai64Length - the specified size to be truncated.
     *
     * @retval 0 Upon successful completion.
     * @retval -1 Failed.
     */
	apl_int_t Truncate( apl_int64_t ai64Length );
	
    /**
     * @brief Ensure that any required storage for regular file data starting at \e ai64_offset and continuing for \e ai64_length bytes is allocated on the file system storage media.
     *
     * @param [in] ai64Offset - the file offset, indicate the beginning of allocating
     * @param [in] ai64Length - the space length is allocated on the file system storage media
     *
     * @retval 0 Upon successful completion.
     * @retval -1 Failed.
     */
	apl_int_t Allocate( apl_int64_t ai64Offset, apl_int64_t ai64Length );
	
    /**
     * @brief Set the file offset.
     *
     * @param [in] ai64Offset - the file offset
     * @param [in] aiWhence - set offset mode
     * - APL_SEEK_SET : the file offset shall be set to \e ai64Offset bytes
     * - APL_SEEK_CUR : the file offset shall be set to its current location plus \e ai64_offset
     * - APL_SEEK_END : the file offset shall be set to the size of the file plus \e ai64_offset
     *
     * @retval >=0 Upon successful completion, the resulting offset, as measured in bytes from the beginning of the file, shall be returned. 
     * @retval -1 Otherwise, -1 shall be returned, errno shall be set to indicate the error, and the file offset shall remain unchanged.
     */
	apl_int64_t Seek( apl_int64_t ai64Offset, apl_int_t aiWhence = APL_SEEK_SET );
	
	/**
     * @brief The tell() function obtains the current value of  the  file-position indicator for the file
     *
     * @retval >=0 Upon successful completion, the resulting offset, as measured in bytes from the beginning of the file, shall be returned. 
     * @retval -1 Otherwise, -1 shall be returned, errno shall be set to indicate the error, and the file offset shall remain unchanged.
     */
	apl_int64_t Tell(void);

    /**
     * @brief Synchronize changes to a file.
     *
     * @retval 0 Upon successful completion.
     * @retval -1 Failed.
     */
	apl_int_t Sync(void);
	
    /**
     * @brief Read \e aiSize bytes from the file into the buffer pointed to by \e apvBuffer.
     *
     * @param [out] apvBuffer - the buffer to store data from file
     * @param [in] aiSize - the bytes to read
     *
     * @retval >=0 Upon successful completion, the number of bytes actually read shall be returned.
     * @retval -1  Otherwise, -1 shall be returned.
     */
	apl_ssize_t Read( void* apvBuffer, apl_size_t aiSize );

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
	apl_ssize_t Read( apl_int64_t ai64Offset, void* apvBuffer, apl_size_t aiSize );
	
    /**
     * @brief Read data from a file and place the them into the \e aiN buffers specified by the members of the \e apoIoVec array. 
     *
     * @param [out] apoIoVec - the array to place the data
     * @param [in] aiN - the array size
     *
     * @retval >=0 Upon successful completion, the number of bytes actually read shall be returned.
     * @retval -1 Otherwise, -1 shall be returned.  
     */
	apl_ssize_t ReadV( apl_iovec_t apoIoVec[], apl_size_t aiN );

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
	apl_ssize_t ReadV( apl_int64_t ai64Offset, apl_iovec_t apoIoVec[], apl_size_t aiN );

    /**
     * @brief Write \e aiSize bytes from the buffer pointed to by \e apvBuffer to the file.
     *
     * @param [in] apvBuffer - the buffer to write to the file
     * @param [in] aiSize - the write bytes
     *
     * @retval >=0 Upon successful completion, the number of bytes actually written to the file shall be returned.This number shall never be greater than nbyte.
     * @retval -1 Otherwise, -1 shall be returned .
     */
	apl_ssize_t Write( void const* apvBuffer, apl_size_t aiSize);
	
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
	apl_ssize_t Write( apl_int64_t ai64Offset, void const* apvBuffer, apl_size_t aiSize );
	
    /**
     * @brief Gather output data from the \e aiN buffers specified by the members of the \e apoIoVec array and write them to the file.
     *
     * @param [in] apoIoVec - the array to place output data.
     * @param [in] aiN - the array size
     *
     * @retval >0 Upon successful completion, the number of bytes actually written to the file shall be returned.
     * @retval -1 Otherwise, -1 shall be returned.
     */
	apl_ssize_t WriteV( apl_iovec_t const apoIoVec[], apl_size_t aiN);
	
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
	apl_ssize_t WriteV( apl_int64_t ai64Offset, apl_iovec_t const apoIoVec[], apl_size_t aiN );

    /**
     * @brief Check the file is open or not.
     *
     * @retval true  the file is open
     * @retval false  the file is close
     */
    bool IsOpened() const;
    
    /**
     * @brief Get FileInfo.
     *
     * @param [out] aoFileInfo - the file info
     *
     * @retval 0 Upon successful completion.
     * @retval -1 Failed.
     */
    apl_int_t GetFileInfo( FileInfoType& aoFileInfo );
    
    /**
     * @brief Get FileInfo.
     *
     * @param [out] apoFileInfo - the file info
     *
     * @retval 0 Upon successful completion.
     * @retval -1 Failed.
     */
    apl_int_t GetFileInfo( FileInfoType* apoFileInfo );
    
    /**
     * @brief Get FileSize.
     *
     * @return if success file size shall be return, Otherwise, -1 shall be returned and errno set to indicate the error
     */
    apl_int64_t GetFileSize(void);

public:
    static apl_int_t Access( char const* apcPathName, apl_int_t aiMode );
	
	static apl_int_t Remove( char const* apcPathName );
	
    static apl_int_t ShmRemove( char const* apcPathName );
	
	static apl_int_t Rename( char const* apcOld, char const* apcNew );
    
    static apl_int_t GetFileInfo( char const* apcPathName, FileInfoType* apoFileInfo );
    
    static apl_int_t GetFileInfo( char const* apcPathName, FileInfoType& aoFileInfo );

protected:
    static void AssignFileInfo( FileInfoType& aoFileInfo, apl_stat_t& aoStat );

private:
    CFile( CFile const& );
    CFile& operator = ( CFile const& );
};

ACL_NAMESPACE_END

#endif//ACL_FILE_H
