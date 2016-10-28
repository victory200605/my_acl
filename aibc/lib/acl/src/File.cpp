
#include "acl/File.h"

ACL_NAMESPACE_START

CFile::CFile(void)
{
}

CFile::~CFile(void)
{
    this->Close();
}

apl_int_t CFile::Open( char const* apcFileName, apl_int_t aiFlag, apl_int_t aiMode )
{
    if ( this->IsOpened() )
    {
        //Opened
        apl_set_errno(APL_EBUSY);
        return -1;
    }
    
    apl_handle_t liHandle = apl_open( apcFileName, aiFlag, aiMode );
    
    this->SetHandle(liHandle);
    
    return liHandle >= 0 ? 0 : -1;
}

apl_int_t CFile::ShmOpen( char const* apcFileName, apl_int_t aiFlag, apl_int_t aiMode )
{
    if ( this->IsOpened() )
    {
        //Opened
        apl_set_errno(APL_EBUSY);
        return -1;
    }
    
    apl_handle_t liHandle = apl_shm_open( apcFileName, aiFlag, aiMode );
    
    this->SetHandle(liHandle);
    
    return liHandle >= 0 ? 0 : -1;
}

apl_int_t CFile::Truncate( apl_int64_t ai64Length )
{
    return apl_ftruncate(this->GetHandle(), ai64Length);
}

apl_int_t CFile::Allocate( apl_int64_t ai64Offset, apl_int64_t ai64Length )
{
    return apl_fallocate(this->GetHandle(), ai64Offset, ai64Length);
}

apl_int64_t CFile::Seek( apl_int64_t ai64Offset, apl_int_t aiWhence )
{
    return apl_lseek(this->GetHandle(), ai64Offset, aiWhence);
}

apl_int64_t CFile::Tell(void)
{
    return apl_lseek(this->GetHandle(), 0, APL_SEEK_CUR);
}

apl_int_t CFile::Sync(void)
{
    return apl_fsync(this->GetHandle() );
}

apl_ssize_t CFile::Read( void* apvBuffer, apl_size_t aiSize )
{
	apl_ssize_t liResult   = 0; 
	apl_size_t  liCmplSize = 0;
	char*       lpcAddr    = (char*)apvBuffer;
	
	for ( ; liCmplSize < aiSize; lpcAddr += liResult, liCmplSize += liResult ) 
	{
		ACL_RETRY_CHK(
		    (liResult = apl_read(this->GetHandle(), lpcAddr, aiSize - liCmplSize) ) < 0, 5 );
		if ( liResult == 0 )
		{
			//EOF
			break;
		}
		else if ( liResult < 0 )
		{
			//Read error
			return -1;
		}
	}

	return liCmplSize;
}

apl_ssize_t CFile::Read( apl_int64_t ai64Offset, void* apvBuffer, apl_size_t aiSize )
{
    apl_ssize_t liResult   = 0; 
	apl_size_t  liCmplSize = 0;
	char*       lpcAddr    = (char*)apvBuffer;
	
	for ( ; liCmplSize < aiSize; lpcAddr += liResult, liCmplSize += liResult, ai64Offset += liResult ) 
	{
		ACL_RETRY_CHK(
		    (liResult = apl_pread(this->GetHandle(), lpcAddr, aiSize - liCmplSize, ai64Offset) ) < 0, 5 );
		if ( liResult == 0 )
		{
			//EOF
			break;
		}
		else if ( liResult < 0 )
		{
			//Read error
			return -1;
		}
	}

	return liCmplSize;
}

