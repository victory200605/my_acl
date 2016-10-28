///////////////////////////////////////////////////////////
//  CMySqlCursor.cpp
//  Implementation of the Class CMySqlCursor
//  Created on:      19-10-2009 14:28:00
//  Original author: hezk
///////////////////////////////////////////////////////////

#include "adl/Adl.h"
#include "MySqlImpl.h"


ADL_NAMESPACE_START

////////////////////////////////////////////////////////////////////////////////

CMySqlCursor::CMySqlCursor()
    :mpoMysqlConn(APL_NULL) 
    ,miErrorCode(0)
    ,muAffectedRows(0)
    ,mpoStmt(APL_NULL)
    ,mpoBindOutput(APL_NULL)
    ,mpuBindLenOutput(APL_NULL)
    ,miLastExecute(0)
    ,mpoMysqlRes(APL_NULL)
    ,muPreFetchRows(1)
    ,muRowCounts(0)
{
    ACL_NEW_ASSERT(this->mpoParamBind, CMySqlParamBind)
}

CMySqlCursor::~CMySqlCursor()
{
    ACL_DELETE(this->mpoParamBind)

    if (this->mpoStmt != APL_NULL)
    {
        mysql_stmt_close(this->mpoStmt);
        this->mpoStmt = APL_NULL;
    }
}

void CMySqlCursor::Close()
{
    //free the result mem
    this->ClearResults();

    delete this;
}

//stmt execute, need to prepare before
apl_int_t CMySqlCursor::Execute()
{
    MYSQL_FIELD* lpoFields = APL_NULL;
    apl_size_t luIter = 0; 
    apl_size_t luFieldCount = 0;
    apl_size_t luLenOfField = 0;
    std::vector<char*> loBindResultVt;
    MYSQL_BIND* lpoBindCur = APL_NULL;
    apl_int_t liRet = 0;
    MYSQL_STMT* lpoStmt = APL_NULL;
    char* lpcResult = APL_NULL;
    //apl_int_t liCount;
    CParamBind* lpoParamBind = APL_NULL;

    lpoStmt = this->GetMysqlStmt();
    lpoParamBind = this->GetParamBind();
    //not prepare yet
    if (lpoStmt == APL_NULL 
            || lpoParamBind == APL_NULL)
    {
        return -1;
    }

    //clear the cursor
    this->ClearCursor();
    this->miLastExecute = LAST_EXECUTE_NOTHING;

    do 
    {
        //update the content len which bind in prepare
        if (lpoParamBind->UpdateParamContentLen())
        {
            liRet = -2;
            break;
        }

        if (mysql_stmt_execute(lpoStmt))
        {
            liRet = -1;
            break;
        }

        this->SetAffectedRows(mysql_stmt_affected_rows(lpoStmt));

        //get the result set if any!!!!!
        this->mpoMysqlRes = mysql_stmt_result_metadata(lpoStmt);

        if (this->mpoMysqlRes == APL_NULL) { break; }

        luFieldCount =  mysql_num_fields(this->mpoMysqlRes);

        if (luFieldCount == 0) { break; }

        lpoFields = mysql_fetch_fields(this->mpoMysqlRes);

        for ( luIter = 0; luIter < luFieldCount; luIter ++ ) 
        { 
            if (IS_NUM(lpoFields[luIter].type))
            {
                //FIXME!! 21 is large enough to store a 64bit NUN's string 
                //format e.g. 18446744073709551616 (20 + "-")
                //24 should enough to store double
                //e.g. 8.1234560000000009E+000
                luLenOfField = 24;
            }
            else 
            {
                luLenOfField = lpoFields[luIter].length;
            }

            this->moCurResultSet.AddFieldName(lpoFields[luIter].name, 
                    luLenOfField);
        }   

        this->mpoBindOutput = new MYSQL_BIND[luFieldCount];
        this->mpuBindLenOutput = new apl_uint_t[luFieldCount];

        if (this->mpoBindOutput == APL_NULL 
                || this->mpuBindLenOutput == APL_NULL) 
        {
            liRet = -2; 
            break;
        }

        apl_memset(this->mpoBindOutput , 0, sizeof(MYSQL_BIND)*luFieldCount);
        apl_memset(this->mpuBindLenOutput , 0, 
                sizeof(apl_int32_t)*luFieldCount);

        //bind all the fields 
        for (luIter = 0; luIter < luFieldCount; ++luIter)
        {
            luLenOfField = this->moCurResultSet.GetFieldNameWidth(luIter);
            lpcResult = (char*)apl_malloc(luLenOfField +1);

            if (lpcResult == APL_NULL) 
            {
                liRet = -2;
                break;
            }

            this->moBindContentOutputVt.push_back(lpcResult);
            apl_memset(lpcResult, 0, luLenOfField +1);

            //bind 
            lpoBindCur = this->mpoBindOutput + luIter;
            lpoBindCur->buffer_type = MYSQL_TYPE_STRING;
            lpoBindCur->buffer = lpcResult;
            lpoBindCur->buffer_length = luLenOfField +1;
            lpoBindCur->length = 
                (unsigned long*)(this->mpuBindLenOutput + luIter);
        }

        if (liRet != 0) { break; }

        if (mysql_stmt_bind_result(lpoStmt, this->mpoBindOutput ))
        {
            liRet = -1;
            break;
        }

        if (mysql_stmt_store_result(lpoStmt))
        {
            liRet = -1;
            break;
        }

        //get how many rows of result set
        this->muRowCounts = mysql_stmt_num_rows(lpoStmt);

        //do not fetch
        this->miLastExecute = LAST_EXECUTE_STMT;
    }
    while (0);

    if (liRet == -1)
    {
        this->SetErrorMsg(mysql_stmt_error(lpoStmt), 
                mysql_stmt_errno(lpoStmt));
    }

    return liRet;
}

