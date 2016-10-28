
#ifndef AAF_CONFIGSERVER_H
#define AAF_CONFIGSERVER_H

#include "acl/stl/vector.h"
#include "acl/stl/map.h"
#include "acl/IniConfig.h"
#include "acl/XmlConfig.h"
#include "aaf/Server.h"

AAF_NAMESPACE_START

class IConfig;
template<class NodeType> class CFileConfig;

////////////////////////////////////////////////////////////////////////////////////////////////////////
class CConfigServer : public IServer
{
public:
    enum EOption
    {
        DONT_DELETE = 0, ///<Don't delete the config object on exit since we don't own it.
        
        DO_DELETE   = 1, ///<Delete the config object on exit since we don't own it.
    };

protected:
    struct CValueNode
    {
        CValueNode( char const* apcValue = NULL );
        
        bool IsEmpty(void);
        
        const char* ToString(void);
        
        apl_int_t ToInt(void);
        
        double ToDouble(void);
    
    //Attribute
    private:
        std::string moValue;
        apl_int_t   miValue;
        double      mdValue;
        bool        mbIsEmpty;
    };
    
    struct CKeyNode
    {
        std::string moFieldName;
        std::string moKeyName;
        std::string moDefault;
        bool mbIsUpdated;
        bool mbIsReloadable;
        bool mbIsSetedDefault;
        std::vector<CValueNode> moValues;
    };
    
    struct CConfigNode
    {
        typedef std::map<std::string, CKeyNode> KeyMapType;
            
        CConfigNode(void) : mpoConfig(NULL) {}
    
    //Attribute
        IConfig*  mpoConfig;
        
        apl_int_t miOption;
            
        KeyMapType moKeys;
    };

public:
    typedef CValueNode ValueType;
    typedef std::vector<CValueNode> ArrayValueType;
    typedef std::map<std::string, CConfigNode> ConfigMapType;
    
    //Default Config
    typedef CFileConfig<acl::CIniConfig> IniFileType;
    typedef CFileConfig<acl::CXmlConfig> XmlFileType;
   
public:
    CConfigServer(void);
    
    virtual ~CConfigServer(void);
    
    virtual apl_int_t Startup( char const* apcConfigFileName );
    
    virtual apl_int_t Restart(void);
    
    virtual apl_int_t Shutdown(void);
    
    apl_int_t ReloadAll(void);
    
    apl_int_t Reload(char const* apcConfigAlias);
    
    apl_int_t RegisterConfig( char const* apcAlias, IConfig* apoConfig, EOption aeOpt );
    
    apl_int_t RegisterValue(
        char const* apcAlias,
        char const* apcConfigAlias,
        char const* apcFieldName,
        char const* apcKeyName,
        char const* apcDefaultValue = NULL,
        bool abIsReloadable = true );
    
    apl_int_t Update(char const* apcConfigAlias);
    
    apl_int_t UpdateAll(void);
    
    ValueType& GetValue( char const* apcAlias );
    
    ArrayValueType& GetArrayValue( char const* apcAlias );

private:
    CValueNode moEmpty;
    std::vector<CValueNode> moEmptyArray;
        
    ConfigMapType moConfigs;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////
class IConfig
{
public:
    typedef std::vector<CConfigServer::ValueType> ListType;
        
public:
    virtual ~IConfig(void) {}
    
    virtual apl_int_t Reload(void) = 0;
    
    virtual apl_ssize_t GetValues(
        char const* apcFieldPattern,
        char const* apcKey,
        char const* apcDefault,
        ListType& aoList ) = 0;
    
    virtual char const* LastError(void) = 0;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////
template<class NodeType>
class CFileConfig : public IConfig
{
public:
    virtual ~CFileConfig(void);
    
    apl_int_t Open( char const* apcFileName );
    
    virtual apl_ssize_t GetValues(
        char const* apcFieldPattern,
        char const* apcKey,
        char const* apcDefault,
        ListType& aoList );
    
    virtual apl_int_t Reload(void);
    
    virtual char const* LastError(void);

private:
    NodeType moFile;
    
    char macLastError[256];
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//implement
template<class NodeType>
CFileConfig<NodeType>::~CFileConfig(void)
{
}
    
template<class NodeType>
apl_int_t CFileConfig<NodeType>::Open( char const* apcFileName )
{
    apl_int_t liRetCode = this->moFile.Open(apcFileName);
    
    apl_snprintf(this->macLastError, sizeof(this->macLastError), "%s", apl_strerror(apl_get_errno() ) );
    
    return liRetCode;
}

template<class NodeType>
apl_ssize_t CFileConfig<NodeType>::GetValues(
    char const* apcFieldPattern,
    char const* apcKey,
    char const* apcDefault,
    ListType& aoList )
{
    char lacFieldName[APL_NAME_MAX];
    apl_snprintf(lacFieldName, sizeof(lacFieldName), "^%s$", apcFieldPattern );

    this->macLastError[0] = '\0';
    typename NodeType::FieldIterType loFieldIter = this->moFile.Begin(lacFieldName);

    for (; loFieldIter != this->moFile.End(); ++loFieldIter)
    {
        typename NodeType::KeyIterType loKeyIter = loFieldIter.GetValue(apcKey, apcDefault);
        if (loKeyIter.IsEmpty() && apcDefault == NULL)
        {
            apl_snprintf( this->macLastError, sizeof(this->macLastError),
                "key %s.%s no found",
                apcFieldPattern,
                apcKey );
            
            return -1;
        }
        
        aoList.push_back(loKeyIter.ToString() );
    }
    
    return aoList.size();
}

template<class NodeType>
apl_int_t CFileConfig<NodeType>::Reload(void)
{
    apl_int_t liRetCode = this->moFile.Reload();
    
    apl_snprintf(this->macLastError, sizeof(this->macLastError), "%s", apl_strerror(apl_get_errno() ) );
    
    return liRetCode;
}

template<class NodeType>
char const* CFileConfig<NodeType>::LastError(void)
{
    return this->macLastError;
}

AAF_NAMESPACE_END

#endif //AAF_CONFIGSERVER_H
