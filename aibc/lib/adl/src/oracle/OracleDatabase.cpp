///////////////////////////////////////////////////////////
//  OracleDataBase.cpp
//  Implementation of the Class COracleDatabase
//  Created on:      19-10-2009 14:28:00
//  Original author: hezk
///////////////////////////////////////////////////////////

#include "adl/Adl.h"
#include "adl/OracleDatabase.h"
#include "acl/stl/map.h"
#include "OracleImpl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


ADL_NAMESPACE_START


COracleDatabaseImpl::COracleDatabaseImpl(
        const char* apcUser, 
        const char* apcPasswd, 
        const char* apcHostStr, 
        apl_int_t aiMaxConn)
    :mpoEnvhp(APL_NULL)
    ,mpoErrhp(APL_NULL)
    ,mpoPoolhp(APL_NULL)
    ,mpoPoolName(APL_NULL)
    ,miPoolNameLen(0)
    ,moUser(apcUser)
    ,moPasswd(apcPasswd)
    ,moHostStr(apcHostStr)
    ,miMaxConn(aiMaxConn)
    ,miErrorCode(0)
{
    this->moErrMsg.clear();
}

COracleDatabaseImpl::~COracleDatabaseImpl()
{
    this->Destroy();
}

apl_int_t COracleDatabaseImpl::Init()
{
    return this->InitConnPool(
            moUser.c_str(), 
            moPasswd.c_str(), 
            moHostStr.c_str(), 
            miMaxConn);
}

apl_int_t COracleDatabaseImpl::InitConnPool(
        const char* apcUser, 
        const char* apcPasswd, 
        const char* apcHostStr, 
        apl_int_t aiMaxConn)
{
    apl_int_t liRet = 0;

    do 
    {
        /* Create a thread-safe OCI environment */
        if (CheckErr((OCIEnvCreate((OCIEnv **)&this->mpoEnvhp,
                            (ub4)OCI_THREADED,
                            (dvoid *)0, (dvoid * (*)(dvoid *, size_t))0,
                            (dvoid * (*)(dvoid *, dvoid *, size_t))0,
                            (void (*)(dvoid *, dvoid *))0,
                            (size_t)0, (dvoid **)0)), "Failed to create Oracle env"))
        {
            liRet = -1;
            break;
        }

        if(CheckErr(OCIHandleAlloc( 
                        (dvoid *) this->mpoEnvhp,
                        (dvoid **) &this->mpoErrhp,
                        OCI_HTYPE_ERROR,
                        (size_t) 0, 
                        (dvoid **) 0), "Alloc Errhp failed"))
        {
            liRet = -1;
            break;
        }

        if(CheckErr(OCIHandleAlloc(
                        (dvoid *) this->mpoEnvhp,
                        (dvoid **) &this->mpoPoolhp,
                        OCI_HTYPE_CPOOL,
                        (size_t) 0, 
                        (dvoid **) 0), "Alloc conn poolhp failed"))
        {
            liRet = -1;
            break;
        }

        if(CheckErr(OCIConnectionPoolCreate(this->mpoEnvhp,
                        this->mpoErrhp, this->mpoPoolhp,
                        &this->mpoPoolName, (sb4*)&miPoolNameLen,
                        (text *)apcHostStr, apl_strlen(apcHostStr),
                        (ub4) 1, (ub4) aiMaxConn, (ub4) 1,
                        (text *)apcUser,apl_strlen(apcUser),
                        (text *)apcPasswd,apl_strlen(apcPasswd),
                        OCI_DEFAULT), "Faied to init Oracle connection pool"))
        {
            liRet = -1;
            break;
        }
    }
    while (0);

    //clear 
    if (liRet != 0)
    {
        if (this->mpoPoolhp != APL_NULL) 
        {
            CheckErr(
                    OCIHandleFree((dvoid *)this->mpoPoolhp, 
                        (ub4)OCI_HTYPE_CPOOL),
                    "Failed to free poolhp");
            this->mpoPoolhp = APL_NULL;
        }

        if (this->mpoErrhp != APL_NULL)
        {
            CheckErr(
                    OCIHandleFree((dvoid *)this->mpoErrhp, 
                        (ub4) OCI_HTYPE_ERROR),
                    "Failed to free errhp");
            this->mpoErrhp = APL_NULL;
        }

        if (this->mpoEnvhp != APL_NULL)
        {
            CheckErr(
                    OCIHandleFree((dvoid *)this->mpoEnvhp, 
                        (ub4) OCI_HTYPE_ENV),
                    "Failed to free envhp");
            this->mpoEnvhp = APL_NULL;
        }
    }

    return liRet;
}