//common execute
apl_int_t CMySqlCursor::Execute(const char* apcSQL)
{
    apl_int_t liRet = 0;
    apl_size_t luIter;
    MYSQL_FIELD* loFields;

    //clear the cursor
    this->ClearCursor();
    this->miLastExecute = LAST_EXECUTE_NOTHING;

    do 
    {
        apl_size_t luFieldCount = 0;
        liRet = mysql_real_query(
                this->mpoMysqlConn, 
                apcSQL, 
                apl_strlen(apcSQL));

        if (liRet)
        {
            liRet = -1;
            break;
        }

        this->SetAffectedRows(mysql_affected_rows(this->mpoMysqlConn));

        //get the result here if any
        this->mpoMysqlRes = mysql_store_result(this->mpoMysqlConn);

        luFieldCount = mysql_field_count(this->mpoMysqlConn);

        //error happens or there is not result or force not fetch
        if (this->mpoMysqlRes == APL_NULL)
        {
            if (luFieldCount != 0)
                liRet = -1;
            break;
        }

        //get how many rows of result set
        this->muRowCounts = mysql_num_rows(this->mpoMysqlRes);

        loFields = mysql_fetch_fields(this->mpoMysqlRes);

        if (loFields == APL_NULL) { liRet = -1; break;}

        for ( luIter = 0; luIter < luFieldCount; luIter ++ ) 
        { 
            this->AddFieldName(loFields[luIter].name);
        }   

        this->miLastExecute = LAST_EXECUTE_COMMON;
    }
    while (0);

    if (liRet == -1)
    {
        this->SetErrorMsg(mysql_error(this->mpoMysqlConn), 
                mysql_errno(this->mpoMysqlConn));
    } 

    return liRet;
}

