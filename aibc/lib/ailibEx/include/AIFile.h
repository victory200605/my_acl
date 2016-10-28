//=============================================================================
/**
* \file    AIFile.h
* \brief file operator class
*
* Copyright (c) 2007 Asiainfo Technologies(China),Inc.
* RCS: $Id: AIFile.h,v 1.2 2009/03/02 13:39:50 daizh Exp $
*
* History
* 2008.01.18 first release by DZH
*/
//=============================================================================
#ifndef __AILIBEX__AIFILE_H__
#define __AILIBEX__AIFILE_H__

#if !defined(WIN32)
#include <unistd.h>
#else
#include <Windows.h>
#include <io.h>
#endif
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stddef.h>
#include <stdio.h>
#include <errno.h> 

#include "AILib.h"

///start namespace
AIBC_NAMESPACE_START

//define file handle 
#if defined(WIN32)

#   define   AI_HANDLE HANDLE //for win32
#   define   AI_ERROR_HANDLE DWORD   

#   if !defined (ssize_t)
#       define  ssize_t int
#   endif

// for use by access()
#   if !defined (R_OK)
#     define R_OK    04      /* Test for Read permission. */
#   endif /* R_OK */

#   if !defined (W_OK)
#     define W_OK    02      /* Test for Write permission. */
#   endif /* W_OK */

#   if !defined (X_OK)
#     define X_OK    01      /* Test for eXecute permission. */
#   endif /* X_OK */

#   if !defined (F_OK)
#     define F_OK    0       /* Test for existence of File. */
#   endif /* F_OK */

// for file open()
#   if !defined (O_CREAT)
#     define O_CREAT  0x01   /* Create file if unexist */
#   endif

#   if !defined (O_RDWR)
#     define O_RDWR   0x02   /* read / write mode */
#   endif

#   if !defined (O_RDONLY)
#     define O_RDONLY   0x04
#   endif

#   if !defined (O_WRONLY)
#     define O_WRONLY   0x04
#   endif

// for file seek()
#   if !defined(SEEK_SET)
#     define SEEK_SET   01  /* the offset is relative to the start of the file */
#   endif

#   if !defined(SEEK_CUR)
#     define SEEK_CUR   02  /* the current position indicator */
#   endif

#   if !defined(SEEK_END)
#     define SEEK_END   03  /* end-of-file, respectively */
#   endif

#else /*end win32*/

#   define   AI_HANDLE int //for unix
#   define   AI_ERROR_HANDLE int

#   if !defined(DWORD)
#      define   DWORD  unsigned long
#   endif

#endif

//for invalid handle
#if !defined(AI_INVALID_HANDLE)
#   define   AI_INVALID_HANDLE ((AI_HANDLE)-1)
#endif

// Judge flag
#define F_ISSET( flag, seted ) ( ( flag & seted ) == seted )

/* Define to 1 if you have the `pread' function. */
//#define HAVE_PREAD 1

/* Define to 1 if you have the `pwrite' function. */
//#define HAVE_PWRITE 1

/* Define retry count */
#define AI_RETRY  5

/* retry check macro */
#define RETRY_CHK( op )                                   \
    do                                                    \
    {                                                     \
        int liRetries;                                    \
        for ( liRetries = AI_RETRY;; )                    \
        {                                                 \
            errno = 0;                                    \
            if ( (op) == 0 )                              \
            {                                             \
                break;                                    \
            }                                             \
            if ( ( errno == EAGAIN || errno == EBUSY ||   \
                   errno == EINTR ) && --liRetries > 0 )  \
            {                                             \
                continue;                                 \
            }                                             \
            break;                                        \
        }                                                 \
    } while (0)
   
class AIFile
{
public:
	enum { 
		AI_NO_ERROR       = 0, 
		AI_ERROR          = -1,
        AI_ERROR_UNOPEN   = -2
    };
    
public:
    AIFile();

    ~AIFile();

    /** 
     *  \brief Open a file
     *  \param apcFileName : file name
     *  \param aiFlag : the file status flags, the value of O_RDONLY/O_WRONLY/O_CREAT
     *  \return Successful (int)0 is returned, Otherwise (int)-1
     */
    int Open( const char* apcFileName, int aiFlag = O_RDONLY, int aiMode = 0 );

    /** 
     *  \brief Close a file
     */
    void Close();

    /** 
     *  \brief Seek to a byte offset in the file.
     *  \param atOffset : file offset
     *  \param aiFlag : flag value of SEEK_SET, SEEK_CUR, SEEK_END
     *  \return Successful, the resulting offset, as measured in bytes from the beginning of the file, is returned
     *          Otherwise (off_t)-1 is returned
     */
    off_t Seek( off_t atOffset, int aiFlag = SEEK_SET );

    /** 
     *  \brief Read from a file handle 
     *  \param apsBuff : read buffer
     *  \param size : read size
     *  \return Successful, the number of items read, Otherwise (ssize_t)-1 is returned
     */
    ssize_t Read( void* apBuff, size_t size );

    /** 
     *  \brief Read from a file handle 
     *  \param atOffset : file offset from begin
     *  \param apsBuff : read buffer
     *  \param size : read size
     *  \return Successful, the number of items read, Otherwise (ssize_t)-1 is returned
     */
    ssize_t Read( off_t atOffset, void* apBuff, size_t aiSize );

    /** 
     *  \brief Write to a file handle
     *  \param apsBuff : write buffer
     *  \param size : write size
     *  \return Successful, the number of items write, Otherwise (ssize_t)-1 is returned
     */
    ssize_t Write( const void* apBuff, size_t size );

    /** 
     *  \brief Write to a file handle
     *  \param atOffset : file offset from begin
     *  \param apsBuff : write buffer
     *  \param size : write size
     *  \return Successful, the number of items write, Otherwise (ssize_t)-1 is returned
     */
    ssize_t Write( off_t atOffset, const void* apBuff, size_t aiSize );

    /** 
     *  \brief file size
     *  \return Successful, return size of file, Othrewise (off_t)-1 is returned
     */
    off_t GetSize() const;
    
    AI_HANDLE GetHandle() const;

    /** 
    *   \brief return current status
    *   \return true=OK, false=FAIL
    */
    bool IsFail() const;

    bool IsOpen() const;

    /** 
    *   \brief sync file to disk
    */
    int Sync();

    static int Access( const char *apcPathName, int aiMode );

protected:
    AI_HANDLE ctHandle; //file handle
};

///end namespace
AIBC_NAMESPACE_END

#endif //__AILIBEX__AIFILE_H__

