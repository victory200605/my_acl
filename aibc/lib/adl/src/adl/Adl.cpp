
#include "adl/Adl.h"
#include "acl/SString.h"
#include "acl/stl/map.h"
#include "acl/Shlib.h"

ADL_NAMESPACE_START

typedef IDatabase* (*CreateFuncType)(std::map<std::string, std::string>&);

IDatabase* CreateDatabase( char const* apcArgv )
{
    acl::CSpliter loSpliter(",");
    acl::CShlib   loShlib;
    std::string   loParameter;
    std::map<std::string, std::string> loParameteres;

    //Set preserve area to support key="..." config
    loSpliter.Preserve('"', '"', false);

    loSpliter.Parse(apcArgv);

    for (apl_size_t luN = 0; luN < loSpliter.GetSize(); luN++)
    {
        loParameter = loSpliter.GetField(luN);

        std::string::size_type luN = loParameter.find('=');
        if  (luN == std::string::npos)
        {
            apl_errprintf("Create database fail, invalid parameter,(%s)\n", loSpliter.GetField(luN) );

            return APL_NULL;
        }

        //Parse parameter key and value
        loParameteres[loParameter.substr(0, luN)] = 
            loParameter.substr(luN + 1, loParameter.length() - luN - 1);
    }

    std::map<std::string, std::string>::iterator loIter = loParameteres.find("lib");
    if (loIter == loParameteres.end() )
    {
        apl_errprintf("Create database fail,please specify the lib path,exemple:lib=liboracle.so\n");

        return APL_NULL;
    }

    if (loShlib.Load(loIter->second.c_str() ) != 0)
    {
        apl_errprintf("Create database fail,%s (path=%s)\n", loShlib.GetDlError(), loIter->second.c_str() );

        return APL_NULL;
    }

    CreateFuncType lpfFunc = loShlib.GetSymbol<CreateFuncType>("CreateDatabase0");
    if (lpfFunc == APL_NULL)
    {
        apl_errprintf("Create database fail,function symbol unexisted\n" );

        return APL_NULL;
    }

    return (*lpfFunc)(loParameteres);
}

void ReleaseDatabase( IDatabase* apoDb )
{
    ACL_DELETE(apoDb);
}

ADL_NAMESPACE_END