apl_int_t CMySqlCursor::StoreResult(apl_size_t auFetchRows)
{
    MYSQL_ROW loRow;
    apl_size_t luIter;
    apl_size_t luFieldCount;
    apl_uint_t* lpuLen;
    bool lbFetchAll = false;

    if (this->mpoMysqlRes == APL_NULL)
    {
        return -1;
    }

    if (auFetchRows == 0) 
    {
        lbFetchAll = true;
    }

    luFieldCount = mysql_num_fields(this->mpoMysqlRes);

    if (luFieldCount < 1) { return -1; }

    apl_int_t liRowCount = 0;

    while (
            (( loRow = mysql_fetch_row(this->mpoMysqlRes)) != APL_NULL))
    { 
        char* lpcResult = APL_NULL;

        //get the length of each fileds
        lpuLen = (apl_uint_t*)mysql_fetch_lengths(this->mpoMysqlRes);

        for ( luIter = 0; luIter < luFieldCount; luIter ++ ) 
        { 
            if ( loRow[luIter] == APL_NULL ) 
            { 
                this->AddResult((void*)APL_NULL, 0, liRowCount);
            } 
            else 
            {
                //cursor would take care of this mem, and '\0' would be added
                lpcResult = (char*)apl_malloc(lpuLen[luIter]+1);

                if (lpcResult == APL_NULL)
                {
                    //FIXME!! don't break even we can't malloc 
                    this->AddResult((void*)APL_NULL, 0, liRowCount);
                }
                else 
                {
                    apl_memset(lpcResult, 0, lpuLen[luIter]+1);
                    apl_memcpy(lpcResult, loRow[luIter], lpuLen[luIter]);
                    this->AddResult(lpcResult, lpuLen[luIter], liRowCount);
                }
            }   
        }   

        liRowCount++;

        if (lbFetchAll) { continue; }

        if (--auFetchRows < 1)
        {
            break;
        }
    }   

    //if last rows had fetched
    if (loRow == APL_NULL)
    {
        this->ClearMysqlResultStruct();
    }

    return liRowCount;
}

apl_int_t CMySqlCursor::StoreResultStmt(apl_size_t auFetchRows)
{
    bool lbFetchAll = false;
    apl_size_t luRowCount = 0;
    apl_size_t luFieldCount = 0;
    apl_size_t luIter;
    apl_size_t luLenOfField;
    char* lpcResult = APL_NULL;
    apl_int_t liRet = 0;
    MYSQL_STMT* lpoStmt = APL_NULL;

    if (this->mpoMysqlRes == APL_NULL)
    {
        return -2;
    }

    if (auFetchRows == 0) 
    {
        lbFetchAll = true;
    }

    lpoStmt = this->GetMysqlStmt();

    if (lpoStmt == APL_NULL) 
    {
        return -2;
    }

    luFieldCount = this->moCurResultSet.GetFieldNameCount();
    luRowCount = 0;
    //fetch next row
    while (!(liRet = mysql_stmt_fetch(lpoStmt)))
    {
        //go thru all fileds
        for (luIter = 0; luIter < luFieldCount; ++luIter)
        {
            //malloc and add result
            lpcResult = (char*)apl_malloc(mpuBindLenOutput[luIter] +1);

            if (lpcResult == APL_NULL) 
            {
                liRet = -2;
                break;
            }

            apl_memset(lpcResult, 0, mpuBindLenOutput[luIter] +1);
            apl_memcpy(lpcResult, moBindContentOutputVt[luIter], 
                    mpuBindLenOutput[luIter]);
            this->AddResult(lpcResult, mpuBindLenOutput[luIter], luRowCount);

            luLenOfField = this->moCurResultSet.GetFieldNameWidth(luIter);
            //clear bind result and ready for next fetch
            apl_memset(moBindContentOutputVt[luIter], 0, luLenOfField +1);
        }

        if (liRet != 0) { break; }

        luRowCount++;

        if (lbFetchAll) { continue; }

        if (--auFetchRows < 1) { break; }
    }

    switch (liRet)
    {
        case 0:
        case MYSQL_NO_DATA:
            liRet = luRowCount;
            break;
        case 1:
            //mysql_stmt_fetch error
            this->SetErrorMsg(mysql_error(this->mpoMysqlConn), 
                    mysql_errno(this->mpoMysqlConn));
            liRet = -1;
            break;
        case MYSQL_DATA_TRUNCATED:
            //FIXME!! should never happen
            this->SetErrorMsg(
                "MYSQL_DATA_TRUNCATED error, please contact the developer", 0);
            liRet = -2;
            break;
        default:
            break;
    }

    return liRet;
}


