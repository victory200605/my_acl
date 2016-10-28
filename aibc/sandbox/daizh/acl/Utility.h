
#ifndef ACL_UTILITY_H
#define ACL_UTILITY_H

#include "apl/apl.h"

#define ACL_NAMESPACE_START namespace acl {

#define ACL_NAMESPACE_END }

#if defined(ACL_ENABLE_EXCEPTION)
#   define ACL_ASSERT( exp ) if( !(exp) ) { throw CException(__FILE__, __LINE__, #exp); }
#   define ACL_ASSERT_MSG( exp, msg ) if( !(exp) ) { throw CException(__FILE__, __LINE__, #exp); }
#else
#   define ACL_ASSERT( exp ) assert( (exp) )
#   define ACL_ASSERT_MSG( exp, msg ) assert( (exp) )
#endif

#define ACL_INVALID_HANDLE -1

/* retry check macro */
#define ACL_IS_RETRY() (errno == EAGAIN || errno == EBUSY ||  errno == EINTR || errno == EWOULDBLOCK)

#define ACL_RETRY_CHK( op, retries )                      \
    do                                                    \
    {                                                     \
        apl_int_t liRetries = retries;                    \
        for ( ;; )                                        \
        {                                                 \
            errno = 0;                                    \
            if ( !(op) )                                  \
            {                                             \
                break;                                    \
            }                                             \
            if ( ACL_IS_RETRY() && --liRetries > 0 )      \
            {                                             \
                continue;                                 \
            }                                             \
            break;                                        \
        }                                                 \
    } while (0)

#define ACL_IS_VALID_SOCKET( handle ) (handle>=0)

#define ACL_NEW(ptr, type)               {try{ptr=new type;}catch(...){ptr=NULL;}}
#define ACL_NEW_ASSERT(ptr, type)        {ACL_NEW(ptr,type);assert(NULL!=ptr);}
#define ACL_NEW_INIT_ASSERT(ptr, type, ...)  {try{ptr=new type(__VA_ARGS__);}catch(...){ptr=NULL;}assert(NULL!=ptr);}
#define ACL_DELETE(ptr)                  {delete ptr; ptr=NULL;} 
#define ACL_DELETE_TYPE(ptr, type)       {delete (type*)ptr; ptr=NULL;}

#define ACL_NEW_N(ptr, type, num)        {try{ptr=new type[num];}catch(...){ptr=NULL;}}
#define ACL_NEW_N_ASSERT(ptr, type, num) {ACL_NEW_N(ptr,type,num);assert(NULL!=ptr);}
#define ACL_DELETE_N(ptr)                {delete[] ptr; ptr=NULL;}
#define ACL_DELETE_N_TYPE(ptr, type)     {delete[] (type*)ptr; ptr=NULL;}

#endif//ACL_ACL_H
