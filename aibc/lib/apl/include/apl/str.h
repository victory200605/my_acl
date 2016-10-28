/**
 * @file proc.h
 * @author  $Author: hezk $
 * @date    $Date: 2009/08/18 09:31:11 $
 * @version $Revision: 1.9 $
 */
#ifndef APL_STR_H
#define APL_STR_H

#include "aibc_config.h"
#include "apl/sys.h"
#include "apl/inttypes.h"
#include "apl/types.h"
#include <stdarg.h>

/* ------------------------------ */ APL_DECLARE_BEGIN /* ------------------------------ */

/**
 * Get string length.
 *
 * @param[in]   apc_str the input string
 *
 * @return Return the length of apc_str.
 */ 
apl_size_t apl_strlen(
    char const* apc_str);

/**
 * Copy part of a string.
 *
 * @param[out]  apc_dest    the output buffer
 * @param[in]   apc_src     the original string
 * @param[in]   au_destlen  the max size of output buffer
 *
 * @return Return apc_dest.
 */ 
char* apl_strncpy(
    char*       apc_dest,
    char const* apc_src,
    apl_size_t  au_destlen);


/**
 * Concatenate a string with part of another, this function shall ensure the 
 * length result string shall be not bigger than au_destlen.
 *
 * @param[out]  apc_dest    the string to be concatenated
 * @param[in]   apc_src     the concatenating string
 * @param[in]   au_destlen  specifies the length of the buffer pointed by 
 *                          apc_dest.
 *
 * @return Return apc_dest.
 */ 
char* apl_strncat(
    char*       apc_dest,
    char const* apc_src,
    apl_size_t  au_destlen);


/**
 * String scanning operation.
 *
 * @param[in]   apc_str the original string.
 * @param[in]   au8_chr the character to be located in the first occurrence.
 *
 * @retval !=APL_NULL   found, return a pointer to the byte.
 * @retval APL_NULL     not found.
 */ 
char* apl_strchr(
    char const* apc_str,
    apl_uint8_t au8_chr);


/**
 * String scanning operation.
 * 
 * @param[in]   apc_str the original string.
 * @param[in]   au8_chr the character to be located in the last occurrence.
 *
 * @retval !=APL_NULL   found, return a pointer to the byte.
 * @retval APL_NULL     not found.
 */ 
char* apl_strrchr(
    char const* apc_str,
    apl_uint8_t au8_chr);


/**
 * Find the substring in original one at the first occurrence.
 *
 * @param[in]    apc_str    the original string
 * @param[in]    apc_substr the sub string
 *
 * @retval !=APL_NULL   found, return a pointer to the located string, or 
 *                      apc_substr points to a string with zero length, 
 *                      the function shall return apc_str.
 * @retval APL_NULL     not found, 
 */ 
char* apl_strstr(
    char const* apc_str,
    char const* apc_substr);


/**
 * Compare two strings.
 * 
 * @param[in]    apc_s1   one string
 * @param[in]    apc_s2   the other string
 *
 * @retval >0   apc_s1 > apc_s2
 * @retval 0    apc_s1 == apc_s2
 * @retval <0   apc_s1 < apc_s2
 */ 
apl_int_t apl_strcmp(
    char const* apc_s1,
    char const* apc_s2);


/**
 * Compare part of two strings.
 *
 * @param[in]   apc_s1   one string
 * @param[in]   apc_s2   the other string
 * @param[in]   au_len   the max size to be compared
 *
 * @retval >0   apc_s1 > apc_s2
 * @retval 0    apc_s1 == apc_s2
 * @retval <0   apc_s1 < apc_s2
 */ 
apl_int_t apl_strncmp(
    char const* apc_s1,
    char const* apc_s2,
    apl_size_t  au_len);


/**
 * Case-insensitive string comparisons.
 *
 * @param[in]    apc_s1   one string
 * @param[in]    apc_s2   the other string
 *
 * @retval >0   apc_s1 > apc_s2
 * @retval 0    apc_s1 == apc_s2
 * @retval <0   apc_s1 < apc_s2
 */ 
apl_int_t apl_strcasecmp(
    char const* apc_s1,
    char const* apc_s2);


/**
 * Compare part of two case-insensitive string.
 *
 * @param[in]    apc_s1   -   one string
 * @param[in]    apc_s2   -   the other string
 * @param[in]    au_len   -   the max size to be compared
 *
 * @retval >0   apc_s1 > apc_s2
 * @retval 0    apc_s1 == apc_s2
 * @retval <0   apc_s1 < apc_s2
 */
apl_int_t apl_strncasecmp(
    char const* apc_s1,
    char const* apc_s2,
    apl_size_t  au_len);


/**
 * Print formatted output.
 *
 * @param[out]    apc_buf   the output buffer
 * @param[in]     au_buflen the buffer size
 * @param[in]     apc_fmt   the buffer format
 * @param[in]     ...       variable argument
 *
 * @retval >=0  The number of bytes that would be written to apc_buf had 
 *              au_buflen been sufficiently large excluding the terminating 
 *              null byte.
 * @retval <0   apc_buf NULL but au_buflen is not zero.
 */ 
apl_int_t apl_snprintf(
    char *      apc_buf,
    apl_size_t  au_buflen,
    char const* apc_fmt,
    ...);


/**
 * Format output of a stdarg argument list.
 *
 * @param[out]  apc_buf     the output buffer
 * @param[in]   au_buflen   the buffer size
 * @param[in]   apc_fmt     the buffer format
 * @param[in]   ap_valist   variable argument
 *
 * @retval >=0  The number of bytes that would be written to apc_buf had 
 *              au_buflen been sufficiently large excluding the terminating 
 *              null byte.
 * @retval <0   apc_buf NULL but au_buflen is not zero.
*/  
apl_int_t apl_vsnprintf(
    char *      apc_buf,
    apl_size_t  au_buflen,
    char const* apc_fmt,
    va_list     ap_valist);


/**
 * Duplicate a string.
 *
 * @param[in]   apc_str the original string to be duplicated
 *
 * @retval !=APL_NULL   success, return a pointer to a new string, which is a 
 *                      duplicate of the string pointed to by apc_str
 * @retval APL_NULL     failed, and set errno to indicate the error
 */ 
char* apl_strdup(
    char const* apc_str);


/* ------------------------------ */ APL_DECLARE_END /* ------------------------------ */

#endif /* APL_STR_H */

