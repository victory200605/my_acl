#include "aibc_config.h"
#include "apl/sys.h"
#include "apl/inttypes.h"
#include "apl/types.h"
#include "apl/regex.h"

/* ---------------------------------------------------------------------- */

apl_int_t apl_regcomp(
    apl_regex_t*    aps_reg, 
    char const*     apc_pattern,
    apl_int_t       ai_flags)
{
    return regcomp(aps_reg, apc_pattern, ai_flags);
}

/* ---------------------------------------------------------------------- */

apl_size_t apl_regerror(
    apl_int_t           ai_errcode, 
    apl_regex_t const*  aps_reg,
    char*               apc_errbuf, 
    apl_size_t          au_errbufsz)
{
    return regerror(ai_errcode, aps_reg, apc_errbuf, au_errbufsz);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_regexec(
    apl_regex_t const*  aps_reg, 
    char const *        apc_str,
    apl_size_t          au_nmatch, 
    apl_regmatch_t*     aps_pmatch, 
    apl_int_t           ai_flags)
{
    return regexec(aps_reg, apc_str, au_nmatch, aps_pmatch, ai_flags);
}

/* ---------------------------------------------------------------------- */

void apl_regfree(
    apl_regex_t*        aps_reg)
{
    return regfree(aps_reg);
}

/* ---------------------------------------------------------------------- */