apl_ssize_t CFile::ReadV( apl_iovec_t apoIoVec[], apl_size_t aiN )
{
	apl_ssize_t liResult    = 0;
    apl_size_t  liCmplSize = 0;

    for ( apl_size_t liN = 0; liN < aiN; )
    {
        ACL_RETRY_CHK(
		    (liResult = apl_readv(this->GetHandle(), apoIoVec + liN, aiN - liN) ) < 0, 5 );
		if (liResult == 0)
		{
			//EOF
			break;
		}
		else if (liResult < 0)
		{
			//Read error
			return -1;
		}

        for ( liCmplSize += liResult; liN < aiN
             && liResult >= static_cast<apl_ssize_t>(apoIoVec[liN].iov_len);
              liN++)
        {
            liResult -= apoIoVec[liN].iov_len;
        }
        
        if (liResult != 0)
        {
            char* lpcBase = reinterpret_cast<char*>(apoIoVec[liN].iov_base);
            apoIoVec[liN].iov_base = lpcBase + liResult;
            apoIoVec[liN].iov_len  = apoIoVec[liN].iov_len - liResult;
        }
    }

    return liCmplSize;
}

apl_ssize_t CFile::ReadV( apl_int64_t ai64Offset, apl_iovec_t apoIoVec[], apl_size_t aiN )
{
    if ( this->Seek(ai64Offset, APL_SEEK_SET) != ai64Offset )
    {
        return -1;
    }
    
    return this->ReadV(apoIoVec, aiN);
}

apl_ssize_t CFile::Write( void const* apvBuffer, apl_size_t aiSize)
{
	apl_ssize_t liResult   = 0;
	apl_size_t  liCmplSize = 0;
	char*       lpcAddr    = (char*)apvBuffer;
	
	for ( ; liCmplSize < aiSize; lpcAddr += liResult, liCmplSize += liResult ) 
	{
		ACL_RETRY_CHK(
		    (liResult = apl_write(this->GetHandle(), lpcAddr, aiSize - liCmplSize) ) < 0, 5 );
		if ( liResult < 0 )
		{
			//Write error, return error
			return -1;
		}
	}

	return liCmplSize;
}

apl_ssize_t CFile::Write( apl_int64_t ai64Offset, void const* apvBuffer, apl_size_t aiSize )
{
	apl_ssize_t liResult   = 0;
	apl_size_t  liCmplSize = 0;
	char*       lpcAddr    = (char*)apvBuffer;
	
	for ( ; liCmplSize < aiSize; lpcAddr += liResult, liCmplSize += liResult, ai64Offset += liResult ) 
	{
		ACL_RETRY_CHK(
		    (liResult = apl_pwrite(this->GetHandle(), lpcAddr, aiSize - liCmplSize, ai64Offset) ) < 0, 5 );
		if (liResult < 0)
		{
			//Write error, return error
			return -1;
		}
	}

	return liCmplSize;
}

apl_ssize_t CFile::WriteV( apl_iovec_t const apoIoVec[], apl_size_t aiN)
{
	apl_ssize_t liResult   = 0;
    apl_size_t  liCmplSize = 0;
    
    apl_iovec_t* lpoIoVac = const_cast<apl_iovec_t*>(apoIoVec);

    for ( apl_size_t liN = 0; liN < aiN; )
    {
        ACL_RETRY_CHK(
		    (liResult = apl_writev(this->GetHandle(), lpoIoVac + liN, aiN - liN) ) < 0, 5 );
		if (liResult == 0)
		{
			//EOF
			break;
		}
		else if (liResult < 0)
		{
			//Read error
			return -1;
		}

        for ( liCmplSize += liResult; liN < aiN
             && liResult >= static_cast<apl_ssize_t>(lpoIoVac[liN].iov_len);
              liN++)
        {
            liResult -= lpoIoVac[liN].iov_len;
        }
        
        if (liResult != 0)
        {
            char* lpcBase = reinterpret_cast<char*>(lpoIoVac[liN].iov_base);
            lpoIoVac[liN].iov_base = lpcBase + liResult;
            lpoIoVac[liN].iov_len  = lpoIoVac[liN].iov_len - liResult;
        }
    }

    return liCmplSize;
}

apl_ssize_t CFile::WriteV( apl_int64_t ai64Offset, apl_iovec_t const apoIoVec[], apl_size_t aiN )
{
    if ( this->Seek(ai64Offset, APL_SEEK_SET) != ai64Offset )
    {
        return -1;
    }
    
    return this->WriteV(apoIoVec, aiN);
}

