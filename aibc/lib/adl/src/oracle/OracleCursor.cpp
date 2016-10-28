///////////////////////////////////////////////////////////
//  COracleCursor.cpp
//  Implementation of the Class COracleCursor
//  Created on:      19-10-2009 14:28:01
//  Original author: hezk
///////////////////////////////////////////////////////////

#include <algorithm>
#include "apl/str.h"
#include "adl/Adl.h"
#include "OracleImpl.h"
#include "acl/StrAlgo.h"


ADL_NAMESPACE_START

///////////////////////////////////////////////////////////////////////////////

COracleCursor::COracleCursor()
    :mpoSvchp(APL_NULL)
    ,mpoStmthp(APL_NULL)
    ,mpoEnvhp(APL_NULL)
    ,mpoErrhp(APL_NULL)
    ,mpoOracleConnImpl(APL_NULL)
    ,miErrorCode(0)
    ,muAffectedRows(0)
    ,muRowCounts(0)
    ,mpoParamBind(APL_NULL)
    ,miLastExecute(0)
    ,muFetchCursorIdx(OCI_FETCH_FIRST)
    ,muPreFetchRows(1)
{
    ACL_NEW_ASSERT(this->mpoParamBind, COracleParamBind)
}

COracleCursor::~COracleCursor()
{
    ACL_DELETE(this->mpoParamBind)
    this->ClearResults();
}

void COracleCursor::Close()
{
    if (this->mpoErrhp) 
    {
        CheckErr(OCIHandleFree((dvoid *)this->mpoErrhp, 
                    (ub4) OCI_HTYPE_ERROR),
                "Failed to free errhp in cursor close");
    }

    if (this->mpoStmthp) 
    {
        CheckErr(OCIHandleFree((dvoid *)this->mpoStmthp, 
                    (ub4) OCI_HTYPE_STMT),
                "Failed to free stmthp in cursor close");
    }

    delete this;
}

apl_int_t COracleCursor::Execute()
{
    return this->ExecuteAfterPrepare();
}

