#ifndef AILIB_H_2007
#define AILIB_H_2007
#include <new>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#ifndef WIN32
#include <inttypes.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <signal.h>
#include <limits.h>
#include <fcntl.h>
#include <dirent.h>
#include <limits.h>
#include <pthread.h>
#include <semaphore.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <poll.h>
#else
#include <io.h>
#include <fcntl.h>
#endif //WIN32

#ifndef INET_ADDRSTRLEN
// 16 == strlen( "xxx.xxx.xxx.xxx" ) + 1;
#  define INET_ADDRSTRLEN 16
#endif

#define AI_NEW(ptr, obj)                try{ptr = new obj; }catch(...){ ptr = NULL; } 
#define AI_NEW_ASSERT(ptr, obj)         {AI_NEW(ptr,obj); assert(NULL != ptr); }
#define AI_NEW_RETURN(ptr, obj, ret)    {AI_NEW(ptr,obj); if(NULL == ptr){return ret;} } 
#define AI_DELETE(ptr)                  {delete ptr; ptr = NULL; } 
#define AI_DELETE_TYPE(ptr, type)		{delete (type)ptr; ptr = NULL; }

#define AI_NEW_ARRAY(ptr, obj, num)             try{ptr = new obj[num]; }catch(...){ ptr = NULL; }
#define AI_NEW_ARRAY_ASSERT(ptr, obj, num )     {AI_NEW_ARRAY(ptr, obj, num); assert(ptr != NULL); }
#define AI_NEW_ARRAY_RETURN(ptr, obj, num, ret) {AI_NEW_ARRAY(ptr,obj,num); if(NULL == ptr){return ret;} }
#define AI_DELETE_ARRAY(ptr)                    {delete[] ptr; ptr = NULL; }


#define AI_MALLOC(ptr, type, size)              {ptr = (type*)malloc(size); memset(ptr, 0x0, size);}
#define AI_MALLOC_ASSERT(ptr, type, size)       {AI_MALLOC(ptr, type, size); assert(ptr); }
#define AI_MALLOC_RETURN(ptr, type, size, ret)  {AI_MALLOC(ptr, type, size); if(NULL == ptr){return ret;} }
#define AI_FREE(ptr)                            {free(ptr); ptr = NULL; }

#define AI_REALLOC(ptr, type, size)             {ptr = (type)realloc(ptr, size); }
#define AI_REALLOC_ASSERT(ptr, type, size)      {AI_REALLOC(ptr, type, size); assert(ptr); }
#define AI_REALLOC_RETURN(ptr, type, size, ret) {type ptr2=ptr; AI_REALLOC(ptr2, type, size); if(NULL == ptr2){return ret;} ptr=ptr2; }


#define AI_BITSOF(T) (sizeof(T)*8)
#define AI_OFFSET(type, member)  ( (unsigned int) &( ((type*)sizeof(void*))->member)  - sizeof(void*) )
#define AI_GET_STRUCT_PTR(ptr, type, member)  (  (type *) ( (char *)(ptr) - AI_OFFSET(type, member) )  )


#define AI_MIN( v1, v2 ) (v1 < v2 ? v1 : v2)
#define AI_MAX( v1, v2 ) (v1 > v2 ? v1 : v2)

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

#endif /* AILIB_H_2007 */

