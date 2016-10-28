
#include "aaf/ConfigService.h"

AAF_NAMESPACE_START

CConfigService::CValueNode::CValueNode( char const* apcValue )
    : mbIsEmpty(apcValue == NULL ? true : false)
{
    if (apcValue != NULL)
    {
        this->moValue = apcValue;
        this->miValue = apl_strtoi32(apcValue, NULL, 10);
        this->mdValue = strtod(apcValue, NULL);
    }
}

bool CConfigService::CValueNode::IsEmpty(void)
{
    return this->mbIsEmpty;
}

const char* CConfigService::CValueNode::ToString(void)
{
    return this->moValue.c_str();
}
        
apl_int_t CConfigService::CValueNode::ToInt(void)
{
    return this->miValue;
}
        
double CConfigService::CValueNode::ToDouble(void)
{
    return this->mdValue;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CConfigService::CConfigService(void)
{
}

CConfigService::~CConfigService(void)
{
    this->Shutdown();
}
    
apl_int_t CConfigService::Startup( char const* apcConfigFileName )
{
    IniFileType* lpoMain = NULL;
    ACL_NEW_ASSERT(lpoMain, IniFileType);
    
    if (this->RegisterConfig("Main", lpoMain, DO_DELETE) != 0)
    {
        ACL_DELETE(lpoMain);

        return -1;
    }
    
	if ( lpoMain->Open(apcConfigFileName) != 0 )
	{
	    apl_errprintf("ConfigService open config %s fail, %s\n", apcConfigFileName, lpoMain->LastError() );
	    
	    return -1;
	}
    else
    {
        return 0;
    }
}
 
apl_int_t CConfigService::Restart(void)
{
    return this->ReloadAll();
}
    
apl_int_t CConfigService::Shutdown(void)
{
    for (ConfigMapType::iterator loIter = this->moConfigs.begin();
        loIter != this->moConfigs.end(); ++loIter )
    {
        if (loIter->second.miOption == DO_DELETE)
        {
            ACL_DELETE(loIter->second.mpoConfig);
        }
    }
    
    this->moConfigs.clear();
    
    return 0;
}

apl_int_t CConfigService::ReloadAll(void)
{
    for (ConfigMapType::iterator loIter = this->moConfigs.begin();
        loIter != this->moConfigs.end(); ++loIter )
    {
        if ( this->Reload(loIter->first.c_str() ) != 0 )
        {
            return -1;
        }
    }
    
    return 0;
}

apl_int_t CConfigService::Reload(char const* apcConfigAlias)
{
    ConfigMapType::iterator loIter = this->moConfigs.find(apcConfigAlias);
    if (loIter == this->moConfigs.end() )
    {
        apl_errprintf("ConfigService reload config %s no found\n", apcConfigAlias );
        
        return -1;
    }
    
    if (loIter->second.mpoConfig->Reload() != 0)
    {
        apl_errprintf(
            "ConfigService reload config %s fail, %s\n",
            apcConfigAlias,
            loIter->second.mpoConfig->LastError() );
        
        return -1;
    }
    
    return 0;
}

apl_int_t CConfigService::RegisterConfig( char const* apcAlias, IConfig* apoConfig, EOption aeOpt )
{
    CConfigNode& loConfig = this->moConfigs[apcAlias];
    
    if ( loConfig.mpoConfig == NULL )
    {
        loConfig.mpoConfig = apoConfig;
        loConfig.miOption  = aeOpt;
        return 0;
    }
    else
    {
        apl_errprintf("ConfigService re-register config %s\n", apcAlias);
                
        return -1;
    }
}
    
apl_int_t CConfigService::RegisterValue(
        char const* apcAlias,
        char const* apcConfigAlias,
        char const* apcFieldName,
        char const* apcKeyName,
        char const* apcDefaultValue,
        bool abIsReloadable )
{
    ConfigMapType::iterator loIter = this->moConfigs.find(apcConfigAlias);
    if (loIter == this->moConfigs.end() )
    {
        apl_errprintf("ConfigService register config %s no found\n", apcConfigAlias);
        
        return -1;
    }
    else
    {
        CKeyNode& loKeyNode = loIter->second.moKeys[apcAlias];
        if (loKeyNode.moFieldName.length() > 0)
        {
            apl_errprintf("ConfigService re-register key %s.%s\n", apcConfigAlias, apcAlias);
                        
            return -1;
        }
        else
        {
            loKeyNode.moFieldName = apcFieldName;
            loKeyNode.moKeyName   = apcKeyName;
            if (apcDefaultValue != NULL)
            {
                loKeyNode.moDefault  = apcDefaultValue;
                loKeyNode.mbIsSetedDefault = true;
            }
            else
            {
                loKeyNode.mbIsSetedDefault = false;
            }
            loKeyNode.mbIsUpdated = false;
            loKeyNode.mbIsReloadable = abIsReloadable;
            
            return 0;
        }
    }
}

apl_int_t CConfigService::UpdateAll(void)
{
    for (ConfigMapType::iterator loIter = this->moConfigs.begin();
        loIter != this->moConfigs.end(); ++loIter )
    {
        if ( this->Update( loIter->first.c_str() ) != 0 )
        {
            return -1;
        }
    }
    
    return 0;
}
    
apl_int_t CConfigService::Update(char const* apcConfigAlias)
{
    apl_ssize_t liResult = 0;
    
    ConfigMapType::iterator loConfigIter = this->moConfigs.find(apcConfigAlias);
    if (loConfigIter == this->moConfigs.end() )
    {
        apl_errprintf("ConfigService update config %s no found\n", apcConfigAlias);

        return -1;
    }
    else
    {
        CConfigNode::KeyMapType::iterator loIter = loConfigIter->second.moKeys.begin();
        for (; loIter != loConfigIter->second.moKeys.end(); ++loIter)
        {
            if (loIter->second.mbIsUpdated && !loIter->second.mbIsReloadable)
            {
                continue;
            }
            
            std::vector<CValueNode> loTempValues;
            if (loIter->second.mbIsSetedDefault)
            {
                liResult = loConfigIter->second.mpoConfig->GetValues(
                    loIter->second.moFieldName.c_str(),
                    loIter->second.moKeyName.c_str(),
                    loIter->second.moDefault.c_str(),
                    loTempValues );
                    
                
            }
            else
            {
                liResult = loConfigIter->second.mpoConfig->GetValues(
                    loIter->second.moFieldName.c_str(),
                    loIter->second.moKeyName.c_str(),
                    NULL,
                    loTempValues );
            }
            
            if (liResult < 0 )
            {
                apl_errprintf(
                    "ConfigService update get values fail, %s\n",
                        loConfigIter->second.mpoConfig->LastError() );
                    
                return -1;
            }
            
            if (loTempValues.size() == 0)
            {
                apl_errprintf(
                    "ConfigService update field %s no found\n",
                    loIter->second.moFieldName.c_str() );
                        
                return -1;
            }
            
            loIter->second.moValues.swap(loTempValues);
            loIter->second.mbIsUpdated = true;
        }
        
        return 0;
    }
}
    
CConfigService::ValueType& CConfigService::GetValue( char const* apcAlias )
{
    ArrayValueType& loArray = this->GetArrayValue(apcAlias);
    
    if (loArray.size() == 0)
    {
        return this->moEmpty;
    }
    else
    {
        return loArray[0];
    }
}

CConfigService::ArrayValueType& CConfigService::GetArrayValue( char const* apcAlias )
{
    char  lacConfigName[APL_NAME_MAX] = {0};
    char* lpcKeyAlias = NULL;
    
    apl_strncpy(lacConfigName, apcAlias, sizeof(lacConfigName) );
    
    if ( ( lpcKeyAlias = apl_strchr(lacConfigName, '.') ) == NULL )
    {
        return this->moEmptyArray;
    }
    else
    {
        *lpcKeyAlias++ = '\0';
    }
    
    ConfigMapType::iterator loConfigIter = this->moConfigs.find(lacConfigName);
    if (loConfigIter == this->moConfigs.end() )
    {
        return this->moEmptyArray;
    }
    else
    {
        CConfigNode::KeyMapType::iterator loIter = loConfigIter->second.moKeys.find(lpcKeyAlias);
        if (loIter == loConfigIter->second.moKeys.end() )
        {
            return this->moEmptyArray;
        }
        
        return loIter->second.moValues;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
CConfigFileHandler::CConfigFileHandler( const char* apcFileName )
    : mbIsOpened(false)
    , moName(apcFileName)
{
}

CConfigFileHandler::~CConfigFileHandler(void)
{
}

apl_int_t CConfigFileHandler::HandleUpdate(void)
{
    if (!this->mbIsOpened)
    {
        if (this->moConfigFile.Open(this->moName.c_str() ) != 0)
        {
            return -1;
        }

        this->mbIsOpened = true;
    }
    else
    {
        this->moConfigFile.Reload();
    }

    return this->BindAll();
}

bool CConfigFileHandler::GetValue( char const* apcField, char const* apcKey, std::string& aoVal )
{
    acl::CIniConfig::KeyIterType loIter = this->moConfigFile.GetValue(apcField, apcKey);
    if (!loIter.IsEmpty() )
    {
        aoVal = loIter.ToString();
        return true;
    }
    else
    {
        return false;
    }
}

bool CConfigFileHandler::GetValue( char const* apcField, char const* apcKey, apl_int64_t& aiVal )
{
    acl::CIniConfig::KeyIterType loIter = this->moConfigFile.GetValue(apcField, apcKey);
    if (!loIter.IsEmpty() )
    {
        aiVal = apl_strtoi64(loIter.ToString(), NULL, 10);
        return true;
    }
    else
    {
        return false;
    }
}

bool CConfigFileHandler::GetValue( char const* apcField, char const* apcKey, apl_int32_t& aiVal )
{
    acl::CIniConfig::KeyIterType loIter = this->moConfigFile.GetValue(apcField, apcKey);
    if (!loIter.IsEmpty() )
    {
        aiVal = apl_strtoi32(loIter.ToString(), NULL, 10);
        return true;
    }
    else
    {
        return false;
    }
}

bool CConfigFileHandler::IsFieldExisted( char const* apcField )
{
    if (this->moConfigFile.GetKeyCount(apcField) > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

acl::CIniConfig& CConfigFileHandler::GetConfigFile(void)
{
    return this->moConfigFile;
}

AAF_NAMESPACE_END

