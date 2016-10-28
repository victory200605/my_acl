#include "apl/signal.h"
#include <signal.h>

/* ---------------------------------------------------------------------- */

apl_sig_func_ptr apl_signal(
    apl_int_t           ai_sig,
    apl_sig_func_ptr    apf_func)
{
    return (apl_sig_func_ptr)signal((int)ai_sig, apf_func);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_kill(
    apl_pid_t       ai_pid,
    apl_int_t       ai_sig)
{
    return (apl_int_t)kill(ai_pid, ai_sig);
}

/* ---------------------------------------------------------------------- */

