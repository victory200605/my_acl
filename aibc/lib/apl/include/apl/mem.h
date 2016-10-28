/**
 * @file mem.h
 * @author  $Author: hezk $
 * @date    $Date: 2009/08/18 09:31:11 $
 * @version $Revision: 1.6 $
 */
#ifndef APL_MEM_H
#define APL_MEM_H

#include "aibc_config.h"
#include "apl/sys.h"
#include "apl/inttypes.h"
#include "apl/types.h"


/* ------------------------------ */ APL_DECLARE_BEGIN /* ------------------------------ */

/** 
 * Copy bytes in memory.
 *
 * This function shall copy au_nbyte bytes from the object pointed to by 
 * apv_src into the object pointed to by apv_dest.
 *
 * If copying takes place between objects that overlap, the behavior is undefined.
 *
 * @param[out]  apv_dest    the destination pointer.
 * @param[in]   apv_src     the source pointer.
 * @param[in]   au_nbyte    the bytes copy from source data to destination.
 *
 * @return    it shall return apv_dest.
 */

void* apl_memcpy(
    void*       apv_dest,
    void const* apv_src,
    apl_size_t  au_nbyte);

/** 
 * Copy bytes in memory with overlapping areas.
 *
 * This function shall copy au_nbyte bytes from the object pointed to by 
 * apv_src into the object pointed to by apv_dest.
 *
 * It is equivalent to the apl_memcpy() function and it resolve the overlap 
 * problem.
 *
 * @param[out]  apv_dest    the destination pointer.
 * @param[in]   apv_src     the source pointer.
 * @param[in]   au_nbyte    the bytes copy from source data to destination,
 *
 * @return    it shall return apv_dest.
 */
void* apl_memmove(
    void*       apv_dest,
    void const* apv_src,
    apl_size_t  au_nbyte);

/** 
 * Copy bytes in memory.
 *
 * This function shall copy au_nbyte from memory area apv_src into apv_dest, 
 * stopping after the first occurrence of byte au8_chr is copied, or after 
 * au_nbyte bytes are copied, whichever comes first. If copying takes place 
 * between objects that overlap, the behavior is undefined.
 *
 * @param[out]  apv_dest    the destination pointer.
 * @param[in]   apv_src     the source pointer.
 * @param[in]   au8_chr     the charactor indicate stopping copy.
 * @param[in]   au_nbyte    max length of copy.
 *
 * @retval APL_NULL     Canot finnd au8_chr in apv_src.
 * @retval !=APL_NULL   A pointer to the byte after the copy of c in s1.
 */
void* apl_memccpy(
    void*       apv_dest,
    void const* apv_src,
    apl_uint8_t au8_chr,
    apl_size_t  au_nbyte);

/** 
 * Set bytes in memory.
 *
 * This function shall copy au8_chr into each of the first au_nbyte bytes of 
 * the object pointed to by apv_ptr.
 *
 * @param[out]  apv_ptr     the destination pointer.
 * @param[in]   au8_chr     the charactor used to copy to the object pointed 
 *                          to by apv_ptr.
 * @param[in]   au_nbyte    indicate numbers of bytes to copy.
 */
void apl_memset(
    void*       apv_ptr,
    apl_uint8_t au8_chr,
    apl_size_t  au_nbyte);

/** 
 * Find byte in memory.
 *
 * This function shall locate the first occurrence of au8_chr in the initial 
 * au_nbyte bytes of the object pointed to by apv_ptr.
 *
 * @param[in]   apv_ptr     a pointer points to input string.
 * @param[in]   au8_chr     the charactor need to search.
 * @param[in]   au_nbyte    indicate the length of apv_ptr to search.
 *
 * @retval APL_NULL     Cannot find au8_chr in apv_ptr.
 * @retval !=APL_NULL   A pointer to the located byte equivalent au8_chr in 
 *                      apv_ptr.
 */
void* apl_memchr(
    void const* apv_ptr,
    apl_uint8_t au8_chr,
    apl_size_t  au_nbyte);

/** 
 *
 * This function shall compare the first au_nbyte bytes of the object pointed 
 * to by apv_m1 to the first au_nbyte bytes of the object pointed to by 
 * au_byte.
 *
 * @param[in]   apv_m1      the string need to compare.
 * @param[in]   apv_m2      the string need to compare.
 * @param[in]   au_nbyte    the bytes of apv_m1 and apv_m2 used to compare.
 *
 * @retval >0   the compare result is apv_m1 > apv_m2.
 * @retval ==0  the compare result is apv_m1 == apv_m2.
 * @retval <0   the compare result is apv_m1 < apv_m2.
 */
apl_int_t apl_memcmp(
    void const* apv_m1,
    void const* apv_m2,
    apl_size_t  au_nbyte);

/* ------------------------------ */ APL_DECLARE_END /* ------------------------------ */

#endif /* APL_MEM_H */

