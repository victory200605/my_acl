//=============================================================================
/**
* \file    AIFile.cpp
* \brief file operator class
*
* Copyright (c) 2007 Asiainfo Technologies(China),Inc.
* RCS: $Id: AIFile.cpp,v 1.2 2009/03/02 13:39:51 daizh Exp $
*
* History
* 2008.01.18 first release by DZH
*/
//=============================================================================
#include "AIFile.h"

///start namespace
AIBC_NAMESPACE_START

AIFile::AIFile():
ctHandle(AI_INVALID_HANDLE) 
{}

AIFile::~AIFile()
{
	this->Close();
}

int AIFile::Open( const char* apcFileName, int aiFlag, int aiMode )
{
	//close file handle and open it again
	this->Close( );

#if defined(WIN32)
	DWORD liAccess = 0, liAttr = 0, liCreateFlag = 0;

	if ( F_ISSET( aiFlag, O_CREAT ) )
		liCreateFlag |= OPEN_ALWAYS;
	else liCreateFlag |= OPEN_EXISTING;

	if ( F_ISSET( aiFlag, O_TRUNC ) )
		liCreateFlag |= CREATE_ALWAYS;	

	if ( F_ISSET( aiFlag, O_RDWR ) )
	{
		liAccess |= GENERIC_READ;
		liAccess |= GENERIC_WRITE;
	}
	else if ( F_ISSET( aiFlag, O_WRONLY ) )
		liAccess |= GENERIC_WRITE;
	else liAccess |= GENERIC_READ;

	liAttr = FILE_ATTRIBUTE_NORMAL;

	ctHandle = ::CreateFile( apcFileName, liAccess, 0, NULL, liCreateFlag, liAttr, 0 );
#else
	ctHandle = ::open( apcFileName, aiFlag, aiMode );
#endif

	if ( ctHandle == AI_INVALID_HANDLE ) 
	{
		return AI_ERROR;
	}

#if defined(WIN32)	
	if ( F_ISSET( aiFlag, O_APPEND ) )
		this->Seek( this->GetSize(), SEEK_SET );
#endif

	return AI_NO_ERROR;
}

void AIFile::Close()
{
	if ( this->IsOpen() )
	{
#if defined(WIN32)
		::CloseHandle( ctHandle );
#else
		::close( ctHandle );
#endif
		ctHandle = AI_INVALID_HANDLE;
	}
}

off_t AIFile::Seek( off_t atOffset, int atWhence )
{
	if ( !this->IsOpen() ) return AI_ERROR_UNOPEN;
#if defined(WIN32)
	/* Yes, this really is how Microsoft designed their API. */
	union {
		__int64 bigint;
		struct {
			unsigned long low;
			long high;
		};
	} luOffbytes;

	int liFlag = 0;
	switch( atWhence )
	{
	case SEEK_SET:
		liFlag = FILE_BEGIN;
		break;
	case SEEK_END:
		liFlag = FILE_END;
		break;
	case SEEK_CUR:
		liFlag = FILE_CURRENT;
		break;
	default:
		liFlag = FILE_BEGIN;
	}

	luOffbytes.bigint = atOffset;

	luOffbytes.low = ::SetFilePointer( ctHandle, luOffbytes.low, &luOffbytes.high, liFlag ) ;

	return ( luOffbytes.low == 0xFFFFFFFF ) ? AI_ERROR : luOffbytes.bigint;
#else
	return ::lseek( ctHandle, atOffset, atWhence );
#endif
}

ssize_t AIFile::Read( void* apBuff, size_t atCount )
{
	if ( !this->IsOpen() ) return AI_ERROR_UNOPEN;
	if ( atCount <= 0 ) return AI_NO_ERROR;

	char* lpcAddr = (char*)apBuff;

	ssize_t liNr = 0; 
	size_t liOffset = 0;
	for ( ; liOffset < atCount; lpcAddr += liNr, liOffset += liNr ) 
	{
#if defined(WIN32)
		BOOL liIsSuccess = false;
		DWORD liCount = 0;
		RETRY_CHK( !( liIsSuccess = ::ReadFile( ctHandle, lpcAddr, (DWORD)(atCount - liOffset), &liCount, NULL) ) );
		if ( liIsSuccess ) liNr = (ssize_t)liCount;
		else liNr = (ssize_t)-1;
#else
		RETRY_CHK( ( ( liNr = ::read( ctHandle, lpcAddr, (DWORD)(atCount - liOffset) ) ) < 0 ? 1 : 0 ) );
#endif
		if ( liNr == 0 )
		{
			//EOF
			break;
		}
		else if ( liNr < 0 )
		{
			//Oh, sorry, read error, return error
			return AI_ERROR;
		}
	}

	return liOffset;
}