const char* CMySqlCursor::GetField(
        apl_size_t auField,
        apl_int_t* apiLen)
{
    return this->moCurResultSet.GetField(auField, apiLen);
}

const char* CMySqlCursor::GetField(
        const char* apcFieldName, 
        apl_int_t* apiLen)
{
    return this->moCurResultSet.GetField(apcFieldName, apiLen);
}

apl_int_t CMySqlCursor::GetRowCounts()
{
    return this->muRowCounts;
}

apl_size_t CMySqlCursor::GetFieldNameCount()
{
    return this->moCurResultSet.GetFieldNameCount();
}

const char* CMySqlCursor::GetFieldName(apl_size_t auIdx)
{
    return this->moCurResultSet.GetFieldName(auIdx);
}

apl_int_t CMySqlCursor::GetFieldIndex(const char* apcName)
{
    return this->moCurResultSet.GetFieldIndex(apcName);
}

const char* CMySqlCursor::GetErrorMsg()
{
    const char* lpcMsg = this->moErrorMsg.c_str();

    return lpcMsg;
}

apl_int_t CMySqlCursor::GetErrorCode()
{
    return this->miErrorCode;
}

//get how many rows are affected in the last execute
apl_int_t CMySqlCursor::GetAffectedRows()
{
    return this->muAffectedRows;
}

apl_int_t CMySqlCursor::MoveToRow(apl_size_t luIndex)
{
    return this->moCurResultSet.MoveToRow(luIndex);
}

apl_int_t CMySqlCursor::MoveToNextRow()
{
    return this->moCurResultSet.MoveToNextRow();
}

apl_int_t CMySqlCursor::MoveToPrevRow()
{
    return this->moCurResultSet.MoveToPrevRow();
}

void CMySqlCursor::SetPreFetchRows(apl_size_t auRows)
{
    this->muPreFetchRows = auRows;
}

apl_int_t CMySqlCursor::FetchNext()
{
    //move to next row OK
    if (this->moCurResultSet.MoveToNextRow() == 0)
    {
        return 0;
    }

    if (this->FetchMany(this->muPreFetchRows) <= 0)
    {
        return -1;
    }

    if (this->moCurResultSet.MoveToNextRow())
    {
        return -1;
    }

    return 0;
}

//for internal use
///////////////////////////////////////////////////////////////////////////////

apl_int_t CMySqlCursor::FetchMany(apl_size_t auRows)
{
    apl_int_t liRet = 0;

    //clear result buffer
    this->moCurResultSet.ClearResults();

    switch (this->miLastExecute)
    {
        case LAST_EXECUTE_COMMON:
            //store result
            liRet = this->StoreResult(auRows);
            break;
        case LAST_EXECUTE_STMT:
            liRet = StoreResultStmt(auRows);
            break;
        default:
            liRet = -1;
            break;
    }

    //all rows fetched
    if ((liRet >= 0 && (apl_size_t)liRet != auRows)
            || liRet < 0)
    {
        this->miLastExecute = LAST_EXECUTE_NOTHING;
        this->ClearMysqlResultStruct();
    }

    return liRet;
}

