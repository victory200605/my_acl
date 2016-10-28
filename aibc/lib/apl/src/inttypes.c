#include "apl/inttypes.h"
#include "apl/types.h"
#include <stdlib.h>

/* ---------------------------------------------------------------------- */
#define APL_REVERSE_BYTES(dest, src, bits) \
    do { \
        apl_size_t       lu_i; \
        for (lu_i = 0; lu_i < (bits)/8; ++lu_i) \
        { \
            ((apl_uint8_t*)&dest)[lu_i] = ((apl_uint8_t*)&src)[(bits)/8 - lu_i - 1]; \
        } \
    } while(0)

/* ---------------------------------------------------------------------- */

apl_int32_t apl_strtoi32(
    char const* apc_path, 
    char** appc_endptr, 
    apl_int_t ai_base)
{
    return strtol(apc_path, appc_endptr, (int)ai_base);
}

/* ---------------------------------------------------------------------- */

apl_uint32_t apl_strtou32(
    char const* apc_path, 
    char** appc_endptr, 
    apl_int_t ai_base)
{
    return strtoul(apc_path, appc_endptr, (int)ai_base);
}

/* ---------------------------------------------------------------------- */

apl_int64_t apl_strtoi64(
    char const* apc_path, 
    char** appc_endptr, 
    apl_int_t ai_base)
{
    return strtoll(apc_path, appc_endptr, (int)ai_base);
}

/* ---------------------------------------------------------------------- */

apl_uint64_t apl_strtou64(
    char const* apc_path, 
    char** appc_endptr, 
    apl_int_t ai_base)
{
    return strtoull(apc_path, appc_endptr, (int)ai_base);
}

/* ---------------------------------------------------------------------- */

apl_intmax_t apl_strtoimax(
    char const* apc_path, 
    char** appc_endptr, 
    apl_int_t ai_base)
{
#if HAVE_STRTOIMAX
    return strtoimax(apc_path, appc_endptr, (int)ai_base);
#else
    return strtoll(apc_path, appc_endptr, (int)ai_base);
#endif
}

/* ---------------------------------------------------------------------- */

apl_uintmax_t   apl_strtoumax(
    char const* apc_path, 
    char** appc_endptr, 
    apl_int_t ai_base)
{
#if HAVE_STRTOUMAX 
    return strtoumax(apc_path, appc_endptr, (int)ai_base);
#else
    return strtoull(apc_path, appc_endptr, (int)ai_base);
#endif
}

/* ---------------------------------------------------------------------- */

apl_uint16_t apl_hton16(apl_uint16_t au16_h)
{
#if defined(WORDS_BIGENDIAN)
    return au16_h;
#else
    apl_uint16_t    lu16_n = 0;

    APL_REVERSE_BYTES(lu16_n, au16_h, 16);

    return lu16_n;
#endif
}

/* ---------------------------------------------------------------------- */

apl_uint16_t apl_ntoh16(apl_uint16_t au16_n)
{
#if defined(WORDS_BIGENDIAN)
    return au16_n;
#else
    apl_uint16_t    lu16_h = 0;

    APL_REVERSE_BYTES(lu16_h, au16_n, 16);

    return lu16_h;
#endif
}

/* ---------------------------------------------------------------------- */

apl_uint32_t apl_hton32(apl_uint32_t au32_h)
{
#if defined(WORDS_BIGENDIAN)
    return au32_h;
#else
    apl_uint32_t    lu32_n = 0;

    APL_REVERSE_BYTES(lu32_n, au32_h, 32);

    return lu32_n;
#endif
}

/* ---------------------------------------------------------------------- */

apl_uint32_t apl_ntoh32(apl_uint32_t au32_n)
{
#if defined(WORDS_BIGENDIAN)
    return au32_n;
#else
    apl_uint32_t    lu32_h = 0;

    APL_REVERSE_BYTES(lu32_h, au32_n, 32);

    return lu32_h;
#endif
}

/* ---------------------------------------------------------------------- */

apl_uint64_t apl_hton64(apl_uint64_t au64_h)
{
#if defined(WORDS_BIGENDIAN)
    return au64_h;
#else
    apl_uint64_t    lu64_n = 0;

    APL_REVERSE_BYTES(lu64_n, au64_h, 64);

    return lu64_n;
#endif
}

/* ---------------------------------------------------------------------- */

apl_uint64_t apl_ntoh64(apl_uint64_t au64_n)
{
#if defined(WORDS_BIGENDIAN)
    return au64_n;
#else
    apl_uint64_t    lu64_h = 0;

    APL_REVERSE_BYTES(lu64_h, au64_n, 64);

    return lu64_h;
#endif
}

/* ---------------------------------------------------------------------- */

