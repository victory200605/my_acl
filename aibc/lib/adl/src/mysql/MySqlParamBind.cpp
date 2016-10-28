///////////////////////////////////////////////////////////
//  MySqlParamBind.cpp
//  Mysql class of the parameters binding
//  Created on:      17-11-2009 15:23:00
//  Original author: hezk
///////////////////////////////////////////////////////////

#include "MySqlParamBind.h"


ADL_NAMESPACE_START
///////////////////////////////////////////////////////////////////////////////

#ifdef ADL_MYSQL_ENABLE

CMySqlParamBind::CMySqlParamBind()
    :mpoStmt(APL_NULL)
    ,mpoBind(APL_NULL)
{}

CMySqlParamBind::~CMySqlParamBind()
{
    if (this->mpoBind != APL_NULL)
    {
        delete [] this->mpoBind;
        this->mpoBind = APL_NULL;
    }
}

void CMySqlParamBind::SetBindingNeed(MYSQL_STMT* apoStmt)
{
    this->mpoStmt = apoStmt;
}

apl_int_t CMySqlParamBind::SqlSubstitute()
{
    apl_int_t liRet = 0;
    apl_size_t luIdx, luSize, luCpSize;
    const char* lpcSqlOrig = APL_NULL;
    char* lpcSqlNew = APL_NULL;
    char* lpcCursorOrig = APL_NULL;
    char* lpcCursorNew = APL_NULL;
    char* lpcCursorNewEnd = APL_NULL;

    //clear in every prepare
    this->mpoStmt = APL_NULL;

    if (this->mpoBind != APL_NULL)
    {
        delete [] this->mpoBind;
        this->mpoBind = APL_NULL;
    }

    luSize = this->moSqlOrig.size();

    if (luSize < 1)
    {
        return -1;
    }

    if (this->moParams.size() == 0)
    {
        //Have not bind any parameter
        this->moSqlParesed = this->moSqlOrig;
        return 0;
    }

    lpcSqlOrig = this->moSqlOrig.c_str();
    ACL_MALLOC_INIT(lpcSqlNew, char, luSize+1, 0)
    lpcCursorNew = lpcSqlNew;
    lpcCursorNewEnd = lpcSqlNew + luSize;
    lpcCursorOrig = (char*)lpcSqlOrig;
    luCpSize = moParams[0].muStartPos;

    for (luIdx=0; luIdx<moParams.size(); ++luIdx)
    {
        apl_memcpy(lpcCursorNew, lpcCursorOrig, luCpSize);
        lpcCursorNew += luCpSize;

        liRet = apl_snprintf(lpcCursorNew-1, 
                lpcCursorNewEnd-lpcCursorNew+1, "? ");

        if (liRet > lpcCursorNewEnd-lpcCursorNew+1 || liRet < 0) 
        {
            liRet = -1;
            break;
        }
        else 
            liRet = 0;

        lpcCursorNew = lpcSqlNew + apl_strlen(lpcSqlNew);
        lpcCursorOrig = (char*)lpcSqlOrig + moParams[luIdx].muEndPos;

        if (luIdx+1 < moParams.size())
        {
            luCpSize = moParams[luIdx+1].muStartPos - moParams[luIdx].muEndPos;
        }
        else //last param
        {
            luCpSize = luSize +1 - moParams[luIdx].muEndPos;
        }
    }

    //copy the last params
    apl_memcpy(lpcCursorNew, lpcCursorOrig, luCpSize);

    //store to std::string for better mem managerment
    if (liRet == 0)
    {
        this->moSqlParesed = lpcSqlNew;
    }

    ACL_FREE(lpcSqlNew);

    return liRet;
}

