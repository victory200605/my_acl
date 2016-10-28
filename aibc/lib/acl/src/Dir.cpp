
#include "acl/Dir.h"

ACL_NAMESPACE_START

CDir::CDir(void)
    : mbIsOpened(false)
{
    apl_memset( &this->moDir, 0, sizeof(this->moDir) );
    apl_memset( &this->moStat, 0, sizeof(this->moStat) );
    apl_memset( &this->macDirPath, 0, sizeof(this->macDirPath) );
    apl_memset( &this->macFullName, 0, sizeof(this->macFullName) );
}
    
CDir::~CDir(void)
{
    this->Close();
}

apl_int_t CDir::Open( char const* apcDirPath )
{
    if (apcDirPath == NULL)
    {
        apl_set_errno(APL_EINVAL);
        return -1;
    }
    
    if (this->mbIsOpened)
    {
        apl_set_errno(APL_EBUSY);
        return -1;
    }
    
    //Open dir
    if ( apl_opendir(&this->moDir, apcDirPath) != 0 )
    {
        return -1;
    }
    
    //Save directory path
    apl_snprintf( this->macDirPath, sizeof(this->macDirPath), "%s/", apcDirPath );
    
    this->mbIsOpened = true;
    
    return 0;
}

apl_int_t CDir::Rewind(void)
{
    return apl_rewinddir(&this->moDir);
}

apl_int_t CDir::Tell(void)
{
    return apl_telldir(&this->moDir);
}

apl_int_t CDir::Seek( apl_size_t aiPos )
{
    return apl_seekdir(&this->moDir, aiPos);
}

void CDir::Close(void)
{
    if (this->mbIsOpened)
    {
        apl_closedir(&this->moDir);
        apl_memset( &this->moDir, 0, sizeof(this->moDir) );
        
        this->mbIsOpened = false;
    }
}

apl_int_t CDir::Read(void)
{
    apl_int_t liRetCode = 0;
	
	// Initialize variable
	apl_memset( &this->moStat,    0, sizeof(this->moStat) );
	apl_memset( this->macFullName, 0, sizeof(this->macFullName) );
	
	this->mpoDirent = apl_readdir(&this->moDir);
	if ( this->mpoDirent == NULL )
	{
	    return -1;
	}
	
	//Make full name
	apl_snprintf( this->macFullName, sizeof(this->macFullName), "%s%s", this->macDirPath, this->mpoDirent->d_name );

	if( !this->IsDots() && (liRetCode = apl_stat(this->macFullName, &this->moStat) ) != 0 ) 
	{
		return -1;
	}
	
	return 0;
}

char const* CDir::GetName() const
{
    return this->mpoDirent->d_name;
}

char const* CDir::GetFullName() const
{
    return this->macFullName;
}

char const* CDir::GetPath() const
{
    return this->macDirPath;
}

bool CDir::IsDir() const
{
    return S_ISDIR( this->moStat.ms_stat.st_mode );
}

bool CDir::IsFile() const
{
    return S_ISREG( this->moStat.ms_stat.st_mode );
}

bool CDir::IsDots() const
{
    if( apl_strcmp( this->mpoDirent->d_name, "." ) == 0 
        || apl_strcmp( this->mpoDirent->d_name, ".." ) == 0 )
	{
		return true;
	}
	else
	{
	    return false;
	}
}

apl_int_t CDir::Make( char const* apcDirPath, apl_int_t aiMode )
{    
	char const* lpcPtr = apcDirPath;
	char		lacMkDir[APL_PATH_MAX] = {0};

#define AI_DIR_MAKE_RETURN( path, mode )                    \
    errno = 0;                                              \
    if ( apl_mkdir( path, mode ) != 0 && errno != EEXIST )  \
	{                                                       \
	    return -1;                                          \
	}
	
	if (apcDirPath == NULL || apl_strlen(apcDirPath) >= APL_PATH_MAX )
	{
	    apl_set_errno(APL_EINVAL);
	    return -1;
	}

	while( (lpcPtr = apl_strchr( lpcPtr, '/' )) != NULL )
	{
	    if ( (apl_uint_t)(lpcPtr - apcDirPath) >= (apl_uint_t)APL_PATH_MAX )
	    {
	        apl_set_errno(APL_EINVAL);
	        return -1;
	    }
		else if ( (apl_uint_t)(lpcPtr - apcDirPath) > 0 )
		{
			apl_memcpy(lacMkDir, apcDirPath, lpcPtr - apcDirPath);
			lacMkDir[lpcPtr-apcDirPath] = '\0';
            
            AI_DIR_MAKE_RETURN(lacMkDir, aiMode);
		}

		lpcPtr += 1;
	}

	AI_DIR_MAKE_RETURN(apcDirPath, aiMode);
	
	return 0;
}

apl_int_t CDir::Remove( char const* apcDirPath )
{
    CDir loDir;
    
    if ( loDir.Open(apcDirPath) != 0 )
    {
        return -1;
    }
    
    while( loDir.Read() == 0 )
    {
        if ( loDir.IsDir() )
        {
            if ( CDir::Remove( loDir.GetFullName() ) != 0 )
            {
                return -1;
            }
        }
        else if ( loDir.IsFile() )
        {
            if ( apl_remove( loDir.GetFullName() ) != 0 )
            {
                return -1;
            }
        }
    }
    
    loDir.Close();
    
    if ( apl_rmdir( apcDirPath ) != 0 )
    {
        return -1;
    }
    
    return 0;
}

ACL_NAMESPACE_END