void COracleDatabaseImpl::Destroy()
{
    if (this->mpoPoolhp != APL_NULL)
    {
        CheckErr(
                OCIConnectionPoolDestroy(this->mpoPoolhp, 
                    this->mpoErrhp, OCI_DEFAULT), 
                "Failed to destroy conn pool");

        CheckErr(
                OCIHandleFree((dvoid *)this->mpoPoolhp, 
                    (ub4)OCI_HTYPE_CPOOL),
                "Failed to free poolhp");

        this->mpoPoolhp = APL_NULL;
    }

    if (this->mpoErrhp != APL_NULL)
    {
        CheckErr(
                OCIHandleFree((dvoid *)this->mpoErrhp, 
                    (ub4) OCI_HTYPE_ERROR),
                "Failed to free errhp");

        this->mpoErrhp = APL_NULL;
    }

    if (this->mpoEnvhp != APL_NULL)
    {
        CheckErr(
                OCIHandleFree((dvoid *)this->mpoEnvhp, 
                    (ub4) OCI_HTYPE_ENV),
                "Failed to free envhp");

        this->mpoEnvhp = APL_NULL;
    }
}

IConnection* COracleDatabaseImpl::GetConnection(acl::CTimeValue const& aoTimeout)
{
    OCISvcCtx* lpoSvchp = APL_NULL;
    OCIAuthInfo* lpoAuthinfo;

    //alloc authinfo and set attr
    if(CheckErr(OCIHandleAlloc((dvoid *) this->mpoEnvhp, 
                    (dvoid **) &lpoAuthinfo, 
                    OCI_HTYPE_AUTHINFO, 
                    (size_t) 0, (dvoid **) 0), 
                "Failed to alloc authinfo"))
    {
        return APL_NULL;
    }

    ACL_ASSERT(lpoAuthinfo != APL_NULL);

    if(CheckErr(OCIAttrSet((dvoid *)lpoAuthinfo, 
                    (ub4) OCI_HTYPE_AUTHINFO, 
                    (dvoid *)this->moUser.c_str(), (ub4)this->moUser.size(),
                    (ub4) OCI_ATTR_USERNAME, this->mpoErrhp), 
                "Failed to set user for authp"))
    {
        return APL_NULL;
    }

    if(CheckErr(OCIAttrSet((dvoid *)lpoAuthinfo, 
                    (ub4) OCI_HTYPE_AUTHINFO, 
                    (dvoid *)this->moPasswd.c_str(), 
                    (ub4)this->moPasswd.size(),
                    (ub4) OCI_ATTR_PASSWORD, this->mpoErrhp), 
                "Failed to set password for authp"))
    {
        return APL_NULL;
    }

    //thread safe in OCI
    if(CheckErr(OCISessionGet(this->mpoEnvhp, this->mpoErrhp, 
                    &lpoSvchp,
                    lpoAuthinfo,
                    this->mpoPoolName,
                    this->miPoolNameLen,
                    NULL, 0, NULL, NULL, NULL,
                    OCI_SESSGET_CPOOL), 
                "Failed to get session from conn pool"))
    {
        return APL_NULL;
    }

    //create conn
    COracleConnection* lpoOracleConn = APL_NULL;
    ACL_NEW_ASSERT(lpoOracleConn, COracleConnection);

    //set the envhp, svchp and authinfo
    lpoOracleConn->SetEnvhp(this->mpoEnvhp);
    lpoOracleConn->SetSvchp(lpoSvchp);
    lpoOracleConn->SetAuthinfo(lpoAuthinfo);

    IConnection* lpoIConn = static_cast<IConnection*>(lpoOracleConn);

    return lpoIConn;
}

//FIXME!! pool will matain the connection itself
void COracleDatabaseImpl::ReleaseConnection(IConnection* apoIConn)
{
    COracleConnection* lpoOracleConn = 
        static_cast<COracleConnection*>(apoIConn);

    CheckErr(OCISessionRelease(
                lpoOracleConn->GetSvchp(),
                this->mpoErrhp,
                NULL, 0, OCI_DEFAULT),
            "Failed to release session");

    //release authinfo
    OCIAuthInfo* lpoAuthinfo = lpoOracleConn->GetAuthinfo();

    if (lpoAuthinfo)
    {   
        CheckErr(OCIHandleFree((dvoid *)lpoAuthinfo, 
                    (ub4)OCI_HTYPE_AUTHINFO),
                "Failed to free authinfo");
    }   

    //delete connection
    ACL_DELETE(lpoOracleConn);
}