bool CFile::IsOpened() const
{
    return ACL_IS_VALID_HANDLE(this->GetHandle() );
}

apl_int_t CFile::GetFileInfo( FileInfoType& aoFileInfo )
{
    return this->GetFileInfo(&aoFileInfo);
}

apl_int_t CFile::GetFileInfo( FileInfoType* apoFileInfo )
{
    struct apl_stat_t loStat;
    
    if ( apl_fstat(this->GetHandle(), &loStat) != 0 )
    {
        return -1;
    }

    if (apoFileInfo != APL_NULL)
    {
        AssignFileInfo(*apoFileInfo, loStat);
    }

    return 0;
}

apl_int64_t CFile::GetFileSize(void)
{
    FileInfoType loFileInfo;
    
    if (this->GetFileInfo(loFileInfo) != 0)
    {
        return -1;
    }
    else
    {
        return loFileInfo.GetSize();
    }
}

apl_int_t CFile::Access( char const *apcPathName, apl_int_t aiMode )
{
    return apl_access(apcPathName, aiMode);
}
	
apl_int_t CFile::Remove( char const *apcPathName )
{
    return apl_unlink(apcPathName);
}

apl_int_t CFile::ShmRemove( char const *apcPathName )
{
    return apl_shm_unlink(apcPathName);
}

apl_int_t CFile::Rename( char const* apcOld, char const* apcNew )
{
    return apl_rename(apcOld, apcNew);
}

apl_int_t CFile::GetFileInfo( char const* apcPathName, FileInfoType& aoFileInfo )
{
    return CFile::GetFileInfo(apcPathName, &aoFileInfo);
}

apl_int_t CFile::GetFileInfo( char const* apcPathName, FileInfoType* apoFileInfo )
{
    struct apl_stat_t loStat;
    
    if ( apl_lstat(apcPathName, &loStat) != 0 )
    {
        return -1;
    }
    
    if (apoFileInfo != APL_NULL)
    {
        AssignFileInfo(*apoFileInfo, loStat);
    }

    return 0;
}

void CFile::AssignFileInfo( CFileInfo& aoFileInfo, struct apl_stat_t& aoStat )
{
    aoFileInfo.muSize  = aoStat.ms_stat.st_size;
    aoFileInfo.miDev   = aoStat.ms_stat.st_dev;
    aoFileInfo.miIno   = aoStat.ms_stat.st_ino;
    aoFileInfo.miMode  = aoStat.ms_stat.st_mode;
    aoFileInfo.miNLink = aoStat.ms_stat.st_nlink;
    aoFileInfo.miUid   = aoStat.ms_stat.st_uid;
    aoFileInfo.miGid   = aoStat.ms_stat.st_gid;
    aoFileInfo.mi64ATime = aoStat.ms_stat.st_atime;
    aoFileInfo.mi64MTime = aoStat.ms_stat.st_mtime;
    aoFileInfo.mi64CTime = aoStat.ms_stat.st_ctime;
}
 
//////////////////////////////////////// FileInfo ////////////////////////////////
apl_size_t CFileInfo::GetSize(void) const
{
    return this->muSize;
}

apl_int_t CFileInfo::GetDev(void) const
{
    return this->miDev;
}

apl_int_t CFileInfo::GetIno(void) const
{
    return this->miIno;
}

apl_int_t CFileInfo::GetMode(void) const
{
    return this->miMode;
}

apl_int_t CFileInfo::GetNLink(void) const
{
    return this->miNLink;
}

apl_int_t CFileInfo::GetUid(void) const
{
    return this->miUid;
}

apl_int_t CFileInfo::GetGid(void) const
{
    return this->miGid;
}

apl_time_t CFileInfo::GetATime(void) const
{
    return this->mi64ATime;
}

apl_time_t CFileInfo::GetMTime(void) const
{
    return this->mi64MTime;
}

apl_time_t CFileInfo::GetCTime(void) const
{
    return this->mi64CTime;
}
        
ACL_NAMESPACE_END
