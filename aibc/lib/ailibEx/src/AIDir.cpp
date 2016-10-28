//=============================================================================
/**
* \file    AIDir.cpp
* \brief directory interface implement
*
* Copyright (c) 2007 Asiainfo Technologies(China),Inc.
* RCS: $Id: AIDir.cpp,v 1.3 2009/03/02 13:39:51 daizh Exp $
*
* History
* 2008.03.08 first release by DZH
*/
//=============================================================================
#include "AIDir.h"
#include "AILib.h"

#include <errno.h>
#include <assert.h>

///start namespace
AIBC_NAMESPACE_START

AIDir::AIDir() 
    : cpoDir(NULL)
    , cpoResultItem(NULL)
    , cpoItem(NULL)
{
	memset( csDirPath,  0, PATH_MAX );
	memset( csFullName, 0, PATH_MAX );
	memset( csBaseName, 0, PATH_MAX );
	memset( &coStat,    0, sizeof(struct stat) );
	AI_MALLOC_ASSERT( cpoItem, struct dirent, sizeof(struct dirent) + PATH_MAX );
}

AIDir::~AIDir()
{
	if( cpoDir != NULL )
	{
		//close directory
		this->Close();
	}
	
	AI_FREE( cpoItem );
}

int AIDir::Open( const char* apcDirName )
{
	assert( apcDirName != NULL );

    size_t liLen = strlen(apcDirName);
    if ( liLen == 0 )
    {
        //invalid Path
        return AI_ERROR_OPEN_DIR;
    }
    
    //close first
    this->Close();
    
	//save directory path
	if( apcDirName[liLen-1] == '/' )
	{
		snprintf( csDirPath, PATH_MAX, "%s", apcDirName );
	}
	else
	{
		snprintf( csDirPath, PATH_MAX, "%s/", apcDirName );
	}
	
	if( (cpoDir = ::opendir( apcDirName )) == NULL )
	{
		return AI_ERROR_OPEN_DIR;
	}

	return AI_NO_ERROR;
}

int AIDir::Make( const char* apcDirName, int aiMode )
{
    assert( apcDirName != NULL );
    
	const char	*lpcPtr = apcDirName;
	char		lsMkDir[PATH_MAX] = {0};

#define AI_DIR_MAKE_RETURN( path, mode )                    \
    errno = 0;                                              \
    if ( ::mkdir( path, mode ) != 0 && errno != EEXIST )    \
	{                                                       \
	    return AI_ERROR_MAKE_DIR;                           \
	}

	while( (lpcPtr = strchr( lpcPtr, '/' )) != NULL )
	{
		if( ( lpcPtr - apcDirName ) > 0 )
		{
			memcpy( lsMkDir, apcDirName, lpcPtr-apcDirName );
			lsMkDir[lpcPtr-apcDirName] = '\0';
            
            AI_DIR_MAKE_RETURN( lsMkDir, aiMode );
		}

		lpcPtr += 1;
	}

	AI_DIR_MAKE_RETURN( apcDirName, aiMode );
	
	return AI_NO_ERROR;
}

int AIDir::Remove( const char* apcDirName )
{
    AIDir loDir;
    int   liRetCode = 0;
    
    if ( ( liRetCode = loDir.Open(apcDirName) ) != AI_NO_ERROR )
    {
        return liRetCode;
    }
    
    while( loDir.ReadNext() == 0 )
    {
        if ( loDir.IsDir() )
        {
            if ( AIDir::Remove( loDir.GetFullName() ) != 0 )
            {
                return AI_ERROR_REMOVE_DIR;
            }
        }
        else if ( loDir.IsFile() )
        {
            if ( ::remove( loDir.GetFullName() ) != 0 )
            {
                return AI_ERROR_REMOVE_FILE;
            }
        }
    }
    
    loDir.Close();
    
    if ( ::remove( apcDirName ) != 0 )
    {
        return AI_ERROR_REMOVE_FILE;
    }
    
    return AI_NO_ERROR;
}

void AIDir::Rewind()
{
	rewinddir(cpoDir);
}

void AIDir::Close()
{
	if( cpoDir != NULL )
	{
		//close it
		::closedir( cpoDir );
		cpoDir = NULL;
	}
}

int AIDir::ReadNext()
{
	int	liRetCode = AI_NO_ERROR;
	
	// Initialize variable
	memset( &coStat,    0, sizeof(struct stat) );
	memset( csFullName, 0, PATH_MAX );
	memset( csBaseName, 0, PATH_MAX );
	
	if( cpoDir == NULL ) 
	{
		return AI_ERROR_OPEN_DIR;
	}

	if( (liRetCode = ::readdir_r( cpoDir, cpoItem, &cpoResultItem )) != 0 )
	{
		return AI_ERROR_READ_DIR;
	}

	if( cpoResultItem == NULL )
	{
		return AI_WARN_END_OF_DIR;
	}
	
	// Save file name
    strncpy( csBaseName, cpoResultItem->d_name, PATH_MAX - 1 );
	snprintf( csFullName, PATH_MAX, "%s%s", csDirPath, csBaseName );

	if( !this->IsDots() && (liRetCode = ::stat( csFullName, &coStat ) ) != 0 ) 
	{
		return AI_ERROR_READ_FILE;
	}
	
	return AI_NO_ERROR;
}

const char* AIDir::GetName() const
{
	return csBaseName;
}

const char* AIDir::GetFullName() const
{
	return csFullName;
}

const char* AIDir::GetDirPath() const
{
	return csDirPath;
}

bool AIDir::IsDir() const
{
	return S_ISDIR( coStat.st_mode );
}

bool AIDir::IsFile() const
{
	return S_ISREG( coStat.st_mode );
}

bool AIDir::IsDots() const
{
	if( ::strcmp( csBaseName, "." ) == 0 || ::strcmp( csBaseName, ".." ) == 0 )
	{
		return true;
	}
	
	return false;
}

///end namespace
AIBC_NAMESPACE_END
