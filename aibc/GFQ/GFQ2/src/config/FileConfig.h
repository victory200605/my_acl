
#ifndef AIBC_GFQ_FILECONFIG_H
#define AIBC_GFQ_FILECONFIG_H

#include "gfq2/Utility.h"
#include "gfq2/GFQConfig.h"
#include "acl/ThreadManager.h"
#include "aaf/ConfigService.h"

AIBC_GFQ_NAMESPACE_START

/** 
 * @brief Global File Queue file config interface
 */
class CFileConfig : public IGFQConfig
{
public:
    CFileConfig(void);
    
    virtual ~CFileConfig(void);
    
    virtual apl_int_t Initialize( apl_int_t aiArgc, char* apcArgv[] );
    
    virtual void Close(void);
    
    virtual void SetUpdateHandler( UpdateHandlerFunc apfHandler );
    
    virtual bool GetValue( char const* apcField, char const* apcKey, std::string& aoVal );

    virtual bool GetValue( char const* apcField, char const* apcKey, apl_int64_t& aiVal );

    virtual bool GetValue( char const* apcField, char const* apcKey, apl_int32_t& aiVal );
    
    virtual bool IsFieldExisted( char const* apcField );
    
    virtual bool GetTable( char const* apcName, TableType& aoTable );

protected:
    static void* Srv( void* apvParam );

private:
    bool mbIsClosed;
    
    UpdateHandlerFunc mpfUpdateHandler;
    
    std::string moFileName;

    acl::CIniConfig moConfig;

    acl::CThreadManager moThreadManager;
};

AIBC_GFQ_NAMESPACE_END

#endif//AIBC_GFQ_FILECONFIG_H

