
#ifndef AIBC_GFQ2_CONFIG_H
#define AIBC_GFQ2_CONFIG_H

#include "gfq2/Utility.h"
#include "acl/stl/vector.h"
#include "acl/stl/string.h"

AIBC_GFQ_NAMESPACE_START

typedef void (*UpdateHandlerFunc)(void);
typedef std::vector<std::vector<std::string> > TableType;

/** 
 * @brief Global File Queue config abstract interface
 */
class IGFQConfig
{
public:
    virtual ~IGFQConfig(void) {};
    
    virtual apl_int_t Initialize( apl_int_t aiArgc, char* argv[] ) = 0;
    
    virtual void Close(void) = 0;
    
    virtual void SetUpdateHandler( UpdateHandlerFunc apfHandler ) = 0;
    
    virtual bool GetValue( char const* apcField, char const* apcKey, std::string& aoVal ) = 0;

    virtual bool GetValue( char const* apcField, char const* apcKey, apl_int64_t& aiVal ) = 0;

    virtual bool GetValue( char const* apcField, char const* apcKey, apl_int32_t& aiVal ) = 0;
    
    virtual bool GetTable( char const* apcName, TableType& aoTable ) = 0;
    
    virtual bool IsFieldExisted( char const* apcField ) = 0;
};

AIBC_GFQ_NAMESPACE_END

#endif//AIBC_GFQ_CONFIG_H

