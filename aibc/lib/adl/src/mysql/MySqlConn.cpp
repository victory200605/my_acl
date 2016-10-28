///////////////////////////////////////////////////////////
//  CMySqlConn.cpp
//  Implementation of the Class CMySqlConnection
//  Created on:      19-10-2009 14:28:00
//  Original author: hezk
///////////////////////////////////////////////////////////

//#include "adl/CMySqlConn.h"
//#include "IConnection.h"

#include "adl/Adl.h"
#include "MySqlImpl.h"
#include "mysql/mysql.h"


ADL_NAMESPACE_START


CMySqlConnection::CMySqlConnection()
    :mpoMysqlConn(APL_NULL)
    ,miErrCode(0)
{
}

CMySqlConnection::~CMySqlConnection()
{
}

void CMySqlConnection::SetMysqlConn(MYSQL* apoMysql)
{
    this->mpoMysqlConn = apoMysql;
}

MYSQL* CMySqlConnection::GetMysqlConn()
{
    return this->mpoMysqlConn;
}

#if 0
apl_int_t CMySqlConnecction::Close(){

	return  NULL;
}
#endif

apl_int_t CMySqlConnection::BeginTrans()
{
    this->ClearErrorMsg();

    if (!this->moErrMsg.empty())
    {
        this->moErrMsg.clear();
    }

    //disable autocommit failed
    if (mysql_autocommit(this->mpoMysqlConn, 0))
    {
        this->SetErrorMsg(mysql_error(this->mpoMysqlConn), 
                mysql_errno(this->mpoMysqlConn));
        return -1;
    }

    return 0;
}

apl_int_t CMySqlConnection::Commit()
{
    apl_int_t liRet = 0;

    this->ClearErrorMsg();

    if (mysql_commit(this->mpoMysqlConn))
    {
        liRet = -1;
        this->SetErrorMsg(mysql_error(this->mpoMysqlConn), 
                mysql_errno(this->mpoMysqlConn));
    }

    if (mysql_autocommit(this->mpoMysqlConn, 1))
    { 
        liRet = -1;
        this->SetErrorMsg(mysql_error(this->mpoMysqlConn), 
                mysql_errno(this->mpoMysqlConn));
    }

    return liRet;
}

apl_int_t CMySqlConnection::Rollback()
{
    apl_int_t liRet = 0;

    this->ClearErrorMsg();

    if(mysql_rollback(this->mpoMysqlConn))
    {
        this->SetErrorMsg(mysql_error(this->mpoMysqlConn), 
                mysql_errno(this->mpoMysqlConn));
        liRet = -1;
    }

    if (mysql_autocommit(this->mpoMysqlConn, 1))
    { 
        this->SetErrorMsg(mysql_error(this->mpoMysqlConn), 
                mysql_errno(this->mpoMysqlConn));
        liRet = -1;
    }

    return liRet;
}

ICursor* CMySqlConnection::Cursor()
{
    //const char* lpcCharSet = "SET character_set_results = NULL";
    CMySqlCursor* lpoMysqlCur = new CMySqlCursor;

    if (lpoMysqlCur == APL_NULL)
    {
        return APL_NULL;
    }

    lpoMysqlCur->SetMysqlVConn(this->mpoMysqlConn);

    ICursor* lpoICursor = static_cast<ICursor*>(lpoMysqlCur);

#if 0
    //default, get the raw data from server
    //force server-side does not convert any character set for me 
    if (lpoICursor->Execute(lpcCharSet) != 0)
    {
        this->SetErrorMsg(lpoICursor->GetErrorMsg(),
                lpoICursor->GetErrorCode());
        delete lpoMysqlCur;

        return APL_NULL;
    }
#endif

    return  lpoICursor;
}

apl_int_t CMySqlConnection::SetCharacters(const char* apcCharacterSet)
{
    std::string  loStr = "SET NAMES '";
    CMySqlCursor loCursor;

    loStr.append(apcCharacterSet);
    loStr.append("'");

    loCursor.SetMysqlVConn(this->mpoMysqlConn);

    if (loCursor.Execute(loStr.c_str() ) != 0)
    {
        this->moErrMsg = loCursor.GetErrorMsg();
        this->miErrCode = loCursor.GetErrorCode();

        return -1;
    }
    else
    {
        return 0;
    }
}

void CMySqlConnection::ClearErrorMsg()
{
    this->moErrMsg.clear();
    this->miErrCode = 0;
}

void CMySqlConnection::SetErrorMsg(const char* apcMsg, apl_int_t aiErrCode)
{
    this->moErrMsg = apcMsg;
    this->miErrCode = aiErrCode;
}

const char* CMySqlConnection::GetErrorMsg()
{
    return this->moErrMsg.c_str();
}

apl_int_t CMySqlConnection::GetErrorCode()
{
    return this->miErrCode;
}


ADL_NAMESPACE_END
