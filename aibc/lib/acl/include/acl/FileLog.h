/**
 * @file FileLog.h
 */
#ifndef ACL_FILELOG_H
#define ACL_FILELOG_H

#include "acl/Utility.h"
#include "acl/Log.h"
#include "acl/File.h"
#include "acl/Synch.h"

ACL_NAMESPACE_START
 
/**
 * Class CFileLog
 */
class CFileLog
{
public:
    /**
     * Define the the max length of line of filelog.
     */
    static const apl_int_t MAX_LOG_BUFFER_SIZE;
    
public:
    /**
     * Constructor
     *
     * @param [in] aoFormat the format object of filelog
     */
    CFileLog( CAnyLogFormat aoFormat = CAnyLogFormat::DefaultFormatType() );
    
    /**
     * Open filelog file.
     *
     * @param [in] apcFileName    the path of filelog
     * @retval >0 FileHandle. If success
     * @retval -1 If fail
     */
    apl_int_t Open( char const* apcFileName );
    
    /**
     * Close the filelog file.
     */
    void Close( void );
    
    /**
     * Write the filelog by the level.
     *
     * @param [in] aeLevel     the level of log
     * @param [in] apcFormat   the format of content of log
     * @param [in] ...         the content of log 
     * @retval >0. the number of elements successfully written 
     * @retval <=0. If fail
     */ 
    apl_int_t Write( ELLType aeLevel, char const* apcFormat, ... );
       
    /**
     * Write the string to  filelog.
     *
     * @param [in] apcBuffer   the content of log
     * @param [in] aiSize      the size of content of log
     * @retval >0. the number of elements successfully written
     * @retval <=0. If fail
     */ 
    apl_int_t Write( char const* apcBuffer, apl_size_t aiSize );
    
    /**
     * Backup the filelog.
     *
     * @param [in] apcPath    the path of backup file
     * @retval 0 If success
     * @retval -1 If fail
     */
    apl_int_t Backup( char const* apcPath );
    
    /**
     * Get the size of filelog file.
     * 
     * @retval >=0. If file exist 
     * @retval -1. If file doesn't exist 
     */
    apl_ssize_t GetSize( void );
    
private:
    CLock moLock;
    
    CFile moFile;
    
    char macFileName[APL_NAME_MAX];
    
    CAnyLogFormat moFormat;
};

ACL_NAMESPACE_END

#endif//ACL_FILELOG_H
