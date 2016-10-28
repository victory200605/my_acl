
#ifndef AIBC_CACHESERVER_H
#define AIBC_CACHESERVER_H

#define AIBC_CACHESERVER_NAMESPACE_START namespace cacheserver2 {
#define AIBC_CACHESERVER_NAMESPACE_END };

namespace cacheserver2 {

enum
{
    NO_ERROR                 =  0,
    WARN_NO_FOUND            =  70001,
    ERROR_OPEN_FILE          = -70001,
    ERROR_WRITE_FILE         = -70002,
    ERROR_READ_FILE          = -70003,
    ERROR_SYNC_FILE          = -70004,
    ERROR_INVALID_FILE       = -70005,
    ERROR_OUT_OF_RANGE       = -70006,
    ERROR_DEL_FREE_NODE      = -70007,
    ERROR_INVALID_DATA       = -70008,
    ERROR_CACHE_FLOW         = -70009,
    ERROR_UNOPENED           = -70010,
    ERROR_DUP_RECORD         = -70011
};

};

#endif//AIBC_CACHESERVER_H