apl_int_t COracleDatabaseImpl::CheckErr(sword aoStatus, char const* apcLogmsg)
{
    apl_int_t liRet = -1;
    text lacErrbuf[512];
    this->miErrorCode = aoStatus;

    switch (aoStatus)
    {
        case OCI_SUCCESS:
            liRet = 0;
            break;
        case OCI_SUCCESS_WITH_INFO:
            this->moErrMsg = "Error - OCI_SUCCESS_WITH_INFO\n";
            break;
        case OCI_NEED_DATA:
            this->moErrMsg = "Error - OCI_NEED_DATA\n";
            break;
        case OCI_NO_DATA:
            this->moErrMsg = "Error - OCI_NODATA\n";
            break;
        case OCI_ERROR:
            (void) OCIErrorGet((dvoid *)this->mpoErrhp, (ub4) 1, (text *) NULL,
                    &miErrorCode, lacErrbuf, 
                    (ub4) sizeof(lacErrbuf), OCI_HTYPE_ERROR);
            this->moErrMsg = (char*)lacErrbuf;
            break;
        case OCI_INVALID_HANDLE:
            this->moErrMsg = "Error - OCI_INVALID_HANDLE\n";
            break;
        case OCI_STILL_EXECUTING:
            this->moErrMsg = "Error - OCI_STILL_EXECUTE\n";
            break;
        case OCI_CONTINUE:
            this->moErrMsg = "Error - OCI_CONTINUE\n";
            break;
        default:
            this->moErrMsg = apcLogmsg;
            break;
    }

    return liRet;
}

const char* COracleDatabaseImpl::GetErrorMsg()
{
    return this->moErrMsg.c_str();
}

apl_int_t COracleDatabaseImpl::GetErrorCode()
{
    return this->miErrorCode;
}

///////////////////////////////////////////////////////////////////////////////

COracleDatabase::COracleDatabase(
        const char* apcUser, 
        const char* apcPasswd, 
        const char* apcHostStr, 
        apl_int_t aiMaxConn )
    :mpoImpl(APL_NULL)
{
    ACL_NEW_ASSERT(this->mpoImpl, COracleDatabaseImpl(
                apcUser, 
                apcPasswd, 
                apcHostStr, 
                aiMaxConn));
}

COracleDatabase::~COracleDatabase()
{
    ACL_DELETE(this->mpoImpl);
}

apl_int_t COracleDatabase::Init()
{
    return this->mpoImpl->Init();
}

void COracleDatabase::Destroy()
{
    this->mpoImpl->Destroy();
}

IConnection* COracleDatabase::GetConnection(acl::CTimeValue const& aoTimeout)
{
    return this->mpoImpl->GetConnection(aoTimeout);
}

void COracleDatabase::ReleaseConnection(IConnection* apoIConn)
{
    this->mpoImpl->ReleaseConnection(apoIConn);
}

const char* COracleDatabase::GetErrorMsg()
{
    return this->mpoImpl->GetErrorMsg();
}

apl_int_t COracleDatabase::GetErrorCode()
{
    return this->mpoImpl->GetErrorCode();
}

apl_int_t COracleDatabase::GetThreadSafety()
{
    return THREAD_SAFETY_CONNECTION;
}

////////////////////////////////////////////////////////////////////////////////////////
//Define for dynamic load database type
extern "C"
{
    IDatabase* CreateDatabase0( std::map<std::string, std::string>& aoParameteres );
}

IDatabase* CreateDatabase0( std::map<std::string, std::string>& aoParameteres )
{
    COracleDatabase* lpoDatabase = APL_NULL;
    std::string loUser;
    std::string loPass;
    std::string loHost;
    apl_int_t   liConnPool = 0;

    std::map<std::string, std::string>::iterator loIter;
    if ( (loIter = aoParameteres.find("user") ) == aoParameteres.end() )
    {
        apl_errprintf("Create database fail,unknown user, example:user=<username>\n");

        return APL_NULL;
    }
    loUser = loIter->second;

    if ( (loIter = aoParameteres.find("pass") ) == aoParameteres.end() )
    {
        apl_errprintf("Create database fail,unknown pass, example:pass=<...>\n");

        return APL_NULL;
    }
    loPass = loIter->second;

    if ( (loIter = aoParameteres.find("host") ) == aoParameteres.end() )
    {
        apl_errprintf("Create database fail,unknown host, example:host=<servername>\n");

        return APL_NULL;
    }
    loHost = loIter->second;

    if ( (loIter = aoParameteres.find("connpool") ) == aoParameteres.end() )
    {
        apl_errprintf("Create database fail,unknown connpool, example:connpool=<num>\n");

        return APL_NULL;
    }
    liConnPool = apl_strtoi32(loIter->second.c_str(), APL_NULL, 10);

    ACL_NEW_ASSERT(lpoDatabase, 
        COracleDatabase(
            loUser.c_str(),
            loPass.c_str(),
            loHost.c_str(),
            liConnPool ) );

    return lpoDatabase;
}


ADL_NAMESPACE_END
