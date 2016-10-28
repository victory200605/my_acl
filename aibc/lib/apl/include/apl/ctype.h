/**
 * @file ctype.h
 * @author  $Author: fzz $
 * @date    $Date: 2010/05/26 08:15:37 $
 * @version $Revision: 1.5 $
 */ 
#ifndef APL_CTYPES_H
#define APL_CTYPES_H

#include "aibc_config.h"
#include "apl/sys.h"
#include "apl/inttypes.h"
#include "apl/types.h"

/* ------------------------------ */ APL_DECLARE_BEGIN /* ------------------------------ */

/**
 * Test for an alphabetic character.
 *
 * @param[in]   au8_chr the charactor to check
 *
 * @retval      0       test failure
 * @retval      !=0     test success
 */
apl_int_t apl_isalpha(apl_uint8_t au8_chr);

/**
 * Test for an alphanumeric character.
 *
 * @param[in]   au8_chr the charactor to check
 *
 * @retval      0       test failure
 * @retval      !=0     test success
 */
apl_int_t apl_isalnum(apl_uint8_t au8_chr);

/**
 * Test for a 7-bit US-ASCII character.
 *
 * @param[in]   au8_chr the charactor to check
 *
 * @retval      0       test failure
 * @retval      !=0     test success
 */
apl_int_t apl_isascii(apl_uint8_t au8_chr);

/**
 * Test for a blank character.
 *
 * @param[in]   au8_chr the charactor to check
 *
 * @retval      0       test failure
 * @retval      !=0     test success
 */
apl_int_t apl_isblank(apl_uint8_t au8_chr);

/**
 * Test for a control character.
 *
 * @param[in]   au8_chr the charactor to check
 *
 * @retval      0       test failure
 * @retval      !=0     test success
 */
apl_int_t apl_iscntrl(apl_uint8_t au8_chr);

/**
 * Test for a decimal digit.
 *
 * @param[in]   au8_chr the charactor to check
 *
 * @retval      0       test failure
 * @retval      !=0     test success
 */
apl_int_t apl_isdigit(apl_uint8_t au8_chr);

/**
 * Test for a visible character.
 *
 * @param[in]   au8_chr the charactor to check
 *
 * @retval      0       test failure
 * @retval      !=0     test success
 */
apl_int_t apl_isgraph(apl_uint8_t au8_chr);

/**
 * Test for a a lowercase letter.
 *
 * @param[in]   au8_chr the charactor to check
 * @retval      0       test failure
 * @retval      !=0     test success
 */
apl_int_t apl_islower(apl_uint8_t au8_chr);

/**
 * Test for a printable character.
 *
 * @param[in]   au8_chr the charactor to check
 *
 * @retval      0       test failure
 * @retval      !=0     test success
 */
apl_int_t apl_isprint(apl_uint8_t au8_chr);

/**
 * Test for a punctuation character.
 *
 * @param[in]   au8_chr the charactor to check
 *
 * @retval      0       test failure
 * @retval      !=0     test success
 */
apl_int_t apl_ispunct(apl_uint8_t au8_chr);

/**
 * Test for a white-space character.
 *
 * @param[in]   au8_chr the charactor to check
 *
 * @retval      0       test failure
 * @retval      !=0     test success
 */
apl_int_t apl_isspace(apl_uint8_t au8_chr);

/**
 * Test for an uppercase letter.
 *
 * @param[in]   au8_chr the charactor to check
 *
 * @retval      0       test failure
 * @retval      !=0     test success
 */
apl_int_t apl_isupper(apl_uint8_t au8_chr);

/**
 * Test for a hexadecimal digit.
 *
 * @param[in]   au8_chr the charactor to check
 *
 * @retval      0       test failure
 * @retval      !=0     test success
 */
apl_int_t apl_isxdigit(apl_uint8_t au8_chr);

/**
 * Transliterate uppercase characters to lowercase 
 *
 * @param[in]   au8_chr the charactor to check
 *
 * @return the corresponding lowercase letter
 */
apl_uint8_t apl_tolower(apl_uint8_t au8_chr);

/**
 * Transliterate lowercase characters to uppercase
 *
 * @param[in]   au8_chr the charactor to check
 *
 * @return the corresponding uppercase letter
 */
apl_uint8_t apl_toupper(apl_uint8_t au8_chr);

/* ------------------------------ */ APL_DECLARE_END /* ------------------------------ */

#endif /* APL_STR_H */

