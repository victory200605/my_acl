/**
 * @file regex.h
 * @author  $Author: hezk $
 * @date    $Date: 2009/08/18 09:31:11 $
 * @version $Revision: 1.5 $
 */
#ifndef APL_REGEX_H
#define APL_REGEX_H

#include "aibc_config.h"
#include "apl/sys.h"
#include "apl/inttypes.h"
#include "apl/types.h"
#include <regex.h>

/* ------------------------------ */ APL_DECLARE_BEGIN /* ------------------------------ */

#define apl_regex_t     regex_t
#define apl_regmatch_t  regmatch_t

/**
 * Use Extended Regular Expressions.
 */
#define APL_REG_EXTENDED (REG_EXTENDED)

/**
 * Ignore case in match.
 */
#define APL_REG_ICASE    (REG_ICASE)

/**
 * Report only success or fail in apl_regexec().
 */
#define APL_REG_NOSUB    (REG_NOSUB)

/**
 * Change the handling of \<newline\>.
 */
#define APL_REG_NEWLINE  (REG_NEWLINE)

/**
 * The circumflex character ( '^' ), when taken as a special character, does 
 * not match the beginning of string.
 */
#define APL_REG_NOTBOL   (REG_NOTBOL)

/**
 * The dollar sign ( '\$' ), when taken as a special character, does not 
 * match the end of string.
 */
#define APL_REG_NOTEOL   (REG_NOTEOL)

/**
 * regexec() failed to match.
 */
#define APL_REG_NOMATCH  (REG_NOMATCH)

/**
 * Invalid regular expression.
 */
#define APL_REG_BADPAT   (REG_BADPAT)

/**
 * Invalid collating element referenced.
 */
#define APL_REG_ECOLLATE (REG_ECOLLATE)

/**
 * Invalid character class type referenced.
 */
#define APL_REG_ECTYPE   (REG_ECTYPE)

/**
 * Trailing '\\' in pattern.
 */
#define APL_REG_EESCAPE  (REG_EESCAPE)

/**
 * Number in \\digit invalid or in error.
 */
#define APL_REG_ESUBREG  (REG_ESUBREG)

/**
 * "\[\]" imbalance.
 */
#define APL_REG_EBRACK   (REG_EBRACK)

/**
 * "\\(\\)" or "()" imbalance.
 */
#define APL_REG_EPAREN   (REG_EPAREN)

/**
 * "\\{\\}" imbalance.
 */
#define APL_REG_EBRACE   (REG_EBRACE)

/**
 * Content of "\\{\\}" invalid: not a number, number too large, more than two 
 * numbers, first larger than second.
 */
#define APL_REG_BADBR    (REG_BADBR)

/**
 * Invalid endpoint in range expression.
 */
#define APL_REG_ERANGE   (REG_ERANGE)

/**
 * Out of memory.
 */
#define APL_REG_ESPACE   (REG_ESPACE)

/**
 * '?', '*', or '+' not preceded by valid regular expression.
 */
#define APL_REG_BADRPT   (REG_BADRPT)


/**
 * Compile the regular expression.
 *
 * This function shall compile the regular expression contained in the string 
 * pointed to by the apc_pattern argument and place the results in the 
 * structure pointed to by aps_reg. The ai_flags argument is the 
 * bitwise-inclusive OR of zero or more of the following flags:
 *
 * - REG_EXTENDED \n
 *   Use Extended Regular Expressions.
 * - REG_ICASE \n
 *   Ignore case in match. (See the Base Definitions volume of IEEE Std 
 *   1003.1-2001, Chapter 9, Regular Expressions.)
 * - REG_NOSUB \n
 *   Report only success/fail in regexec().
 * - REG_NEWLINE
 *   Change the handling of \<newline\>s, as described in the text.
 *
 * @param[out]  aps_reg     The compiled regular pattern.
 * @param[in]   apc_pattern The literal regular pattern.
 * @param[in]   ai_flags    The compiling flags.
 */
apl_int_t apl_regcomp(
    apl_regex_t*    aps_reg, 
    char const*     apc_pattern,
    apl_int_t       ai_flags);

/**
 * Get regex error message string.
 *
 * The regerror() function provides a mapping from error codes returned by 
 * apl_regcomp() and apl_regexec() to unspecified printable strings. 
 *
 * @param[in]   ai_errcode  Error code of regex operation.
 * @param[in]   aps_reg     The corresponding apl_regex_t.
 * @param[out]  apc_errbuf  The error message.
 * @param[in]   au_errbufsz The length of error message output buffer.
 *
 * @return  Upon successful completion, the apl_regerror() function shall 
 *          return the number of bytes needed to hold the entire generated 
 *          string, including the null termination. If the return value is 
 *          greater than au_errbufsz, the string returned in the buffer 
 *          pointed to by errbuf has been truncated.
 */
apl_size_t apl_regerror(
    apl_int_t           ai_errcode, 
    apl_regex_t const*  aps_reg,
    char*               apc_errbuf, 
    apl_size_t          au_errbufsz);

/**
 * Execute a pattern matching.
 *
 * The apl_regexec() function must fill in all au_nmatch elements of 
 * aps_pmatch, where au_nmatch and aps_pmatch are supplied by the 
 * application, even if some elements of aps_pmatch do not correspond to 
 * subexpressions in pattern. The application writer should note that there 
 * is probably no reason for using a value of au_nmatch that is larger than 
 * aps_reg->re_nsub + 1.
 *
 * @param[in]   aps_reg 
 * @param[in]   apc_str
 * @param[in]   au_nmatch
 * @param[out]  aps_pmatch
 * @param[in]   ai_flags
 *
 * @retval 0                success.
 * @retval APL_REG_NOMATCH  indicate that no match.
 */
apl_int_t apl_regexec(
    apl_regex_t const*  aps_reg, 
    char const *        apc_str,
    apl_size_t          au_nmatch, 
    apl_regmatch_t*     aps_pmatch, 
    apl_int_t           ai_flags);

/**
 * Frees any memory allocated by apl_regcomp()
 * 
 * @param[in]   aps_reg The structure pointer returned by apl_regcomp()
 */
void apl_regfree(
    apl_regex_t*        aps_reg);

/* ------------------------------ */  APL_DECLARE_END  /* ------------------------------ */

#endif /* APL_REGEX_H */

