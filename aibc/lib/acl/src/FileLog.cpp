
#include "acl/FileLog.h"

ACL_NAMESPACE_START

const apl_int_t CFileLog::MAX_LOG_BUFFER_SIZE = 1024;

CFileLog::CFileLog( CAnyLogFormat aoFormat )
    : moFormat(aoFormat)
{
    apl_memset( this->macFileName, 0, sizeof(this->macFileName) );
}

apl_int_t CFileLog::Open( char const* apcFileName )
{
    apl_int_t liRetCode = 0;
    
    apl_strncpy(this->macFileName, apcFileName, sizeof(this->macFileName) );

    if ( (liRetCode = this->moFile.Open(apcFileName, APL_O_CREAT | APL_O_WRONLY, 0600) ) == 0 )
    {
        if ( this->moFile.Seek(0, APL_SEEK_END) < 0 )
        {
            this->moFile.Close();
            
            return -1;
        }
    }

    return liRetCode;
}
    
void CFileLog::Close( void )
{
    this->moFile.Close();
}

apl_int_t CFileLog::Write( ELLType aeLevel, char const* apcFormat, ... )
{
    va_list    loVaList;
    char       lsBuffer[MAX_LOG_BUFFER_SIZE] = {0};
    apl_size_t liRetSize = 0;
    
    va_start(loVaList, apcFormat);

    liRetSize = this->moFormat( aeLevel, apcFormat, loVaList, lsBuffer, sizeof(lsBuffer) );

    va_end(loVaList);
    
    return this->Write(lsBuffer, liRetSize);
}

apl_int_t CFileLog::Write( char const* apcBuffer, apl_size_t aiSize )
{
    TSmartLock<CLock> loLock(this->moLock);
    
    return this->moFile.Write(apcBuffer, aiSize);
}
    
apl_int_t CFileLog::Backup( char const* apcPath )
{
    apl_int_t liRetCode1 = 0;
    apl_int_t liRetCode2 = 0;
    
    TSmartLock<CLock> loLock(this->moLock);
    
    this->Close();
     
    liRetCode1 = CFile::Rename(this->macFileName, apcPath);
    
    liRetCode2 = this->Open(this->macFileName);
    
    return liRetCode1 == 0 ? liRetCode2 : liRetCode1;
}
    
apl_ssize_t CFileLog::GetSize( void )
{
    CFile::FileInfoType loFileInfo;
    
    if ( this->moFile.GetFileInfo(loFileInfo) != 0 )
    {
        return -1;
    }
    else
    {
        return loFileInfo.GetSize();
    }
}

ACL_NAMESPACE_END