apl_int_t CMySqlCursor::Prepare(const char* apcSQLStmt)
{
    const char* lpcParsedSql = APL_NULL;
    apl_size_t luCount;
    apl_int_t liRet = 0;
    MYSQL_STMT* lpoStmt = APL_NULL;
    apl_size_t luParamCount = 0;
    
    if (apcSQLStmt == APL_NULL) 
    {
        return -1;
    }

    this->ClearCursor();
    lpoStmt = this->GetMysqlStmt();

    if (lpoStmt == APL_NULL)
    {
        lpoStmt = mysql_stmt_init(this->mpoMysqlConn);

        if (lpoStmt == APL_NULL) 
        {
            return -1;
        }

        this->SetMysqlStmt(lpoStmt);
    }

    if (this->mpoParamBind->ParseSql(apcSQLStmt))
    {
        return -2;
    }

    //set stmt for binding
    this->mpoParamBind->SetBindingNeed(lpoStmt);

    lpcParsedSql = this->mpoParamBind->GetParasedSql();

    if (lpcParsedSql == APL_NULL) 
    {
        return -2;
    }

    //bind by pos
    luCount = this->mpoParamBind->GetParamCount();

    if (luCount < 1) 
    {
        return -2;
    }

    do {

        if (mysql_stmt_prepare(lpoStmt, 
                    lpcParsedSql, apl_strlen(lpcParsedSql)))
        {
            liRet = -1;
            break;
        }

        luParamCount= mysql_stmt_param_count(lpoStmt);

        if (luCount != luParamCount)
        {
            liRet = -1;
            break;
        }

    } while (0);


    if (liRet != 0)
    {
            this->SetErrorMsg(mysql_error(this->mpoMysqlConn), 
                    mysql_errno(this->mpoMysqlConn));
    }

    return liRet;
}

apl_int_t CMySqlCursor::BindParam(const char* apcParamName, apl_int16_t* apiParam)
{
    apl_int_t liRet;

    liRet = this->mpoParamBind->BindParam(apcParamName, apiParam);

    if (liRet == -1)
    {
        this->SetErrorMsg(mysql_stmt_error(this->mpoStmt), 
                mysql_stmt_errno(this->mpoStmt));
    }

    return liRet;
}

apl_int_t CMySqlCursor::BindParam(const char* apcParamName, apl_int32_t* apiParam)
{
    apl_int_t liRet;

    liRet = this->mpoParamBind->BindParam(apcParamName, apiParam);

    if (liRet == -1)
    {
        this->SetErrorMsg(mysql_stmt_error(this->mpoStmt), 
                mysql_stmt_errno(this->mpoStmt));
    }

    return liRet;
}

apl_int_t CMySqlCursor::BindParam(const char* apcParamName, apl_int64_t* apiParam)
{
    apl_int_t liRet;

    liRet = this->mpoParamBind->BindParam(apcParamName, apiParam);

    if (liRet == -1)
    {
        this->SetErrorMsg(mysql_stmt_error(this->mpoStmt), 
                mysql_stmt_errno(this->mpoStmt));
    }

    return liRet;
}

apl_int_t CMySqlCursor::BindParam(const char* apcParamName, apl_uint32_t* apuParam)
{
    apl_int_t liRet;

    liRet = this->mpoParamBind->BindParam(apcParamName, apuParam);

    if (liRet == -1)
    {
        this->SetErrorMsg(mysql_stmt_error(this->mpoStmt), 
                mysql_stmt_errno(this->mpoStmt));
    }

    return liRet;
}

apl_int_t CMySqlCursor::BindParam(
        const char* apcParamName, 
        const char* apcParam, 
        apl_size_t auLen)
{
    apl_int_t liRet;

    liRet = this->mpoParamBind->BindParam(apcParamName, apcParam, auLen);

    if (liRet == -1)
    {
        this->SetErrorMsg(mysql_stmt_error(this->mpoStmt), 
                mysql_stmt_errno(this->mpoStmt));
    }

    return liRet;
}

apl_int_t CMySqlCursor::BindParam(
        const char* apcParamName, 
        const void* apcParam, 
        apl_size_t auLen,
        bool abIsBinary)
{
    apl_int_t liRet;

    liRet = this->mpoParamBind->BindParam(apcParamName, apcParam, auLen, abIsBinary);

    if (liRet == -1)
    {
        this->SetErrorMsg(mysql_stmt_error(this->mpoStmt), 
                mysql_stmt_errno(this->mpoStmt));
    }

    return liRet;
}

apl_int_t CMySqlCursor::BindParam(const char* apcParamName, float* apfParam)
{
    apl_int_t liRet;

    liRet = this->mpoParamBind->BindParam(apcParamName, apfParam);

    if (liRet == -1)
    {
        this->SetErrorMsg(mysql_stmt_error(this->mpoStmt), 
                mysql_stmt_errno(this->mpoStmt));
    }

    return liRet;
}