apl_int_t CMySqlParamBind::UpdateParamContentLen()
{
    apl_size_t luSize;

    for (luSize = 0; luSize < moParams.size(); ++luSize)
    {
        if (moParams[luSize].mpvParamAddr == APL_NULL)
        {
            return -1;
        }

        switch (moParams[luSize].miType)
        {
            case MYSQL_TYPE_STRING:
            case MYSQL_TYPE_VAR_STRING:
                moParams[luSize].muContentSize = 
                    apl_strlen((char*)moParams[luSize].mpvParamAddr);
                break;
            case MYSQL_TYPE_BLOB:
            case MYSQL_TYPE_TINY_BLOB:
            case MYSQL_TYPE_MEDIUM_BLOB:
            case MYSQL_TYPE_LONG_BLOB:
            default:
                moParams[luSize].muContentSize = moParams[luSize].muLength;
                break;
        }
    }

    return 0;
}

#if 0
//string to type
apl_int_t CMySqlParamBind::ParseParamType(
        std::string const & aoString, 
        CParams* apoParam)
{
    if (aoString.compare("short") == 0)
    {
    }
    else if (aoString.compare("int") == 0)
    {
        apoParam->miType = MYSQL_TYPE_LONG;
        //apoParam->muLength = sizeof(int);
        apoParam->muLength = 0;
    }
    else if (aoString.compare("bigint") == 0)
    {
        apoParam->miType = MYSQL_TYPE_LONGLONG;
        //apoParam->muLength = sizeof(apl_int64_t);
        apoParam->muLength = 0;
    }
#if 1
    else if (aoString.compare("uint") == 0)
    {
        //FIXME, mysql don't have this type in binding
        apoParam->miType = MYSQL_TYPE_LONG;
        //apoParam->muLength = sizeof(unsigned int);
        apoParam->muLength = 0;
    }
#endif
    else if (aoString.compare("char") == 0)
    {
        apoParam->miType = MYSQL_TYPE_STRING;
    }
    else if (aoString.compare("vchar") == 0)
    {
        apoParam->miType = MYSQL_TYPE_VAR_STRING;
    }
#if 0
    else if (aoString.compare("longchar") == 0)
    {
        //blob/text
        apoParam->miType = MYSQL_TYPE_LONG_BLOB;
    }
    else if (aoString.compare("uchar") == 0)
    {
        //FIXME!! not sure this prepresent the UNICODE or not!!
        apoParam->miType = MYSQL_TYPE_BLOB;
    }
#endif
    else if (aoString.compare("float") == 0)
    {
        apoParam->miType = MYSQL_TYPE_FLOAT;
        //apoParam->muLength = sizeof(float);
        apoParam->muLength = 0;
    }
    else if (aoString.compare("double") == 0)
    {
        apoParam->miType = MYSQL_TYPE_DOUBLE;
        //apoParam->muLength = sizeof(double);
        apoParam->muLength = 0;
    }
    else 
    {
        return -1;
    }

    return 0;
}
#endif

void CMySqlParamBind::SetDuplicatedParams(apl_int_t aiIdx)
{
    apl_size_t liIdx;

    for (liIdx = aiIdx+1; liIdx < moParams.size(); ++liIdx)
    {
        //duplicated
        if (moParams[liIdx].miDuplicated == aiIdx)
        {
            moParams[liIdx].mpvParamAddr = 
                moParams[aiIdx].mpvParamAddr;
            moParams[liIdx].miType = 
                moParams[aiIdx].miType;
            moParams[liIdx].muLength = 
                moParams[aiIdx].muLength;
        }
    }
}

