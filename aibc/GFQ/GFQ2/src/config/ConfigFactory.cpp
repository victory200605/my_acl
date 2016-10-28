
#include "ConfigFactory.h"
#include "FileConfig.h"
#include "acl/Shlib.h"
#include "acl/SString.h"

AIBC_GFQ_NAMESPACE_START

typedef IGFQConfig* (*CreateConfigFunc)(void);

IGFQConfig* CConfigFactory::Create( char const* apcCmd )
{
    acl::CSpliter loSpliter(":");
    apl_int_t     liArgc = 0;
    char const*   lpcArgv[100] = {0};
    IGFQConfig*   lpoConfigImpl = APL_NULL;
        
    loSpliter.Parse(apcCmd);
    if (loSpliter.GetSize() == 0 || apl_strcmp(loSpliter.GetField(0), "default") == 0)
    {
        //Load from default file config
        ACL_NEW_ASSERT(lpoConfigImpl, CFileConfig);
    }
    else
    {
        acl::CShlib loShlib;
       
        //Load from dynamic library
        if (loShlib.Load(loSpliter.GetField(0) ) != 0)
        {
            apl_errprintf("Shlib Load fail, %s, (Lib=%s)\n", loShlib.GetDlError(), loSpliter.GetField(0) );
            
            return APL_NULL;
        }
        
        CreateConfigFunc lpfCreator = loShlib.GetSymbol<CreateConfigFunc>("CreateConfig");
        if (lpfCreator == APL_NULL)
        {
            apl_errprintf("Shlib GetSymbol fail, %s\n, (Lib=%s)", loShlib.GetDlError(), loSpliter.GetField(0) );
    
            return APL_NULL;
        }
        
        lpoConfigImpl = (*lpfCreator)();
        ACL_ASSERT(lpoConfigImpl != APL_NULL);
    }

    //Make argv array
    for (apl_size_t luN = 1; luN < loSpliter.GetSize() && luN <= 100; luN++)
    {
        lpcArgv[liArgc++] = loSpliter.GetField(luN);
    }

    //Initialize config implement
    if (lpoConfigImpl->Initialize(liArgc, (char**)lpcArgv) != 0)
    {
        CConfigFactory::Destroy(lpoConfigImpl);

        return APL_NULL;
    }
    
    return lpoConfigImpl;
}

void CConfigFactory::Destroy( IGFQConfig* apoConfigImpl )
{
    ACL_DELETE(apoConfigImpl);
}

AIBC_GFQ_NAMESPACE_END

