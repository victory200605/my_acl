///////////////////////////////////////////////////////////
//  config.h
//  Implementation of the Class clsBillCache
//  Created on:      15-ÎåÔÂ-2008 14:43:49
//  Original author: dzh
///////////////////////////////////////////////////////////
#ifndef AI_CACHEDEFINE_H_2008
#define AI_CACHEDEFINE_H_2008

#include "cacheserver/CacheUtility.h"
#include "AILogSys.h"
#include <stdarg.h>

#define AI_ENABLE_BILLCACHE_LOG 1
#define AI_BILLCACHE_SYS_LOG "./cache_sys.log"

#if !defined(ASSERT)
#define ASSERT( what, expression ) \
    if ( !(expression) ) \
    { \
        bool what = false; \
        assert( what ); \
    }
#endif

#define AI_NEW_ARRAY AI_NEW_N

#define AI_DELETE_ARRAY AI_DELETE_N

#define AI_NEW_ARRAY_ASSERT AI_NEW_N_ASSERT

#define AI_DELETE_ARRAY_ASSERT AI_DELETE_N_ASSERT

#define AI_BC_NEW_ASSERT( ptr, obj, errwhat ) \
    AI_NEW( ptr, obj ) \
    ASSERT( errwhat, ptr != NULL ); 

#define AI_BC_NEW_ARRAY_ASSERT( ptr, obj, num, errwhat ) \
    AI_NEW_ARRAY( ptr, obj, num ) \
    ASSERT( errwhat, ptr != NULL );
    
#endif