apl_int_t CMySqlParamBind::TryBinding()
{
    apl_size_t luIdx;
    apl_size_t luCount;
    apl_int16_t liType = 0;
    apl_uint32_t luLength = 0;
    apl_size_t* lpuContentSize = APL_NULL;
    MYSQL_BIND* lpoBindCur = APL_NULL;
    void* lpvParamAddr = APL_NULL;

    luCount = this->moParams.size();

    for (luIdx = 0; luIdx < luCount; ++luIdx)
    {
        if (moParams[luIdx].mpvParamAddr == APL_NULL) 
        {
            break;
        }
    }

    if (luIdx < luCount)
    {
        return 0;
    }

    if (this->mpoStmt == APL_NULL)
    {
        return -2;
    }

    if (this->mpoBind != APL_NULL)
    {
        delete [] this->mpoBind;
        this->mpoBind = APL_NULL;
    }

    this->mpoBind = new MYSQL_BIND[luCount];

    if (this->mpoBind == APL_NULL) { return -2; }

    apl_memset(this->mpoBind, 0, sizeof(MYSQL_BIND)*luCount);
    //init all the bind param
    for (luIdx = 0; luIdx < luCount; ++luIdx)
    {   
        lpoBindCur = this->mpoBind + luIdx;
        lpvParamAddr = this->GetParam(luIdx, 
                &liType, &luLength, &lpuContentSize);

        if (lpvParamAddr == APL_NULL) 
        {   
            return -2;
        }   

        lpoBindCur->buffer = (char *)lpvParamAddr;
        lpoBindCur->buffer_type = (enum_field_types)liType;

        if (luLength != 0)  
        {
            lpoBindCur->buffer_length = luLength;
        }

        lpoBindCur->is_null= 0;
        lpoBindCur->length = (unsigned long*)lpuContentSize;
    }   

    if (mysql_stmt_bind_param(mpoStmt, mpoBind))
    {
        return -1;
    }

    return 0;
}

apl_int_t CMySqlParamBind::BindParamCommon(
        const char* apcParamName, 
        void* apvParam, 
        apl_int16_t aiType,
        apl_size_t auLen)
{
    apl_int_t liIdx;
    liIdx = FindParam(apcParamName);

    if (liIdx == -1 || apvParam == APL_NULL)
    {
        return -2;
    }

    moParams[liIdx].mpvParamAddr = apvParam;
    moParams[liIdx].miType = aiType;
    moParams[liIdx].muLength = auLen;

    this->SetDuplicatedParams(liIdx);

    return this->TryBinding();
}

apl_int_t CMySqlParamBind::BindParam(const char* apcParamName, apl_int16_t* apiParam)
{
    return BindParamCommon(apcParamName, (void*)apiParam, MYSQL_TYPE_SHORT, 0);
}

apl_int_t CMySqlParamBind::BindParam(const char* apcParamName, apl_int32_t* apiParam)
{
    return BindParamCommon(apcParamName, (void*)apiParam, MYSQL_TYPE_LONG, 0);
}

apl_int_t CMySqlParamBind::BindParam(const char* apcParamName, apl_int64_t* apiParam)
{
    return BindParamCommon(apcParamName, (void*)apiParam, MYSQL_TYPE_LONGLONG, 0);
}

apl_int_t CMySqlParamBind::BindParam(const char* apcParamName, apl_uint32_t* apuParam)
{
    return BindParamCommon(apcParamName, (void*)apuParam, MYSQL_TYPE_LONG, 0);
}

apl_int_t CMySqlParamBind::BindParam(const char* apcParamName, const char* apcParam, apl_size_t auLen)
{
    return BindParamCommon(apcParamName, (void*)apcParam, MYSQL_TYPE_STRING, auLen);
}

apl_int_t CMySqlParamBind::BindParam(const char* apcParamName, const void* apcParam, apl_size_t auLen, bool abIsBinary)
{
    if (abIsBinary)
    {
        return BindParamCommon(apcParamName, (void*)apcParam, MYSQL_TYPE_BLOB, auLen);
    }
    else
    {
        return BindParamCommon(apcParamName, (void*)apcParam, MYSQL_TYPE_STRING, auLen);
    }
}

apl_int_t CMySqlParamBind::BindParam(const char* apcParamName, float* apfParam)
{
    return BindParamCommon(apcParamName, (void*)apfParam, MYSQL_TYPE_FLOAT, 0);
}

apl_int_t CMySqlParamBind::BindParam(const char* apcParamName, double* apdParam)
{
    return BindParamCommon(apcParamName, (void*)apdParam, MYSQL_TYPE_DOUBLE, 0);
}


ADL_NAMESPACE_END

#endif

