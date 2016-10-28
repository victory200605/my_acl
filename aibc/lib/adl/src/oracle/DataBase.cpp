///////////////////////////////////////////////////////////
//  OracleDataBase.cpp
//  Implementation of the Class CDataBase
//  Created on:      19-10-2009 14:28:00
//  Original author: hezk
///////////////////////////////////////////////////////////

#include "adl/Adl.h"
#include "OracleImpl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


ADL_NAMESPACE_START

///////////////////////////////////////////////////////////////////////////////
CDataBase::CDataBase(
        const char* apcUser, 
        const char* apcPasswd, 
        const char* apcHostStr, 
        apl_int_t aiMaxConn,
        const char* apcDBName,
        apl_int_t aiPort)
    :mpoImpl(APL_NULL)
{
    this->mpoImpl = (CDataBaseImpl*)new COracleDatabaseImpl(
                apcUser, 
                apcPasswd, 
                apcHostStr, 
                aiMaxConn);
    assert(this->mpoImpl != APL_NULL);
}

CDataBase::~CDataBase()
{
    delete (COracleDatabaseImpl*)this->mpoImpl;
}

apl_int_t CDataBase::Init()
{
    return ((COracleDatabaseImpl*)this->mpoImpl)->Init();
}

void CDataBase::Destroy()
{
    ((COracleDatabaseImpl*)this->mpoImpl)->Destroy();
}

IConnection* CDataBase::GetConnection(acl::CTimeValue const& aoTimeout)
{
    return ((COracleDatabaseImpl*)this->mpoImpl)->GetConnection(aoTimeout);
}

void CDataBase::ReleaseConnection(IConnection* apoIConn)
{
    ((COracleDatabaseImpl*)this->mpoImpl)->ReleaseConnection(apoIConn);
}

const char* CDataBase::GetErrorMsg()
{
    return ((COracleDatabaseImpl*)this->mpoImpl)->GetErrorMsg();
}

apl_int_t CDataBase::GetErrorCode()
{
    return ((COracleDatabaseImpl*)this->mpoImpl)->GetErrorCode();
}

apl_int_t CDataBase::GetThreadSafety()
{
    return THREAD_SAFETY_CONNECTION;
}


ADL_NAMESPACE_END
