#include "apl/proc.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#if defined(HAVE_SPAWN_H)
#   include <spawn.h>
#endif

/* ---------------------------------------------------------------------- */

apl_pid_t apl_getpid(void)
{
    return (apl_pid_t) getpid();
}

/* ---------------------------------------------------------------------- */

apl_pid_t apl_getppid(void)
{
    return (apl_pid_t) getppid();
}

/* ---------------------------------------------------------------------- */

apl_pid_t apl_wait( 
    apl_int_t*      api_stat)
{
    int li_stat;
    apl_pid_t li_ret = (apl_pid_t) wait(&li_stat);

    if (li_ret >= 0 && api_stat != NULL)
    {
        *api_stat = (apl_int_t)li_stat;
    }

    return li_ret;
};

/* ---------------------------------------------------------------------- */

apl_pid_t apl_waitpid(
    apl_pid_t       ai_pid,
    apl_int_t*      api_stat,
    apl_uint_t      au_flags)
{
    int li_stat;
    apl_pid_t li_ret = (apl_pid_t) waitpid((pid_t)ai_pid, &li_stat, (int)au_flags);

    if (li_ret >= 0 && api_stat != NULL)
    {
        *api_stat = (apl_int_t)li_stat;
    }

    return li_ret;
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_getcwd(
    char*       apc_buf,
    apl_size_t  au_buflen)
{
    return APL_NULL == getcwd(apc_buf, (size_t)au_buflen) ? APL_INT_C(-1) : 0;
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_chdir(
    char const*     apc_dir)
{
    return (apl_int_t) chdir(apc_dir);
}

/* ---------------------------------------------------------------------- */

apl_pid_t apl_fork(void)
{
    return (apl_pid_t) fork();
}

/* ---------------------------------------------------------------------- */

apl_pid_t apl_exec(
    char const*     apc_path,
    char* const*    appc_argv,
    char* const*    appc_env)
{
    return (apl_pid_t) execve(apc_path, appc_argv, appc_env);
}

/* ---------------------------------------------------------------------- */

apl_int_t apl_spawn(
    apl_pid_t*          api_pid,
    char const*         apc_path,
    char * const*       appc_argv,
    char * const*       appc_env)
{
    pid_t   li_pid = 0;

#if defined(HAVE_POSIX_SPAWN)
    apl_int_t li_ret = (apl_int_t) posix_spawn(
        &li_pid, 
        apc_path, 
        APL_NULL, 
        APL_NULL, 
        appc_argv, 
        appc_env);

    if (li_ret < 0)
    {
        return li_ret;
    }
#else
    li_pid = fork();

    if (0 == li_pid)
    { /* child */
        char* lapc_argv[] = { NULL };
        char* lapc_env[] = { NULL };

        apl_int_t li_ret = execve(
            apc_path, 
            appc_argv ?  appc_argv : lapc_argv, 
            appc_env ? appc_env : lapc_env);

        if (li_ret < 0)
        {
            apl_perror("execve");
            exit(0);
        }
        /* TODO: how to return error? */
    }
#endif

    if (li_pid > 0 && api_pid != NULL)
    {
        *api_pid = (apl_pid_t) li_pid;
        return 0;
    }

    return APL_INT_C(-1);
}

/* ---------------------------------------------------------------------- */

void apl_exit(
    apl_uint8_t     ai_returncode) 
{
    exit((int)ai_returncode);
}

/* ---------------------------------------------------------------------- */

void apl_abort(void)
{
    abort();
}

/* ---------------------------------------------------------------------- */

