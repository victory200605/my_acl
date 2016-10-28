
#include "FileConfig.h"
#include "acl/SString.h"
#include "acl/File.h"

AIBC_GFQ_NAMESPACE_START

CFileConfig::CFileConfig(void)
    : mbIsClosed(false)
    , mpfUpdateHandler(APL_NULL)
{
}
    
CFileConfig::~CFileConfig(void)
{
}
    
apl_int_t CFileConfig::Initialize( apl_int_t aiArgc, char* apcArgv[] )
{
    if (aiArgc != 1)
    {
        apl_errprintf("FileConfig initialize fail, invalid param, (Argc=%"APL_PRIdINT")\n", aiArgc);

        return -1;
    }
    
    this->moFileName = apcArgv[0];

    if (this->moConfig.Open(this->moFileName.c_str() ) != 0)
    {
        apl_errprintf("FileConfig initialize fail, %s\n", apl_strerror(apl_get_errno() ) );
        
        return -1;
    }
    
    if (this->moThreadManager.Spawn(CFileConfig::Srv, this) != 0)
    {
        apl_errprintf("FileConfig initialize fail, %s\n", apl_strerror(apl_get_errno() ) );
        
        return -1;
    }

    return 0;
}

void CFileConfig::Close(void)
{
    this->mbIsClosed = true;
    this->moConfig.Close();

    this->moThreadManager.WaitAll();
}

void CFileConfig::SetUpdateHandler( UpdateHandlerFunc apfHandler )
{
    this->mpfUpdateHandler = apfHandler;
}

bool CFileConfig::GetValue( char const* apcField, char const* apcKey, std::string& aoVal )
{
    acl::CIniConfig::KeyIterType loValue = this->moConfig.GetValue(apcField, apcKey);
    
    if (loValue.IsEmpty() )
    {
        return false;
    }
    else
    {
        aoVal = loValue.ToString();
        
        return true;
    }
}

bool CFileConfig::GetValue( char const* apcField, char const* apcKey, apl_int64_t& aiVal )
{
    acl::CIniConfig::KeyIterType loValue = this->moConfig.GetValue(apcField, apcKey);
    
    if (loValue.IsEmpty() )
    {
        return false;
    }
    else
    {
        aiVal = apl_strtoi64(loValue.ToString(), NULL, 10);
        
        return true;
    }
}

bool CFileConfig::GetValue( char const* apcField, char const* apcKey, apl_int32_t& aiVal )
{
    acl::CIniConfig::KeyIterType loValue = this->moConfig.GetValue(apcField, apcKey);
    
    if (loValue.IsEmpty() )
    {
        return false;
    }
    else
    {
        aiVal = apl_strtoi32(loValue.ToString(), NULL, 10);
        
        return true;
    }
}

bool CFileConfig::IsFieldExisted( char const* apcField )
{
    if (this->moConfig.GetKeyCount(apcField) > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}
    
bool CFileConfig::GetTable( char const* apcName, TableType& aoTable )
{
    acl::CIniConfig::FieldIterType loFieldIter = this->moConfig.Begin(apcName);
    if (loFieldIter != this->moConfig.End() )
    {
        std::vector<std::string> loRow;

        for (acl::CIniConfig::KeyIterType loKeyIter = loFieldIter.Begin();
             loKeyIter != loFieldIter.End(); ++loKeyIter)
        {
            loRow.clear();

            loRow.push_back(loKeyIter.GetName() );

            acl::CSpliter loSpliter(":");

            loSpliter.Parse(loKeyIter.ToString() );

            for (apl_size_t luN = 0; luN < loSpliter.GetSize(); luN++)
            {
                loRow.push_back(loSpliter.GetField(luN) );
            }

            aoTable.push_back(loRow);
        }
    }

    return true;
}

void* CFileConfig::Srv( void* apvParam )
{
    CFileConfig* lpoConfig = static_cast<CFileConfig*>(apvParam);
    apl_time_t   liLastMTime = 0;
    
    while(!lpoConfig->mbIsClosed)
    {
        acl::CFileInfo loInfo;
        
        if (acl::CFile::GetFileInfo(lpoConfig->moFileName.c_str(), loInfo) == 0)
        {
            if (liLastMTime != 0 && loInfo.GetMTime() != liLastMTime)
            {
                if (lpoConfig->moConfig.Reload() != 0)
                {
                    apl_errprintf("FileConfig reload fail, %s\n", apl_strerror(apl_get_errno() ) );
                }
                else if (lpoConfig->mpfUpdateHandler != APL_NULL)
                {
                    (*lpoConfig->mpfUpdateHandler)();
                }
            }

            liLastMTime = loInfo.GetMTime();
        }
        
        apl_sleep(APL_TIME_SEC);
    }
    
    return APL_NULL;
}

AIBC_GFQ_NAMESPACE_END
