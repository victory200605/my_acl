///////////////////////////////////////////////////////////
//  COracleConn.cpp
//  Implementation of the Class COracleConn
//  Created on:      19-10-2009 14:28:01
//  Original author: hezk
///////////////////////////////////////////////////////////

#include "adl/Adl.h"
#include "OracleImpl.h"


ADL_NAMESPACE_START



///////////////////////////////////////////////////////////////////////////////

COracleConnection::COracleConnection()
    :mpoSvchp(APL_NULL)
    ,mpoAuthinfo(APL_NULL)
    ,mpoEnvhp(APL_NULL)
    ,mpoErrhp(APL_NULL)
    ,mbTransFlag(false)
    ,miErrorCode(0)
{
}

COracleConnection::~COracleConnection()
{
    if (this->mpoErrhp) 
    {
        CheckErr(OCIHandleFree((dvoid *)this->mpoErrhp, 
                    (ub4) OCI_HTYPE_ERROR), 
                "Failed to free errhp");
    }
}

ICursor* COracleConnection::Cursor()
{
    OCIStmt* lpoStmthp = APL_NULL;

    if(CheckErr(OCIHandleAlloc((dvoid *) this->mpoEnvhp, 
                    (dvoid **) &lpoStmthp, OCI_HTYPE_STMT, 
                    (size_t) 0, (dvoid **) 0),
                "Failed to alloc stmthp"))
    {
        return APL_NULL;
    }

    COracleCursor* lpoOracleCursor = APL_NULL;
    ACL_NEW(lpoOracleCursor, COracleCursor);

    if (lpoOracleCursor == APL_NULL)
    {
        return APL_NULL;
    }

    lpoOracleCursor->SetEnvhp(this->mpoEnvhp);
    //cursor will delete stmthp at Close()
    lpoOracleCursor->SetStmthp(lpoStmthp);
    lpoOracleCursor->SetSvchp(this->mpoSvchp);
    lpoOracleCursor->SetOracleConnImpl(this);

    ICursor* lpoICursor = static_cast<ICursor*>(lpoOracleCursor);

    return lpoICursor;
}

//FIXME!! Only the simplest transsion is support here.
//a default transsion had started since the last commit 
//or session begin
apl_int_t COracleConnection::BeginTrans()
{
    //we are already in a transsion
    if (this->mbTransFlag)
    {
        return -1;
    }

    this->mbTransFlag = true;

    return 0;
}

apl_int_t COracleConnection::Commit()
{
    //we are not in a transsion
    if (!this->mbTransFlag)
    {
        return -1;
    }

    this->mbTransFlag = false;

    //FIXME!! in case of error, -1 should be return
    if (CheckErr(OCITransCommit(this->mpoSvchp, 
                    this->mpoErrhp, (ub4) 0), 
                "Failed to commit"))
    {
        return -1;
    }

    return 0;
}

apl_int_t COracleConnection::Rollback()
{
    //we are not in a transsion
    if (!this->mbTransFlag)
    {
        return -1;
    }

    this->mbTransFlag = false;

    if (CheckErr(OCITransRollback(this->mpoSvchp, 
                    this->mpoErrhp, OCI_DEFAULT),
                "Failed to rollback"))
    {
        return -1;
    }

    return 0;
}

apl_int_t COracleConnection::SetCharacters(const char* apcCharacterSet)
{
    this->miErrorCode = -1;

    this->moErrMsg.assign("Not support. Only available for mysql!");

    return -1;
}


//for internal use
void COracleConnection::SetEnvhp(OCIEnv* apoEnvhp)
{
    this->mpoEnvhp = apoEnvhp;

    if (apoEnvhp == APL_NULL)
    {
        return;
    }

    //we can malloc our errhp now
    CheckErr(OCIHandleAlloc(
                (dvoid *) this->mpoEnvhp, (dvoid **) &this->mpoErrhp,
                OCI_HTYPE_ERROR, (size_t) 0,  (dvoid **) 0), 
            "Failed to alloc errhp in oracle conn impl");
}

void COracleConnection::SetSvchp(OCISvcCtx* apoSvchp)
{
    this->mpoSvchp = apoSvchp;
}

OCISvcCtx* COracleConnection::GetSvchp()
{
    return this->mpoSvchp;
}

void COracleConnection::SetAuthinfo(OCIAuthInfo* apoAuthinfo)
{
    this->mpoAuthinfo = apoAuthinfo;
}

OCIAuthInfo* COracleConnection::GetAuthinfo(void)
{
    return this->mpoAuthinfo;
}

bool COracleConnection::IsTransMode()
{
    return this->mbTransFlag;
}

apl_int_t COracleConnection::CheckErr(sword aoStatus, char const* apcLogmsg)
{
    apl_int_t liRet = -1;
    text lacErrbuf[512];
    this->miErrorCode = 0;
    this->moErrMsg.clear();

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
            break;
    }

    return liRet;
}

const char* COracleConnection::GetErrorMsg()
{
    return this->moErrMsg.c_str();
}

apl_int_t COracleConnection::GetErrorCode()
{
    return this->miErrorCode;
}

ADL_NAMESPACE_END