apl_int_t CMySqlCursor::BindParam(const char* apcParamName, double* apdParam)
{
    apl_int_t liRet;

    liRet = this->mpoParamBind->BindParam(apcParamName, apdParam);

    if (liRet == -1)
    {
        this->SetErrorMsg(mysql_stmt_error(this->mpoStmt), 
                mysql_stmt_errno(this->mpoStmt));
    }

    return liRet;
}

apl_int_t CMySqlCursor::AddFieldName(const char* apcField)
{
    return this->moCurResultSet.AddFieldName(apcField);
}

void CMySqlCursor::ClearFieldNames()
{
    return this->moCurResultSet.ClearFieldNames();
}

void CMySqlCursor::AddResult(
        void* apvPtr, 
        apl_size_t auLen, 
        apl_size_t auRow)
{
    return this->moCurResultSet.AddResult(apvPtr, auLen, auRow);
}

void CMySqlCursor::ClearMysqlResultStruct()
{
    if (this->mpoMysqlRes != APL_NULL)
    {
        mysql_free_result(this->mpoMysqlRes);
        this->mpoMysqlRes = APL_NULL;
    }

    //for stmt only 
    if (this->mpoBindOutput != APL_NULL)
    {
        delete [] this->mpoBindOutput;
        this->mpoBindOutput = APL_NULL;
    }

    //for stmt only 
    if (this->mpuBindLenOutput != APL_NULL)
    {
        delete [] this->mpuBindLenOutput;
        this->mpuBindLenOutput = APL_NULL;
    }

    //for stmt only
    if (!this->moBindContentOutputVt.empty())
    {
        apl_size_t luIter;

        for (luIter = 0; luIter < moBindContentOutputVt.size(); ++luIter)
        {
            apl_free(moBindContentOutputVt[luIter]);
        }

        this->moBindContentOutputVt.clear();
    }
}

void CMySqlCursor::ClearResults()
{
    this->ClearMysqlResultStruct();
    return this->moCurResultSet.ClearResults();
}

void CMySqlCursor::SetErrorMsg(const char* apcMsg, apl_int_t aiErrCode)
{
    this->moErrorMsg = apcMsg;
    this->miErrorCode = aiErrCode;
}

void CMySqlCursor::ClearErrorMsg()
{
    this->moErrorMsg.clear();
    this->miErrorCode = 0;
}

void CMySqlCursor::SetAffectedRows(apl_size_t auRows)
{
    this->muAffectedRows = auRows;
}

void CMySqlCursor::SetRowCounts(apl_size_t auRows)
{
    this->muRowCounts = auRows;
}

void CMySqlCursor::ClearCursor()
{
    this->SetAffectedRows(0);
    this->ClearErrorMsg();
    this->ClearFieldNames();
    this->ClearResults();
    this->SetCurrentRow(0);
    this->SetRowCounts(0);
}

apl_size_t CMySqlCursor::GetCurrentRow()
{
    return this->moCurResultSet.GetCurrentRow();
}

void CMySqlCursor::SetCurrentRow(apl_size_t auRow)
{
    return this->moCurResultSet.SetCurrentRow(auRow);
}

void CMySqlCursor::SetMysqlVConn(MYSQL* apoMysqlConn)
{
    this->mpoMysqlConn = apoMysqlConn;
}

MYSQL_STMT* CMySqlCursor::GetMysqlStmt()
{
    return this->mpoStmt;
}

void CMySqlCursor::SetMysqlStmt(MYSQL_STMT* apoMysqlStmt)
{
    this->mpoStmt = apoMysqlStmt;
}

CParamBind* CMySqlCursor::GetParamBind()
{
    return this->mpoParamBind;
}

apl_int_t CMySqlCursor::SetCharacters(const char* apcCharacterSet)
{
    std::string loStr = "SET NAMES '";

    loStr.append(apcCharacterSet);
    loStr.append("'");

    return Execute(loStr.c_str());
}

ADL_NAMESPACE_END
