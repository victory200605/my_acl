
#ifndef AAF_SERVICE_H
#define AAF_SERVICE_H

#include "aaf/Utility.h"

AAF_NAMESPACE_START

class IService
{
public:
    virtual ~IService() {}
    
    virtual apl_int_t Startup( char const* apcConfigFileName ) = 0;
    
    virtual apl_int_t Restart(void) = 0;
    
    virtual apl_int_t Shutdown(void) = 0;
};

AAF_NAMESPACE_END

#endif//AAF_SERVICE_H
