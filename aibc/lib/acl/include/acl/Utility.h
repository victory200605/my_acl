
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

/* retry check macro */
#define ACL_IS_RETRY() (apl_get_errno() == APL_EINTR)

#define ACL_RETRY_CHK( op, retries )                      \
    {                                                     \
        apl_int_t liRetries =                             \
            retries <= 0 ? APL_INT32_MAX : retries;       \
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
    }

#define ACL_NEW(ptr, type)               {try{ptr=new type;}catch(...){ptr=NULL;}}
#define ACL_NEW_ASSERT(ptr, type)        {ACL_NEW(ptr,type);assert(NULL!=ptr);}
#define ACL_NEW_INIT_ASSERT(ptr, type, ...)  {try{ptr=new type(__VA_ARGS__);}catch(...){ptr=NULL;}assert(NULL!=ptr);}
#define ACL_DELETE(ptr)                  {delete ptr; ptr=NULL;} 
#define ACL_DELETE_TYPE(ptr, type)       {delete (type*)ptr; ptr=NULL;}

#define ACL_NEW_N(ptr, type, num)        {try{ptr=new type[num];}catch(...){ptr=NULL;}}
#define ACL_NEW_N_ASSERT(ptr, type, num) {ACL_NEW_N(ptr,type,num);assert(NULL!=ptr);}
#define ACL_DELETE_N(ptr)                {delete[] ptr; ptr=NULL;}
#define ACL_DELETE_N_TYPE(ptr, type)     {delete[] (type*)ptr; ptr=NULL;}

#define ACL_MALLOC(ptr, type, size)              {ptr=(type*)malloc(size);}
#define ACL_MALLOC_ASSERT(ptr, type, size)       {ACL_MALLOC(ptr,type,size);assert(ptr);}
#define ACL_MALLOC_INIT(ptr, type, size, val)    {ACL_MALLOC_ASSERT(ptr,type,size);memset(ptr,val,size);}
#define ACL_REALLOC(ptr, type, size)             {ptr = (type*)realloc(ptr,size);}
#define ACL_REALLOC_ASSERT(ptr, type, size)      {ACL_REALLOC(ptr,type,size);assert(size > 0 && ptr);}
#define ACL_FREE(ptr)                            {free(ptr); ptr=NULL;}

//Align value 'x' to boundary 'b' which should be power of 2
#define ACL_DOALIGN(x,b)   (((x) + (b) - 1) & ~((b) - 1))

//Bit operation macro
#define ACL_BIT_ENABLED(word, bit) (((word) & (bit)) != 0)
#define ACL_BIT_DISABLED(word, bit) (((word) & (bit)) == 0)
#define ACL_BIT_CMP_MASK(word, bit, mask) (((word) & (bit)) == mask)
#define ACL_SET_BITS(word, bits) (word |= (bits))
#define ACL_CLR_BITS(word, bits) (word &= ~(bits))

#endif//ACL_ACL_H
