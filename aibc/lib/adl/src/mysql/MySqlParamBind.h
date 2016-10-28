///////////////////////////////////////////////////////////
//  MySqlParamBind.h
//  Mysql class of the parameters binding
//  Created on:      17-11-2009 15:23:00
//  Original author: hezk
///////////////////////////////////////////////////////////

#if !defined(ADL_MYSQL_PARAM_BIND_H)
#define ADL_MYSQL_PARAM_BIND_H
///////////////////////////////////////////////////////////////////////////////
#include "../ParamBind.h"

#ifdef ADL_MYSQL_ENABLE

#include <mysql/mysql.h>


ADL_NAMESPACE_START


class CMySqlParamBind :public CParamBind
{
public:
    CMySqlParamBind();

    ~CMySqlParamBind();

    void SetDuplicatedParams(apl_int_t aiIdx);

    void SetBindingNeed(MYSQL_STMT*);

    apl_int_t TryBinding();

    apl_int_t BindParamCommon(
            const char* apcParamName, 
            void* apvParam, 
            apl_int16_t aiType,
            apl_size_t auLen);

    apl_int_t BindParam(const char* apcParamName, apl_int16_t* apiParam);

    apl_int_t BindParam(const char* apcParamName, apl_int32_t* apiParam);

    apl_int_t BindParam(const char* apcParamName, apl_int64_t* apiParam);

    apl_int_t BindParam(const char* apcParamName, apl_uint32_t* apuParam);

    apl_int_t BindParam(const char* apcParamName, const char* apcParam, apl_size_t auLen);

    apl_int_t BindParam(const char* apcParamName, const void* apcParam, apl_size_t auLen, bool abIsBinary);

    apl_int_t BindParam(const char* apcParamName, float* apfParam);

    apl_int_t BindParam(const char* apcParamName, double* apdParam);

    virtual apl_int_t UpdateParamContentLen();

    virtual apl_int_t SqlSubstitute();

    //virtual apl_int_t ParseParamType(std::string const & aoString, CParams* apoParam);

private:

    MYSQL_STMT* mpoStmt;
    MYSQL_BIND* mpoBind;

};


ADL_NAMESPACE_END

#endif //ADL_MYSQL_ENABLE

#endif //ADL_MYSQL_PARAM_BIND_H