ssize_t AIFile::Write( const void* apBuff, size_t atCount )
{
	if ( !this->IsOpen() ) return AI_ERROR_UNOPEN;

	if ( atCount <= 0 ) return AI_NO_ERROR;

	char* lpcAddr = (char*)apBuff;
	ssize_t liNw = 0;
	size_t liOffset = 0;
	for ( ; liOffset < atCount; lpcAddr += liNw, liOffset += liNw ) 
	{
#if defined(WIN32)
		BOOL liIsSuccess = false;
		DWORD liCount = 0;
		RETRY_CHK( !( liIsSuccess = ::WriteFile( ctHandle, lpcAddr, (DWORD)(atCount - liOffset), &liCount, NULL) ) );
		if ( liIsSuccess ) liNw = (ssize_t)liCount;
		else liNw = (ssize_t)-1;
#else
		RETRY_CHK( ( ( liNw = ::write( ctHandle, lpcAddr, (DWORD)(atCount - liOffset) ) ) < 0 ? 1 : 0 ) );
#endif
		if ( liNw == 0 )
		{
			//EOF
			break;
		}
		else if ( liNw < 0 )
		{
			//Oh, sorry, write error, return error
			return AI_ERROR;
		}
	}

	return liOffset;
}

ssize_t AIFile::Read( off_t atOffset, void* apBuff, size_t atCount )
{
	if ( !this->IsOpen() ) return AI_ERROR_UNOPEN;

	if ( atCount <= 0 ) return AI_NO_ERROR;

#if defined(HAVE_PREAD) && defined(HAVE_PWRITE)
	return ::pread( ctHandle, apBuff,  atCount,  atOffset );
#endif
	if ( atOffset >= 0 )
	{
		if ( this->Seek( atOffset ) != atOffset )
		{
			return AI_ERROR;
		}
	}
	return this->Read( apBuff, atCount );
}

ssize_t AIFile::Write( off_t atOffset, const void* apBuff, size_t atCount )
{
	if ( !this->IsOpen() ) return AI_ERROR_UNOPEN;

	if ( atCount <= 0 ) return AI_NO_ERROR;

#if defined(HAVE_PREAD) && defined(HAVE_PWRITE)
	return ::pwrite( ctHandle, apBuff,  atCount,  atOffset );
#endif
	if ( atOffset >= 0 )
	{
		if ( this->Seek( atOffset ) != atOffset )
		{
			return AI_ERROR;
		}
	}
	return this->Write( apBuff, atCount );
}

off_t AIFile::GetSize() const
{
	if ( !this->IsOpen() ) return AI_ERROR_UNOPEN;

#if defined(WIN32)
	union {
		__int64 bigint;
		struct {
			DWORD low;
			DWORD high;
		};
	} ldwSize;

	ldwSize.low = ::GetFileSize ( ctHandle, &ldwSize.high ) ; 

	// If we failed ... 
	if ( ldwSize.low == 0xFFFFFFFF ) return AI_ERROR;
	else return ldwSize.bigint;
#else
	struct stat loBuff;

	if ( ::fstat( ctHandle, &loBuff ) != 0 ) return AI_ERROR;
	else return loBuff.st_size;
#endif
}

AI_HANDLE AIFile::GetHandle() const
{
	return ctHandle;
}

bool AIFile::IsFail() const
{
	if ( ctHandle == AI_INVALID_HANDLE ) return true;
	else return false;
}

bool AIFile::IsOpen() const
{
	if ( ctHandle == AI_INVALID_HANDLE ) return false;
	else return true;
}

int AIFile::Sync()
{
	int liRet = AI_NO_ERROR;
	if ( this->IsOpen() )
	{
#if defined(WIN32)
		if ( !::FlushFileBuffers( ctHandle ) )
		{
			liRet = AI_ERROR;
		}
#else
		liRet = ::fsync( ctHandle );
#endif
	}
	return liRet;
}

int AIFile::Access( const char *apcPathName, int aiMode )
{
#if defined(WIN32)
	return ::access( apcPathName, aiMode & 6 );
#else
	return ::access( apcPathName, aiMode );
#endif
}

///end namespace
AIBC_NAMESPACE_END
