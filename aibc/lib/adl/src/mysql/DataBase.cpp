///////////////////////////////////////////////////////////
//  MySqlDataBase.cpp
//  Implementation of the Class CDataBase
//  Created on:      19-10-2009 14:27:59
//  Original author: hezk
///////////////////////////////////////////////////////////

#include "adl/Adl.h"
#include "MySqlImpl.h"

ADL_NAMESPACE_START

////////////////////////////////////////////////////////////////////////////////
CDataBase::CDataBase(
            const char* apcUser, 
            const char* apcPasswd, 
            const char* apcHostStr, 
            apl_int_t aiMaxConn, 
            const char* apcDBName,
            apl_int_t aiPort)
{
    this->mpoImpl = (CDataBaseImpl*)new CMySqlDatabaseImpl(
            apcUser,
            apcPasswd, 
            apcHostStr, 
            aiMaxConn, 
            apcDBName,
            aiPort);

    assert(this->mpoImpl != APL_NULL);
}

CDataBase::~CDataBase()
{
    delete (CMySqlDatabaseImpl*)this->mpoImpl;
}

apl_int_t CDataBase::Init()
{
    return ((CMySqlDatabaseImpl*)this->mpoImpl)->Init();
}

void CDataBase::Destroy()
{
    ((CMySqlDatabaseImpl*)this->mpoImpl)->Destroy();
}

IConnection* CDataBase::GetConnection(acl::CTimeValue const& aoTimeout)
{
    return ((CMySqlDatabaseImpl*)this->mpoImpl)->GetConnection(aoTimeout);
}

void CDataBase::ReleaseConnection(IConnection* apoIConn)
{
    ((CMySqlDatabaseImpl*)this->mpoImpl)->ReleaseConnection(apoIConn);
}

const char* CDataBase::GetErrorMsg()
{
    return ((CMySqlDatabaseImpl*)this->mpoImpl)->GetErrorMsg();
}

apl_int_t CDataBase::GetErrorCode()
{
    return ((CMySqlDatabaseImpl*)this->mpoImpl)->GetErrorCode();
}

apl_int_t CDataBase::GetThreadSafety()
{
    return THREAD_SAFETY_MODULE;
}


ADL_NAMESPACE_END
