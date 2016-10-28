///////////////////////////////////////////////////////////
//  OracleParamBind.h
//  Oracle class of the parameters binding 
//  Created on:      17-11-2009 15:23:00
//  Original author: hezk
///////////////////////////////////////////////////////////

#if !defined(ADL_ORACLE_PARAM_BIND_H)
#define ADL_ORACLE_PARAM_BIND_H

#include "../ParamBind.h"

#ifdef ADL_ORACLE_ENABLE
#include <oci.h>


ADL_NAMESPACE_START


class COracleParamBind : public CParamBind
{
public:
    COracleParamBind();

    ~COracleParamBind();

    apl_int_t OciBingByPosWraper(
            apl_size_t liIdx,
            void* apvParam, 
            apl_int16_t aiType,
            apl_size_t auLen);

    apl_int_t BindDuplicatedParams(apl_int_t aiIdx);

    void SetBindingNeed(OCIStmt* apoOciStmt, OCIError* apoOciErr);

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

    virtual apl_int_t UpdateParamContentLen() {return APL_NULL;}

    virtual apl_int_t SqlSubstitute();

    //virtual apl_int_t ParseParamType(std::string const & aoString, 
            //CParams* apoParam);

private:

    OCIStmt* mpoOciStmt;
    OCIError* mpoOciErr;
};

ADL_NAMESPACE_END

#endif //ADL_ORACLE_ENABLE

#endif //ADL_ORACLE_PARAM_BIND_H
