///////////////////////////////////////////////////////////
//  ParamBind.cpp
//  Public class of the parameters binding (for all DB)
//  Created on:      17-11-2009 15:23:00
//  Original author: hezk
///////////////////////////////////////////////////////////

#include "ParamBind.h"


ADL_NAMESPACE_START


CParamBind::CParamBind() 
{}

CParamBind::~CParamBind() 
{}

apl_int_t CParamBind::FindParam(const char* lpoName)
{
    apl_size_t luIdx;

    for (luIdx = 0; luIdx < moParams.size(); ++luIdx)
    {
        if (moParams[luIdx].moParamName == lpoName)
        {
            return luIdx;
        }
    }

    return -1;
}

apl_int_t CParamBind::ParseSql(const char* apcSql)
{
    apl_int_t liRet = 0;
    char* lpcCur = (char*)apcSql;
    char* lpcTmp = APL_NULL;
    char* lpcTypeEnd = APL_NULL;
    CParams loParam;
    std::string loString;
    apl_size_t luDuplicated = 0; //how many params are duplicated

    this->moSqlOrig = apcSql;
    //clear the last result
    this->moParams.clear();
    this->moSqlParesed.clear();

    //parse sql
    while (*lpcCur) 
    {
        //param start
        if(*lpcCur==':' && 
                ((lpcCur > apcSql && *(lpcCur-1)!='\\') || lpcCur == apcSql))
        {
            lpcTypeEnd = APL_NULL;
            ++lpcCur;
            lpcTmp = lpcCur;
            while (apl_isalnum(*lpcCur) || *lpcCur=='_')
                lpcCur++;

            //invalid parameter name
            if (lpcTmp == lpcCur) 
            {
                return BIND_INVALID_PARAMETER_NAME;
            }

            loParam.moParamName.assign(lpcTmp, lpcCur - lpcTmp);
            loParam.mpvParamAddr = APL_NULL;
            loParam.miType = 0;
            loParam.muStartPos = lpcTmp - apcSql;
            loParam.muEndPos = lpcCur - apcSql;

            //duplicated param
            liRet = FindParam(loParam.moParamName.c_str());
            if (liRet != -1)
            {
                luDuplicated++; 
                loParam.miDuplicated = liRet;
                this->moParams.push_back(loParam);
                continue;
            }

            loParam.miDuplicated = -1;
            this->moParams.push_back(loParam);
        }

        ++lpcCur;
    }

#if 0
    for (luIdx = 0; luIdx < auParamCount + luDuplicated; ++luIdx)
    {
        if (moParams[luIdx].miDuplicated >= 0)
        {
            moParams[luIdx].mpvParamAddr = 
                moParams[moParams[luIdx].miDuplicated].mpvParamAddr;
            continue;
        }

        moParams[luIdx].mpvParamAddr = va_arg(asValist, void*);
    }
#endif

    //substitue parameters based on different DB
    liRet = this->SqlSubstitute();

    return liRet;
}


apl_int_t CParamBind::GetParamCount()
{
    return this->moParams.size();
}

void* CParamBind::GetParam(
        apl_size_t auIndex, 
        apl_int16_t* apiType, 
        apl_uint32_t* apuLength,
        apl_size_t** appuContentSize)
{
    if (auIndex +1 > moParams.size())
    {
        return APL_NULL;
    }

    if (apiType != APL_NULL)
    {
        *apiType = this->moParams[auIndex].miType;
    }

    if (apuLength != APL_NULL)
    {
        *apuLength = this->moParams[auIndex].muLength;
    }

    //only for mysql
    if (appuContentSize != APL_NULL)
    {
        *appuContentSize = &(this->moParams[auIndex].muContentSize);
    }

    return this->moParams[auIndex].mpvParamAddr;
}

const char* CParamBind::GetParasedSql()
{
    if (this->moSqlParesed.empty())
    {
        return APL_NULL;
    }

    return this->moSqlParesed.c_str();
}


ADL_NAMESPACE_END
