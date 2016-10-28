
///////////////////////////////////////////////////////////
//  OracleParamBind.cpp
//  Oracle class of the parameters binding 
//  Created on:      17-11-2009 15:23:00
//  Original author: hezk
///////////////////////////////////////////////////////////


#include "OracleParamBind.h"
///////////////////////////////////////////////////////////////////////////////

ADL_NAMESPACE_START

#ifdef ADL_ORACLE_ENABLE
COracleParamBind::COracleParamBind()
    :mpoOciStmt(APL_NULL)
    ,mpoOciErr(APL_NULL)
{}

COracleParamBind::~COracleParamBind()
{}

//substitute params with pos var. e.g. :1, :2, :3 ...
apl_int_t COracleParamBind::SqlSubstitute()
{
    apl_int_t liRet = 0;
    apl_size_t luIdx, luSize, luCpSize;
    const char* lpcSqlOrig = APL_NULL;
    char* lpcSqlNew = APL_NULL;
    char* lpcCursorOrig = APL_NULL;
    char* lpcCursorNew = APL_NULL;
    char* lpcCursorNewEnd = APL_NULL;

    luSize = this->moSqlOrig.size();

    if (luSize < 1) { return -1; }

    //clear
    this->mpoOciStmt = APL_NULL;
    this->mpoOciErr = APL_NULL;

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

        liRet = apl_snprintf(lpcCursorNew, 
                lpcCursorNewEnd-lpcCursorNew+1, "%d", luIdx+1);

        if (liRet > lpcCursorNewEnd-lpcCursorNew+1 || liRet < 0) 
        {
            liRet = -1;
            break;
        }
        else 
        {
            liRet = 0;
        }

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

#if 0
//string to type
apl_int_t COracleParamBind::ParseParamType(
        std::string const & aoString, 
        CParams* apoParam)
{
    //FIXME!! some type also need to set the length, e.g. int, double
    
    if (aoString.compare("short") == 0)
    {
        apoParam->miType = SQLT_INT;
        apoParam->muLength = sizeof(short int);
    }
    else if (aoString.compare("int") == 0)
    {
        apoParam->miType = SQLT_INT;
        apoParam->muLength = sizeof(int);
    }
    else if (aoString.compare("long") == 0)
    {
        apoParam->miType = SQLT_INT;
        apoParam->muLength = sizeof(long);
    }
    else if (aoString.compare("bigint") == 0)
    {
        apoParam->miType = SQLT_INT;
        apoParam->muLength = sizeof(apl_int64_t);
    }
    else if (aoString.compare("uint") == 0)
    {
        apoParam->miType = SQLT_UIN;
        apoParam->muLength = sizeof(unsigned int);
    }
    else if (aoString.compare("char") == 0 || aoString.compare("vchar") == 0)
    {
        apoParam->miType = SQLT_STR;
        //apoParam->miType = SQLT_AVC;
    }
#if 0
    else if (aoString.compare("longchar") == 0)
    {
        apoParam->miType = SQLT_LVC;
    }
    else if (aoString.compare("uchar") == 0)
    {
        //UNICODE
        apoParam->miType = SQLT_VCS;
    }
#endif
    else if (aoString.compare("float") == 0)
    {
        apoParam->miType = SQLT_BFLOAT;
        apoParam->muLength = sizeof(float);
    }
    else if (aoString.compare("double") == 0)
    {
        apoParam->miType = SQLT_BDOUBLE;
        apoParam->muLength = sizeof(double);
    }
    else 
    {
        return -1;
    }

    return 0;
}
#endif

void COracleParamBind::SetBindingNeed(OCIStmt* apoOciStmt, OCIError* apoOciErr)
{
    this->mpoOciStmt = apoOciStmt;
    this->mpoOciErr = apoOciErr;
}

apl_int_t COracleParamBind::OciBingByPosWraper(
        apl_size_t luIdx,
        void* apvParam, 
        apl_int16_t aiType,
        apl_size_t auLen)
{
    OCIBind *lpoBndhp = APL_NULL; //not use here
    moParams[luIdx].mpvParamAddr = apvParam;
    moParams[luIdx].miType = aiType;
    moParams[luIdx].muLength = auLen;

    //sb2 li2Indp = -1;

    if (OCIBindByPos(
                this->mpoOciStmt, &lpoBndhp, this->mpoOciErr,
                luIdx+1,(dvoid *)apvParam, (sword)auLen,
                (ub2)aiType, NULL, (ub2 *) 0,(ub2 *) 0,(ub4) 0,
                (ub4 *)0,OCI_DEFAULT) != 0)
    {
        return -1;
    }

    return 0;
}

apl_int_t COracleParamBind::BindDuplicatedParams(apl_int_t aiIdx)
{
    apl_size_t luIdx;
    apl_int_t liRet = 0;
    void* lpvParam = moParams[aiIdx].mpvParamAddr;
    apl_int16_t liType = moParams[aiIdx].miType;
    apl_uint32_t luLen = moParams[aiIdx].muLength;

    for (luIdx = aiIdx+1; luIdx < moParams.size(); ++luIdx)
    {
        //duplicated
        if (moParams[luIdx].miDuplicated == aiIdx)
        {
            liRet = this->OciBingByPosWraper(luIdx, lpvParam, liType, luLen);

            if (liRet != 0)
            {
                break;
            }
        }
    }

    return liRet;
}

apl_int_t COracleParamBind::BindParamCommon(
        const char* apcParamName, 
        void* apvParam, 
        apl_int16_t aiType,
        apl_size_t auLen)
{
    apl_int_t liIdx;
    apl_int_t liRet;
    liIdx = FindParam(apcParamName);

    if (liIdx == -1 || apvParam == APL_NULL)
    {
        return -2;
    }

    liRet = this->OciBingByPosWraper(liIdx, apvParam, aiType, auLen);

    if (liRet != 0) 
    {
        return liRet;
    }

    return BindDuplicatedParams(liIdx);
}

apl_int_t COracleParamBind::BindParam(const char* apcParamName, apl_int16_t* apiParam)
{
    return this->BindParamCommon(
            apcParamName, (void*)apiParam, SQLT_INT, sizeof(short int));
}

apl_int_t COracleParamBind::BindParam(const char* apcParamName, apl_int32_t* apiParam)
{
    return this->BindParamCommon(
            apcParamName, (void*)apiParam, SQLT_INT, sizeof(int));
}

apl_int_t COracleParamBind::BindParam(const char* apcParamName, apl_int64_t* apiParam)
{
    return this->BindParamCommon(
            apcParamName, (void*)apiParam, SQLT_INT, sizeof(long long));
}

apl_int_t COracleParamBind::BindParam(const char* apcParamName, apl_uint32_t* apuParam)
{
    return this->BindParamCommon(
            apcParamName, (void*)apuParam, SQLT_UIN, sizeof(unsigned int));
}

apl_int_t COracleParamBind::BindParam(const char* apcParamName, const char* apcParam, apl_size_t auLen)
{
    return this->BindParamCommon(
            apcParamName, (void*)apcParam, SQLT_STR, auLen);
}

apl_int_t COracleParamBind::BindParam(const char* apcParamName, const void* apcParam, apl_size_t auLen, bool abIsBinary)
{
    if (abIsBinary)
    {
        return this->BindParamCommon(
            //apcParamName, (void*)apcParam, SQLT_LNG, auLen);
            apcParamName, (void*)apcParam, SQLT_LBI, auLen);
    }
    else
    {
        return this->BindParamCommon(
            apcParamName, (void*)apcParam, SQLT_STR, auLen);
    }
}

apl_int_t COracleParamBind::BindParam(const char* apcParamName, float* apfParam)
{
    return this->BindParamCommon(
            apcParamName, (void*)apfParam, SQLT_FLT, sizeof(float));
}

apl_int_t COracleParamBind::BindParam(const char* apcParamName, double* apdParam)
{
    return this->BindParamCommon(
            apcParamName, (void*)apdParam, SQLT_FLT, sizeof(double));
}


ADL_NAMESPACE_END

#endif
