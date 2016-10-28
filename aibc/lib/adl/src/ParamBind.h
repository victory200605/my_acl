///////////////////////////////////////////////////////////
//  ParamBind.h
//  Public class of the parameters binding (for all DB)
//  Created on:      17-11-2009 15:23:00
//  Original author: hezk
///////////////////////////////////////////////////////////

#if !defined(ADL_PARAM_BIND_H)
#define ADL_PARAM_BIND_H

#include "apl/apl.h"
#include "adl/Adl.h"
#include <string>
#include <vector>


ADL_NAMESPACE_START

/**
 * ADL supports the following type parameters binding curentlly:
 *
 * <short>      ADL_TYPE_SHORT        
 * <int>        ADL_TYPE_LONG  
 * <bigint>     ADL_TYPE_LONGLONG  
 * <double>     ADL_TYPE_DOUBLE    
 * <float>      ADL_TYPE_FLOAT 
 * <char[XXX]>  ADL_TYPE_VARCHAR
 *
 **/

#define BIND_INVALID_PARAMETER_NAME         (-1)
#define BIND_INVALID_PARAMETER_FORMAT       (-2)
#define BIND_INVALID_PARAMETER_LENGTH       (-3)
#define BIND_INVALID_PARAMETER_COUNT        (-4)
#define BIND_INVALID_PARAMETER_TYPE         (-5)

struct CParams
{
    std::string moParamName;

    void* mpvParamAddr; //addr of bind parameter

    apl_int16_t miType;

    apl_uint32_t muLength; //the buffer-len

    apl_size_t muContentSize;  //how many size in buffer

    apl_int_t miDuplicated;

    //start pos at the SQL string, the position right after ':'
    //e.g. ":name<int>",
    apl_size_t muStartPos; 

    //end pos at the SQL string, the position of ritht after '>'
    apl_size_t muEndPos;   
};


class CParamBind
{

public:

    CParamBind();

    virtual ~CParamBind();

    apl_int_t FindParam(const char* lpoName);

    apl_int_t ParseSql(const char* apcSql);

    virtual apl_int_t SqlSubstitute() =0;

    virtual apl_int_t UpdateParamContentLen() =0;

    //virtual apl_int_t ParseParamType(std::string const & aoString, 
            //CParams* apoParams) =0;

    apl_int_t GetParamCount();

    const char* GetParasedSql();

    void* GetParam(
            apl_size_t aiIndex, 
            apl_int16_t* apiType, 
            apl_uint32_t* apuLength,
            apl_size_t** appuContentSize = APL_NULL);

protected:

    std::string moSqlOrig; 
    std::string moSqlParesed;
    std::vector<CParams> moParams;
    
};



ADL_NAMESPACE_END


#endif //!define(ADL_PARAM_BIND_H)