apl_int_t COracleCursor::Execute(const char* apcSQL)
{
    if (!apcSQL) { return -1; }

    text *lpoStmt = (text*)apcSQL;

    /* prepare create statement */
    if (CheckErr(OCIStmtPrepare(
                    this->mpoStmthp, this->mpoErrhp, 
                    lpoStmt, (ub4) apl_strlen((char*)lpoStmt), 
                    (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT),
            "Failed to prepare in cursor:execute"))
    {
        return -1;
    }

    return this->ExecuteAfterPrepare();
}

const char* COracleCursor::GetField(apl_size_t auField, apl_int_t* apiLen)
{
    return this->moCurResultSet.GetField(auField, apiLen);
}

const char* COracleCursor::GetField(
        const char* apcFieldName, 
        apl_int_t* apiLen)
{
    return this->moCurResultSet.GetField(apcFieldName, apiLen);
}

apl_int_t COracleCursor::GetFieldIndex(const char* apcFieldName)
{
    return this->moCurResultSet.GetFieldIndex(apcFieldName);
}

apl_size_t COracleCursor::GetFieldNameCount()
{
    return this->moCurResultSet.GetFieldNameCount();
}

const char* COracleCursor::GetFieldName(apl_size_t auIdx)
{
    return this->moCurResultSet.GetFieldName(auIdx);
}

apl_int_t COracleCursor::GetRowCounts()
{
    return this->muRowCounts;
}

const char* COracleCursor::GetErrorMsg()
{
    return this->moErrMsg.c_str();
}

apl_int_t COracleCursor::GetErrorCode()
{
    return this->miErrorCode;
}

apl_int_t COracleCursor::GetAffectedRows()
{
    return this->muAffectedRows;
}

apl_int_t COracleCursor::MoveToNextRow()
{
    return this->moCurResultSet.MoveToNextRow();
}

void COracleCursor::SetPreFetchRows(apl_size_t auRows)
{
    this->muPreFetchRows = auRows;
}

apl_int_t COracleCursor::FetchNext()
{
    //move to next row OK
    if (this->moCurResultSet.MoveToNextRow() == 0)
    {
        return 0;
    }

    if (this->FetchMany(1/*this->muPreFetchRows disable*/) <= 0)
    {
        return -1;
    }

    if (this->moCurResultSet.MoveToNextRow())
    {
        return -1;
    }

    return 0;
}

apl_int_t COracleCursor::FetchMany(apl_size_t auRows)
{
    apl_int_t liRet;

    this->moCurResultSet.ClearResults(1);

    if (auRows < 1) { return 0; }

    if (this->miLastExecute != LAST_EXECUTE_NOT_FETCH)
    {
        return -1;
    }

    liRet = this->StoreResult(auRows, muFetchCursorIdx);

    if ((liRet >= 0 && (apl_size_t)liRet != auRows)
        || liRet < 0)
    {
        this->miLastExecute = LAST_EXECUTE_NOTHING;
        //clear the start cursor index
        this->muFetchCursorIdx = OCI_FETCH_FIRST;
    }

    //changing the cursor
    if (liRet > 0)
    {
        this->muFetchCursorIdx = OCI_FETCH_NEXT;
    }

    return liRet;
}

//for internal use
apl_int_t COracleCursor::ExecuteAfterPrepare()
{
    //adjust the iters 
    //For non-SELECT statements, iters = 1, rowoff = 0
    //For SELECT statements, iters = 0, rowoff = 0
    ub4 luIters = 1;
    ub2 loStmtType = 0;
    ub4 loExecMode = OCI_DEFAULT;

    //clear 
    this->miLastExecute = LAST_EXECUTE_NOTHING;
    //clear the start cursor index
    this->muFetchCursorIdx = OCI_FETCH_FIRST;

    if (CheckErr(OCIAttrGet((dvoid *) this->mpoStmthp, (ub4) OCI_HTYPE_STMT, 
                    (void *) &loStmtType, (ub4 *) NULL, 
                    (ub4) OCI_ATTR_STMT_TYPE, this->mpoErrhp), 
                "Failed to get type of command in cursor:execute"))
    {
        return -1;
    }

    //SELECT statement
    if (loStmtType == OCI_STMT_SELECT)
    {
        luIters = 0;
        loExecMode = OCI_STMT_SCROLLABLE_READONLY;
    }

    //execute create statement, SCROLLABLE mode 
    if (CheckErr(OCIStmtExecute(
                    this->mpoSvchp, this->mpoStmthp, 
                    this->mpoErrhp, 
                    luIters, //iters
                    (ub4) 0, //rowoff
                    (CONST OCISnapshot *) 0, (OCISnapshot *) 0,
                    (ub4) loExecMode), 
                "Failed to execute in cursor:execute"))
    {
        return -1;
    }

    //commit or not
    if (!this->mpoOracleConnImpl->IsTransMode())
    {
        if (CheckErr(OCITransCommit(this->mpoSvchp, 
                        this->mpoErrhp, OCI_DEFAULT), 
                    "Failed to commit in cursor:execute"))
            return -1;
    }

    //do not need at this point
#if 0
    //SQL Command Code, which SQL command we had just executed
    ub2 loSqlCmdCode = 0;
    if (CheckErr(OCIAttrGet((dvoid *) this->mpoStmthp, (ub4) OCI_HTYPE_STMT, 
                    (void *) &loSqlCmdCode, (ub4 *) NULL, 
                    (ub4) OCI_ATTR_SQLFNCODE, this->mpoErrhp), 
                "Failed to get type of command in cursor:execute"))
    {
        return -1;
    }
#endif
    
    //clear and reset
    this->muRowCounts = 0;
    this->ClearResults();

    //In case of UPDATE/INSERT/DELETE... we should know how many rows are 
    //affected get how many rows are affected
    if (loStmtType != OCI_STMT_SELECT)
    {
        if (CheckErr(OCIAttrGet((dvoid *) this->mpoStmthp, 
                        (ub4) OCI_HTYPE_STMT, 
                        (void *) &this->muAffectedRows, (ub4 *) NULL, 
                        (ub4) OCI_ATTR_ROW_COUNT, this->mpoErrhp), 
                    "Failed to get affected rows cursor:execute"))
        {
            return -1;
        }
    }
    //SELECT command, get the result set
    else 
    {
        if (this->GetFieldsAttr())
        {
            return -1;
        }

        //fetch the result if any 
        if (this->FetchResultSet())
        {
            return -1;
        }
    }

    return 0;
}

apl_int_t COracleCursor::GetFieldsAttr()
{
    //let's check we can get anything back
    OCIParam     *loMypard = (OCIParam *) 0;
    ub2          luDtype;
    text         *lpoColName = APL_NULL;
    ub4          luCounter, luColNameLen, luCharSemantics;
    ub2          luColWidth;
    sb4          liParmStatus;
    apl_int_t    liRet = 0;

    //clear last fileds
    this->moCurResultSet.ClearFieldNames();
    luCounter = 1;
    //try to get field
    liParmStatus = OCIParamGet((dvoid *)this->mpoStmthp, OCI_HTYPE_STMT, 
            this->mpoErrhp, (dvoid **)&loMypard, (ub4) luCounter);

    if (liParmStatus != OCI_SUCCESS) 
    {
        return 0;
    }
    /* Loop only if a descriptor was successfully retrieved for
       current position, starting at 1 */
    while (liParmStatus == OCI_SUCCESS) {
        /* Retrieve the datatype attribute */
        if (CheckErr(OCIAttrGet((dvoid*) loMypard, (ub4) OCI_DTYPE_PARAM,
                        (dvoid*) &luDtype, (ub4 *) 0, 
                        (ub4) OCI_ATTR_DATA_TYPE,
                        (OCIError *) this->mpoErrhp), 
                    "Failed to get data type"))
        {
            liRet = -1;
            break;
        }

        /* Retrieve the column name attribute */
        luColNameLen = 0;
        if (CheckErr(OCIAttrGet((dvoid*) loMypard, (ub4) OCI_DTYPE_PARAM,
                        (dvoid**) &lpoColName, (ub4 *) &luColNameLen, 
                        (ub4) OCI_ATTR_NAME,
                        (OCIError *) this->mpoErrhp ), 
                    "Failed t o get col name"))
        {
            liRet = -1;
            break;
        }

        /* Retrieve the length semantics for the column */
        luCharSemantics = 0;
        luColWidth = 0;

        if (CheckErr(OCIAttrGet((dvoid*) loMypard, (ub4) OCI_DTYPE_PARAM,
                        (dvoid*) &luCharSemantics,(ub4 *) 0, 
                        (ub4) OCI_ATTR_CHAR_USED,
                        (OCIError *)this->mpoErrhp), 
                    "Failed to get char semantics"))
        {
            liRet = -1;
            break;
        }

        if (luCharSemantics)
        {
            /* Retrieve the column width in characters */
            if (CheckErr(OCIAttrGet((dvoid*) loMypard, (ub4) OCI_DTYPE_PARAM,
                            (dvoid*) &luColWidth, (ub4 *) 0, 
                            (ub4) OCI_ATTR_CHAR_SIZE, 
                            (OCIError *)this->mpoErrhp), 
                        "Failed to get col width"))
            {
                liRet = -1;
                break;
            }
        }
        else
        {
            /* Retrieve the column width in bytes */
            if (CheckErr(OCIAttrGet((dvoid*) loMypard, (ub4) OCI_DTYPE_PARAM,
                            (dvoid*) &luColWidth,(ub4 *) 0, 
                            (ub4) OCI_ATTR_DATA_SIZE,
                            (OCIError *) this->mpoErrhp), 
                        "Failed to get col width"))
            {
                liRet = -1;
                break;
            }
        }

        //FIXME!! more data type will be need to enlarge the size
        switch (luDtype)
        {
        case SQLT_FLT:
#ifdef SQLT_BFLOAT 
        case SQLT_BFLOAT:
#endif
#ifdef SQLT_BDOUBLE
        case SQLT_BDOUBLE:
#endif
#ifdef SQLT_IBFLOAT 
        case SQLT_IBFLOAT:
#endif
#ifdef SQLT_IBDOUBLE
        case SQLT_IBDOUBLE:
#endif
            //32 is enough for every canonical binary double
            //e.g. 8 bytes in double but it occupy 24 bytes in string 
            //format 1.1000000000000001E+000
            luColWidth = 32;
            break;
        case SQLT_ODT:
        case SQLT_DAT:
        case SQLT_DATE:
        case SQLT_TIME:
        case SQLT_TIME_TZ:
        case SQLT_TIMESTAMP:
        case SQLT_TIMESTAMP_TZ:
        case SQLT_TIMESTAMP_LTZ:
            // YYYY-MM-DD HH:MM:SS ZZZ
            luColWidth = 32; 
            break;
        case SQLT_INT:
        case SQLT_UIN:
            luColWidth *= 3;
        default:
            break;
        }

        //add filed to result set
        //in case of not \0 terminated colname
        std::string loStrName((const char*)lpoColName, luColNameLen); 
        this->moCurResultSet.AddFieldName(loStrName.c_str(), luColWidth, luDtype);

        /* increment luCounter and get next descriptor, if there is one */
        luCounter++;
        liParmStatus = OCIParamGet((dvoid *)this->mpoStmthp, OCI_HTYPE_STMT,
                this->mpoErrhp, (dvoid **)&loMypard, (ub4) luCounter);
    } /* while */

    return liRet;
}

void COracleCursor::GetOCIErrMsg()
{
    text lacErrbuf[512];
    sb4 loErrcode = 0;
    (void) OCIErrorGet((dvoid *)this->mpoErrhp, (ub4) 1, (text *) NULL, 
            &loErrcode, lacErrbuf, (ub4) sizeof(lacErrbuf), OCI_HTYPE_ERROR);
    this->moErrMsg = (char*)lacErrbuf;
    this->miErrorCode = loErrcode;
}

void COracleCursor::ReserveResultCount(apl_size_t auCount)
{
    this->moCurResultSet.ReserveResultCount(auCount);
}

void COracleCursor::AddResult(
        void* apvPtr, 
        apl_size_t auLen, 
        apl_size_t auRow)
{
    this->moCurResultSet.AddResult(apvPtr, auLen, auRow);
}

void COracleCursor::ClearResults()
{
    this->moCurResultSet.ClearResults(1);
}

struct _FieldVariable 
{
    inline void Init(apl_size_t auWidth, apl_size_t auNum)
    {
        muWidth = auWidth;
        moBuffer.resize(auWidth*auNum);
        moIndicators.resize(auNum);
        for (apl_size_t i = 0; i < auNum; ++i)
        {
            moIndicators[i] = OCI_IND_NULL;
        }
    }

    inline char* BufPtr(apl_size_t auNum)
    {
        return &moBuffer[muWidth*auNum];
    }

    inline sb2* IndPtr(apl_size_t auNum)
    {
        return &moIndicators[auNum];
    }

    apl_size_t          muWidth;
    std::vector<sb2>    moIndicators;
    std::vector<char>   moBuffer;
};

struct IDefineBuffer
{
    virtual ~IDefineBuffer(void) {};

    virtual void* GetPtr(void) = 0;
    
    virtual sb2 GetIndp(void) = 0;
};

struct CRawBuffer : public IDefineBuffer
{
    CRawBuffer( apl_size_t auSize ) : muLen(auSize), miIndp(-1)
    {
        ACL_MALLOC_INIT(this->mpvBuffer, char, auSize, 0)
    }

    ~CRawBuffer(void)
    {
        ACL_FREE(this->mpvBuffer);
    }
    
    void* GetPtr(void)
    {
        return this->mpvBuffer;
    }

    sb2 GetIndp(void)
    {
        return this->miIndp;
    }

    void* mpvBuffer;
    apl_size_t muLen;
    sb2 miIndp;
};

struct CLocatorBuffer : public IDefineBuffer
{
    CLocatorBuffer(void) : mpoLocator(APL_NULL), muLen(0), miIndp(-1)
    {
    }

    ~CLocatorBuffer(void)
    {
        if (this->mpoLocator != APL_NULL)
        {
            OCIDescriptorFree(this->mpoLocator, OCI_DTYPE_LOB);
        }
    }

    void* GetPtr(void)
    {
        return this->mpoLocator;
    }

    sb2 GetIndp(void)
    {
        return this->miIndp;
    }

    OCILobLocator* mpoLocator;

    ub2 muLen;
    
    sb2 miIndp;
};

apl_int_t COracleCursor::OutputParam(std::vector<IDefineBuffer*>& aoOutput )
{
    apl_size_t luFieldCounts = this->moCurResultSet.GetFieldNameCount();
    OCIDefine *lpoDefnp = (OCIDefine *) 0;
    ub4 luSize = 0;
    ub2 luDtype;
    //sb2 li2Indp = OCI_IND_NULL;
   
    if (luFieldCounts == 0) 
    {
        return 0; 
    }

    //go thru all fields, malloc space and define by pos
    for (ub4 luIdx = 0; luIdx < luFieldCounts; ++luIdx) 
    {
        //define by pos
        luSize = this->moCurResultSet.GetFieldNameWidth(luIdx);
        luDtype = this->moCurResultSet.GetFieldNameType(luIdx);
        
        switch(luDtype)
        {
            #if 0
            default:
            {
                CRawBuffer* lpoBuffer = APL_NULL;
                ACL_NEW_ASSERT(lpoBuffer, CRawBuffer(luSize + 1) );//must append 1 end character for SQLT_STR
                
                //remember what we alloc
                aoOutput.push_back(lpoBuffer);

                //pos start from 1 (luIdx+1)
                if (CheckErr(OCIDefineByPos(this->mpoStmthp, &lpoDefnp, 
                                this->mpoErrhp, luIdx+1, (dvoid *) lpoBuffer->mpvBuffer, 
                                lpoBuffer->muLen, SQLT_STR,
                                (dvoid*)&lpoBuffer->miIndp,//NULL,//(dvoid *) &li2Indp
                                (ub2 *) 0, 
                                (ub2 *) 0, OCI_DEFAULT), 
                            "Failed to define by pos when fetchall"))
                {
                    return -1;
                }

                break;
            }
            #endif
            case SQLT_BIN:
            {
                CRawBuffer* lpoBuffer = APL_NULL;
                ACL_NEW_ASSERT(lpoBuffer, CRawBuffer(luSize*2) );
                
                //remember what we alloc
                aoOutput.push_back(lpoBuffer);
                
                //pos start from 1 (luIdx+1)
                if (CheckErr(OCIDefineByPos(this->mpoStmthp, &lpoDefnp, 
                                this->mpoErrhp, luIdx+1, (dvoid *) lpoBuffer->mpvBuffer, 
                                lpoBuffer->muLen, SQLT_LVB, 
                                (dvoid*)&lpoBuffer->miIndp,//NULL,//(dvoid *) &li2Indp
                                (ub2 *) 0, 
                                (ub2 *) 0, OCI_DEFAULT), 
                            "Failed to define by pos when fetchall"))
                {
                    return -1;
                }
                
                break;
            }
            case SQLT_BLOB:
            {
                CLocatorBuffer* lpoLocator = APL_NULL;
                ACL_NEW_ASSERT(lpoLocator, CLocatorBuffer);

                if (CheckErr(OCIDescriptorAlloc(this->mpoEnvhp,
                                (dvoid**)&lpoLocator->mpoLocator,
                                OCI_DTYPE_LOB, 0, NULL),
                            "Failed to alloc descriptor when fetchall"))
                {
                    return -1;
                }

                aoOutput.push_back(lpoLocator);
                
                //pos start from 1 (luIdx+1)
                if (CheckErr(OCIDefineByPos(this->mpoStmthp, &lpoDefnp, 
                                this->mpoErrhp, luIdx+1, (dvoid *)&lpoLocator->mpoLocator, 
                                -1, luDtype,
                                (dvoid*)&lpoLocator->miIndp,//NULL,//(dvoid *) &li2Indp
                                &lpoLocator->muLen, 
                                (ub2 *) 0, OCI_DEFAULT), 
                            "Failed to define by pos when fetchall"))
                {
                    return -1;
                }

                break;
            }
            default:
            {
                CRawBuffer* lpoBuffer = APL_NULL;
                ACL_NEW_ASSERT(lpoBuffer, CRawBuffer(luSize + 1) );//must append 1 end character for SQLT_STR
                
                //remember what we alloc
                aoOutput.push_back(lpoBuffer);

                //pos start from 1 (luIdx+1)
                if (CheckErr(OCIDefineByPos(this->mpoStmthp, &lpoDefnp, 
                                this->mpoErrhp, luIdx+1, (dvoid *) lpoBuffer->mpvBuffer, 
                                lpoBuffer->muLen, SQLT_STR,
                                (dvoid*)&lpoBuffer->miIndp,//NULL,//(dvoid *) &li2Indp
                                (ub2 *) 0, 
                                (ub2 *) 0, OCI_DEFAULT), 
                            "Failed to define by pos when fetchall"))
                {
                    return -1;
                }

                break;
            }
        }
    }

    return 0;
}

//FIXME!! Note: OCI_ATTR_ROW_COUNT is the whole row counts of the 
//result set!!! (only when the first fetch toggle, this attr is 
//available.) So we use the OCI_FETCH_LAST and get current 
//position instead.
apl_int_t COracleCursor::FetchResultsetRows()
{
    apl_int_t liRet = 0;
    ub4 luIdx = 0;
    apl_size_t luFieldCounts = this->moCurResultSet.GetFieldNameCount();
    std::vector<IDefineBuffer*> loDefinePtr; //ptr for first define (define by pos)
    sword liStatus;

    if (luFieldCounts == 0) 
    {
        return 0; 
    }

    do
    {
        if (this->OutputParam(loDefinePtr) != 0)
        {
            liRet = -1;
            break;
        }

        //go to the last
        liStatus = OCIStmtFetch2(this->mpoStmthp, this->mpoErrhp, 
                (ub4) 1, OCI_FETCH_LAST, (sb4) 0, OCI_DEFAULT);
        if (liStatus != OCI_SUCCESS && liStatus != OCI_SUCCESS_WITH_INFO)
        {
            this->GetOCIErrMsg();
            if (liStatus != OCI_NO_DATA)
            {
                liRet = -1;
            }
            break;
        }

        //get how many rows we have
        if (CheckErr(OCIAttrGet((dvoid *) this->mpoStmthp, 
                        (ub4)OCI_HTYPE_STMT,
                        (void *) &this->muRowCounts, (ub4 *) NULL, 
                        (ub4) OCI_ATTR_CURRENT_POSITION, this->mpoErrhp), 
                    "Failed to get type of command in cursor:execute"))
        {
            liRet = -1;
            break;
        }
        //only do once
        break;
    }
    while(false);

    //free the define mem 
    for (luIdx = 0; luIdx < loDefinePtr.size(); ++luIdx)
    {
        ACL_DELETE(loDefinePtr[luIdx])
    }

    return 0;
}

apl_int_t COracleCursor::StoreResult(
        apl_size_t auFetchRows, 
        ub2 auFetchCursorIdx)
{
    apl_size_t luFieldCounts = this->moCurResultSet.GetFieldNameCount();
    sword liStatus;
    apl_int_t liRet = -1;
    ub4 luFetchRows = 1;
    ub4 luSize = 0;
    ub2 luDtype;
    std::vector<IDefineBuffer*> loDefinePtr; //ptr for first define (define by pos)
   
    if (luFieldCounts == 0)
    {
        return 0; 
    }

    if (this->OutputParam(loDefinePtr) != 0)
    {
        goto FETCH_EXIT;
    }

    //fetch result set from the first
    liStatus = OCIStmtFetch2(this->mpoStmthp, this->mpoErrhp, 
            (ub4)luFetchRows, auFetchCursorIdx, 
            (sb4) 0, OCI_DEFAULT);

    switch(liStatus)
    {
        case OCI_NO_DATA:
            liRet = 0;
            goto FETCH_EXIT;

        case OCI_SUCCESS:
        case OCI_SUCCESS_WITH_INFO:
            break;

        default:
            CheckErr(liStatus, "Failed to fetch result set");
            liRet = -1;
            goto FETCH_EXIT;

    };

    //reserve mem for faster purpose
    this->ReserveResultCount(luFetchRows);

    for (ub4 luIdx = 0; luIdx < luFieldCounts; ++luIdx) 
    {
        char* lpcContent = APL_NULL;
        //define by pos
        luSize = this->moCurResultSet.GetFieldNameWidth(luIdx);
        luDtype = this->moCurResultSet.GetFieldNameType(luIdx);
        
        switch(luDtype)
        {
            #if 0
            default:
                ACL_MALLOC_INIT(lpcContent, char, luSize + 1, 0)
                apl_strncpy(lpcContent, (char*)loDefinePtr[luIdx]->GetPtr(), luSize + 1);

                this->AddResult(lpcContent, apl_strlen(lpcContent), 0);

                break;
            #endif
            case SQLT_BIN:
                //raw is struct { ub4 len; char *buf; }
                luSize = *(ub4*)loDefinePtr[luIdx]->GetPtr();

                ACL_MALLOC_INIT(lpcContent, char, luSize + 1, 0)
                apl_memcpy(lpcContent, ((char*)loDefinePtr[luIdx]->GetPtr() ) + sizeof(ub4), luSize);

                this->AddResult(lpcContent, luSize, 0);

                break;
            case SQLT_BLOB:
            {
                OCILobGetLength(this->mpoSvchp, this->mpoErrhp, (OCILobLocator*)loDefinePtr[luIdx]->GetPtr(), &luSize);
                
                ACL_MALLOC_INIT(lpcContent, char, luSize + 1, 0)
                
                if (luSize > 0 && CheckErr(OCILobRead(this->mpoSvchp, this->mpoErrhp, (OCILobLocator*)loDefinePtr[luIdx]->GetPtr(),
                        &luSize, 1, (dvoid*)lpcContent, luSize,
                        NULL, NULL, 0, 0) ) )
                {
                    ACL_FREE(lpcContent);

                    goto FETCH_EXIT;
                }

                this->AddResult(lpcContent, luSize, 0);

                break;
            }
            default:
            {
                ACL_MALLOC_INIT(lpcContent, char, luSize + 1, 0)
                apl_strncpy(lpcContent, (char*)loDefinePtr[luIdx]->GetPtr(), luSize + 1);

                this->AddResult(lpcContent, apl_strlen(lpcContent), 0);

                break;
            }
        }
    }

    liRet = luFetchRows;

FETCH_EXIT:
    //free the define mem 
    for (apl_size_t luI = 0; luI < loDefinePtr.size(); ++luI)
    {
        ACL_DELETE(loDefinePtr[luI]);
    }

    return liRet;
}

//curentlly, only the c string are supported
apl_int_t COracleCursor::FetchResultSet()
{
    apl_int_t liRet = 0;

    //get how many rows in result set
    //Disable get resultset size for muRowCounts
    liRet = this->FetchResultsetRows();

    if (liRet) { return -1; }

    //remember it
    this->miLastExecute = LAST_EXECUTE_NOT_FETCH;

    return 0;
}

apl_int_t COracleCursor::Prepare(const char* apcSQLStmt)
{
    text* lpoParsedSql = APL_NULL;
    apl_int_t liCount;
    
    if (mpoParamBind->ParseSql(apcSQLStmt))
    {
        return -1;
    }

    lpoParsedSql = (text*)mpoParamBind->GetParasedSql();

    if (lpoParsedSql == APL_NULL) { return -1; }

    this->mpoParamBind->SetBindingNeed(this->mpoStmthp, this->mpoErrhp);

    //prepare with lpoParsedSql
    if (CheckErr(OCIStmtPrepare(
                    this->mpoStmthp, this->mpoErrhp, 
                    lpoParsedSql, (ub4) apl_strlen((char*)lpoParsedSql), 
                    (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT),
            "Failed to prepare in cursor:execute"))
    {
        return -1;
    }
    
    //bind by pos
    liCount = mpoParamBind->GetParamCount();

    if (liCount < 1) 
    {
        return -1;
    }

    return 0;
}

apl_int_t COracleCursor::BindParam(const char* apcParamName, apl_int16_t* apiParam)
{
    apl_int_t liRet;

    liRet = this->mpoParamBind->BindParam(apcParamName, apiParam);

    if (liRet == -1)
    {
        this->GetOCIErrMsg();
    }

    return liRet;
}

apl_int_t COracleCursor::BindParam(const char* apcParamName, apl_int32_t* apiParam)
{
    apl_int_t liRet;

    liRet = this->mpoParamBind->BindParam(apcParamName, apiParam);

    if (liRet == -1)
    {
        this->GetOCIErrMsg();
    }

    return liRet;
}

apl_int_t COracleCursor::BindParam(const char* apcParamName, apl_int64_t* apiParam)
{
    apl_int_t liRet;

    liRet = this->mpoParamBind->BindParam(apcParamName, apiParam);

    if (liRet == -1)
    {
        this->GetOCIErrMsg();
    }

    return liRet;
}

apl_int_t COracleCursor::BindParam(const char* apcParamName, apl_uint32_t* apuParam)
{
    apl_int_t liRet;

    liRet = this->mpoParamBind->BindParam(apcParamName, apuParam);

    if (liRet == -1)
    {
        this->GetOCIErrMsg();
    }

    return liRet;
}

apl_int_t COracleCursor::BindParam(const char* apcParamName, const char* apcParam, apl_size_t auLen)
{
    apl_int_t liRet;

    liRet = this->mpoParamBind->BindParam(apcParamName, apcParam, auLen);

    if (liRet == -1)
    {
        this->GetOCIErrMsg();
    }

    return liRet;
}

apl_int_t COracleCursor::BindParam(const char* apcParamName, const void* apcParam, apl_size_t auLen, bool abIsBinary)
{
    apl_int_t liRet;

    liRet = this->mpoParamBind->BindParam(apcParamName, apcParam, auLen, abIsBinary);

    if (liRet == -1)
    {
        this->GetOCIErrMsg();
    }

    return liRet;
}

apl_int_t COracleCursor::BindParam(const char* apcParamName, float* apfParam)
{
    apl_int_t liRet;

    liRet = this->mpoParamBind->BindParam(apcParamName, apfParam);

    if (liRet == -1)
    {
        this->GetOCIErrMsg();
    }

    return liRet;
}

apl_int_t COracleCursor::BindParam(const char* apcParamName, double* apdParam)
{
    apl_int_t liRet;

    liRet = this->mpoParamBind->BindParam(apcParamName, apdParam);

    if (liRet == -1)
    {
        this->GetOCIErrMsg();
    }

    return liRet;
}

void COracleCursor::SetEnvhp(OCIEnv* apoEnvhp)
{
    this->mpoEnvhp = apoEnvhp;

    if (apoEnvhp == APL_NULL)
    {
        return;
    }

    //we can malloc our errhp now, delete at ~Impl()
    (void) OCIHandleAlloc( (dvoid *) this->mpoEnvhp, 
            (dvoid **) &this->mpoErrhp, OCI_HTYPE_ERROR,
            (size_t) 0, (dvoid **) 0); 
    ACL_ASSERT(this->mpoErrhp != APL_NULL);

}

void COracleCursor::SetStmthp(OCIStmt* apoStmthp)
{
    this->mpoStmthp = apoStmthp;
}

void COracleCursor::SetSvchp(OCISvcCtx* apoSvchp)
{
    this->mpoSvchp = apoSvchp;
}

void COracleCursor::SetOracleConnImpl(COracleConnection* apoConnImpl)
{
    this->mpoOracleConnImpl = apoConnImpl;
}

apl_int_t COracleCursor::CheckErr(sword aoStatus, char const* apcLogmsg)
{
    apl_int_t liRet = -1;
    text lacErrbuf[512];
    sb4 loErrcode = 0;

    this->miErrorCode = 0;

    if (!this->moErrMsg.empty()) 
    {
        this->moErrMsg.clear();
    }

    switch (aoStatus)
    {   
        case OCI_SUCCESS:
        case OCI_SUCCESS_WITH_INFO:
            liRet = 0;
            break;
        case OCI_NEED_DATA:
            this->moErrMsg = "Error - OCI_NEED_DATA\n";
            break;
        case OCI_NO_DATA:
            this->moErrMsg = "Error - OCI_NODATA\n";
            break;
        case OCI_ERROR:
            (void) OCIErrorGet((dvoid *)this->mpoErrhp, (ub4) 1, (text *) NULL, 
                    &loErrcode, lacErrbuf, (ub4) sizeof(lacErrbuf), 
                    OCI_HTYPE_ERROR);
            this->moErrMsg = (char*)lacErrbuf;
            this->miErrorCode = loErrcode;
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

apl_int_t COracleCursor::SetCharacters(const char* apcCharacterSet)
{
    this->miErrorCode = -1;

    this->moErrMsg.assign("Not support. Only available for mysql!");

    return -1;
}

ADL_NAMESPACE_END
