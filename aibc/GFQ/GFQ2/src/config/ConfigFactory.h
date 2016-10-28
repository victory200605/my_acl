
#ifndef AIBC_GFQ_CONFIGFACTORY_H
#define AIBC_GFQ_CONFIGFACTORY_H

#include "gfq2/GFQConfig.h"

AIBC_GFQ_NAMESPACE_START

class CConfigFactory
{
public:
    static IGFQConfig* Create( char const* apcCmd );

    static void Destroy( IGFQConfig* apoConfigImpl );
};

AIBC_GFQ_NAMESPACE_END

#endif //AIBC_GFQ_CONFIGFACTORY_H

