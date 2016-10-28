#include "apl/heap.h"
#include <stdlib.h>

/* ---------------------------------------------------------------------- */

void* apl_malloc(
    apl_size_t au_size)
{
    return (void*) malloc(au_size);
}

/* ---------------------------------------------------------------------- */

void* apl_realloc(
    void*        apv_ptr, 
    apl_size_t  au_size) 
{
    return (void*) realloc(apv_ptr, au_size);
}

/* ---------------------------------------------------------------------- */

void apl_free(void* apv_ptr)
{
    (void) free(apv_ptr);
}

/* ---------------------------------------------------------------------- */

