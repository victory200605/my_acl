#ifndef __AILIBEX__AILIB_H__
#define __AILIBEX__AILIB_H__

#if defined(__cplusplus)
#   define  __STDC_LIMIT_MACROS
#   define  __STDC_FORMAT_MACROS
#   define  __STDC_CONSTANT_MACROS
#endif

#include <new>

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <inttypes.h>
#include <stddef.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdarg.h>
#include <signal.h>
#include <fcntl.h>
#include <dirent.h>
#include <limits.h>
#include <pthread.h>
#include <semaphore.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <poll.h>
#include <dlfcn.h>

// define INET_ADDRSTRLEN if not existed
#ifndef INET_ADDRSTRLEN
#   define INET_ADDRSTRLEN   16
#endif

// define PATH_MAX if not existed
#ifndef PATH_MAX
#   define _AILIBEX_PATH_MAX (1024)
#   if   defined(_XOPEN_PATH_MAX) && (_XOPEN_PATH_MAX >= _AILIBEX_PATH_MAX)
#       define PATH_MAX _XOPEN_PATH_MAX
#   elif defined(_POSIX_PATH_MAX) && (_POSIX_PATH_MAX >= _AILIBEX_PATH_MAX)
#       define PATH_MAX _POSIX_PATH_MAX
#   else 
#       define PATH_MAX _AILIBEX_PATH_MAX
#   endif
#   undef _AILIBEX_PATH_MAX
#endif    

#ifndef PRIuMAX
#   define PRIuMAX "llu"
#endif

#ifndef PRIdMAX
#   define PRIdMAX "lld"
#endif

/////////////////////////////////////////////////////////////////////////
//
#define AI_NEW(ptr, type)               {try{ptr=new type;}catch(...){ptr=NULL;}}
#define AI_NEW_ASSERT(ptr, type)        {AI_NEW(ptr,type);assert(NULL!=ptr);}
#define AI_NEW_INIT_ASSERT(ptr, type, ...)  {try{ptr=new type(__VA_ARGS__);}catch(...){ptr=NULL;}assert(NULL!=ptr);}
#define AI_DELETE(ptr)                  {delete ptr; ptr=NULL;} 
#define AI_DELETE_TYPE(ptr, type)       {delete (type*)ptr; ptr=NULL;}

#define AI_NEW_N(ptr, type, num)        {try{ptr=new type[num];}catch(...){ptr=NULL;}}
#define AI_NEW_N_ASSERT(ptr, type, num) {AI_NEW_N(ptr,type,num);assert(NULL!=ptr);}
#define AI_DELETE_N(ptr)                {delete[] ptr; ptr=NULL;}
#define AI_DELETE_N_TYPE(ptr, type)     {delete[] (type*)ptr; ptr=NULL;}

/////////////////////////////////////////////////////////////////////////
//
#define AI_MALLOC(ptr, type, size)              {ptr=(type*)malloc(size);}
#define AI_MALLOC_ASSERT(ptr, type, size)       {AI_MALLOC(ptr,type,size);assert(ptr);}
#define AI_MALLOC_INIT(ptr, type, size, val)    {AI_MALLOC_ASSERT(ptr,type,size);memset(ptr,val,size);}
#define AI_REALLOC(ptr, type, size)             {ptr = (type*)realloc(ptr,size);}
#define AI_REALLOC_ASSERT(ptr, type, size)      {AI_REALLOC(ptr,type,size);assert(size > 0 && ptr);}
#define AI_FREE(ptr)                            {free(ptr); ptr=NULL;}

/////////////////////////////////////////////////////////////////////////
//
#define AI_BITSOF(type)     (sizeof(type)*8)

#define AI_ARRAY_SIZE(arr)  (sizeof(arr)/sizeof(arr[0]))

#define AI_OFFSET(stype, member)                (((ptrdiff_t)&(((stype*)sizeof(void*))->member)) - sizeof(void*))
#define AI_GET_STRUCT_PTR(mp, stype, member)    ((stype*)(((char*)mp) - AI_OFFSET(stype,member)))

/////////////////////////////////////////////////////////////////////////
//
#define AI_MIN(v1, v2) ((v1) < (v2) ? (v1) : (v2))
#define AI_MAX(v1, v2) ((v1) > (v2) ? (v1) : (v2))
#define AI_SWAP(v1, v2, type) {type v3 = v1; v1 = v2; v2 = v3;}

////aibc namespace
#if defined(_AI_ENABLE_NAMESPACE)
#   define AIBC AIBC
#   define AIBC_NAMESPACE_START namespace AIBC {
#   define AIBC_NAMESPACE_END   }
#else
#   define AIBC
#   define AIBC_NAMESPACE_START
#   define AIBC_NAMESPACE_END
#endif

///start namespace
AIBC_NAMESPACE_START

template < typename T >
T AIMin( T const& aVal1, T const& aVal2 )
{
    return  aVal1 < aVal2 ? aVal1 : aVal2;
}

template < typename T >
T AIMax( T const& aVal1, T const& aVal2 )
{
    return  aVal1 > aVal2 ? aVal1 : aVal2;
}

template < typename T >
void AISwapValue( T& aVal1, T& aVal2 )
{
    T lTmp = aVal1;
    aVal1 = aVal2;
    aVal2 = lTmp;
}

///end namespace
AIBC_NAMESPACE_END

/////////////////////////////////////////////////////////////////////////

#define AI_RETURN_IF(ret, exp) if(exp) { return ret; }
#define AI_GOTO_IF( to, exp )  if(exp) { goto to; }

#define AI_RAND_NUM(min, max) (min+(int)(double(max)*rand()/(RAND_MAX+1.0))

#endif // __AILIBEX__AILIB_H__
