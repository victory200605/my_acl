#include "apl/mem.h"
#include <string.h>

/* ---------------------------------------------------------------------- */

void* apl_memcpy(
    void*       apv_dest,
    void const* apv_src,
    apl_size_t  au_nbyte)
{
    return (void*) memcpy(apv_dest, apv_src, (size_t)au_nbyte);
}

/* ---------------------------------------------------------------------- */

void* apl_memmove(
    void*       apv_dest,
    void const* apv_src,
    apl_size_t  au_nbyte) 
{
    return (void*) memmove(apv_dest, apv_src, (size_t)au_nbyte);
}

/* ---------------------------------------------------------------------- */

void* apl_memccpy(
    void*       apv_dest,
    void const* apv_src,
    apl_uint8_t au8_chr,
    apl_size_t  au_nbyte)
{
    return (void*) memccpy(apv_dest, apv_src, (int)au8_chr, (size_t)au_nbyte);
}

/* ---------------------------------------------------------------------- */

void apl_memset(
    void*       apv_ptr,
    apl_uint8_t au8_chr,
    apl_size_t  au_nbyte)
{
    memset(apv_ptr, (int)au8_chr, (size_t)au_nbyte);
}

/* ---------------------------------------------------------------------- */

void* apl_memchr(
    void const* apv_ptr,
    apl_uint8_t au8_chr,
    apl_size_t  au_nbyte)
{
    return (void*) memchr(apv_ptr, (int)au8_chr, (size_t)au_nbyte);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_memcmp(
    void const* apv_m1,
    void const* apv_m2,
    apl_size_t  au_nbyte)
{
    return (apl_int_t) memcmp(apv_m1, apv_m2, (size_t)au_nbyte);
}

/* ---------------------------------------------------------------------- */
