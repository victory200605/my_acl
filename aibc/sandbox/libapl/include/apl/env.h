/**
 *
 */
#ifndef APL__ENV_H
#define APL__ENV_H

#include "apl/common.h"

APL_BEGIN_C_DECLARE

/**
 *
 */
char const* apl_env_get(char const* p_name);

/**
 *
 */
int apl_env_set(char const* p_name, char const* p_value, int overwrite);

/**
 *
 */
int apl_env_unset(char const* p_name);


APL_END_C_DECLARE

#endif // APL__ENV_H

