/**
 * @file proc.h
 * @author  $Author: hezk $
 * @date    $Date: 2009/08/18 09:31:11 $
 * @version $Revision: 1.11 $
 */
#ifndef APL_PROC_H
#define APL_PROC_H

#include "aibc_config.h"
#include "apl/sys.h"
#include "apl/inttypes.h"
#include "apl/types.h"
#include <unistd.h>


/* ------------------------------ */ APL_DECLARE_BEGIN /* ------------------------------ */

/** designates a type capable of representing any value of process ID */
typedef apl_int_t       apl_pid_t;


/**
 * Do not hang if no status is available
 */
#define APL_WNOHANG     (WNOHANG)

/**
 * Report status of stopped child process
 */
#define APL_WUNTRACED   (WUNTRACE)


/**
 * Get the process exit status according to the stat argument.
 *
 * @param[in]   stat    the process status value.
 *
 * @return  exit status of the process.
 */
#define APL_WEXITSTATUS(stat)       WEXITSTATUS(stat)

/**
 * Check the process status according to the stat argument.
 *
 * @param[in]    stat    the process status value.
 *
 * @retval !=0  it specifies child has been continued.
 * @retval 0    false
 */
#define APL_WIFCONTINUED(stat)      WIFCONTINUED(stat)



/**
 * Check the process status according to the stat argument.
 *
 * @param[in]    stat    the process status value.
 *
 * @retval !=0  it specifies child exited normally.
 * @retval 0    false
 */
#define APL_WIFEXITED(stat)         WIFEXITED(stat)

/**
 * Check the process status according to the stat argument.
 *
 * @param[in]    stat    the process status value.
 *
 * @retval !=0  it specifies child exited due to uncaught signal.
 * @retval 0    false.
 */
#define APL_WIFSIGNALED(stat)       WIFSIGNALED(stat)

/**
 * Check the process status according to the stat argument.
 *
 * @param[in]    stat    the process status value.
 *
 * @retval !=0  it specifies child is currently stopped.
 * @retval 0    false.
 */
#define APL_WIFSTOPPED(stat)        WIFSTOPPED(stat)

/**
 * Get the process exit status according to the stat argument.
 *
 * @param[in]    stat    the process status value.
 *
 * @return    signal number that caused process to stop.
 */
#define APL_WSTOPSIG(stat)          WSTOPSIG(stat)

/**
 * Get the process exit status according to the stat argument.
 *
 * @param[in]    stat    the process status value.
 *
 * @return    signal number that caused process to terminate.
 */
#define APL_WTERMSIG(stat)          WTERMSIG(stat)

/** 
 * Get the process ID.
 *
 * This function shall return the process ID of the calling process.
 *
 * @return    the value of processID.
 */
apl_pid_t apl_getpid(void);


/** 
 * Get the parent process ID.
 *
 * This function shall return the parent process ID of the calling process.
 *
 * @return    the value of processID.
 */
apl_pid_t apl_getppid(void);


/** 
 * Wait for a child process to stop or terminate.
 *
 * This function shall obtain status information pertaining to one of the 
 * caller's child processes.
 *
 * Various options permit status information to be obtained for child 
 * processes that have terminated or stopped.
 *
 * This function shall suspend execution of the calling thread until status 
 * information for one of the terminated child processes of the calling 
 * process is available, or until delivery of a signal whose action is either 
 * to execute a signal-catching function or to terminate the process.
 *
 * @param[out]  api_stat    a pointer point to the location that used to 
 *                          store the child process status information.
 *
 * @retval -1   fail, errno indicate the error.
 * @retval !=-1 successful, the return value is equal to the process ID of 
 *              the child process for which status is reported.
 */
apl_pid_t apl_wait(
    apl_int_t*      api_stat);

/** 
 * Wait for a child process to stop or terminate.
 *
 * This function shall obtain status information pertaining to one of the 
 * caller's child processes.
 *
 * This function's behavior shall be modified by the values of the ai_pid and 
 * au_flags arguments.
 *
 * @param[in]   ai_pid  this argument specifies a set of child processes for 
 *                      which status is requested.
 *                      - -1 \n
 *                        status is requested for any child process.
 *                      - >0 \n
 *                        it specifies the process ID of a single child 
 *                        process for which status is requested.
 *                      - 0 \n
 *                        status is requested for any child process whose 
 *                        process group ID is equal to that of the calling 
 *                        process.
 *                      - <-1 \n
 *                        status is requested for any child process whose 
 *                        process group ID is equal to the absolute value of 
 *                        ai_pid.
 * @param[out]  api_stat    a pointer point to the location that used to 
 *                          store the child process status information.
 * @param[in]   au_flags    This argument is constructed from the 
 *                          bitwise-inclusive OR of zero or more of the 
 *                          following flags:
 *                          - APL_WNOHANG \n
 *                            The function shall not suspend execution of the 
 *                            calling thread if status is not immediately 
 *                            available for one of the child processes 
 *                            specified by ai_pid.
 *                          - APL_WUNTRACED \n
 *                            The status of any child processes specified by 
 *                            ai_pid that are stopped, and whose status has 
 *                            not yet been reported since they stopped, shall 
 *                            also be reported to the requesting process.
 *
 * @retval -1   fail, errno indicate the error.
 * @retval 0    status is not available for any process specified by ai_pid.
 * @retval >0   successful, the return value is equal to the process ID of 
 *              the child process for which status is reported.
 */
apl_pid_t apl_waitpid(
    apl_pid_t       ai_pid,
    apl_int_t*      api_stat,
    apl_uint_t      au_flags);

/** 
 * Get the pathname of the current working directory.
 *
 * This function shall place an absolute pathname of the current working 
 * directory in the array pointed to by apc_buf.
 *
 * @param[out]  apc_buf     a pointer points to the location which used to 
 *                          store the pathname.
 * @param[in]   au_buflen   length of the array apc_buf.
 *
 * @retval 0    successful.
 * @retval -1   fail, errno indicate the error.
 */
apl_int_t apl_getcwd(
    char*       apc_buf,
    apl_size_t  au_buflen);

/** 
 * Change working directory.
 *
 * This function shall cause the directory named by the pathname pointed to 
 * by the apc_dir argument to become the current working directory.
 *
 * @param[in]    apc_dir    a pointer points to pathname string.
 *
 * @retval 0    successful.
 * @retval -1   fail, errno indicate the error.
 */
apl_int_t apl_chdir(
    char const*     apc_dir);

/** 
 * Create a new process.
 *
 * This function shall create a new process.
 *
 * @return
 *      - If successful, return 0 to child process and the process ID of the 
 *        child process to the parent process.
 *      - If fail, return -1 to parent process and child process is not 
 *        created, errno indicate the error.
 */
apl_pid_t apl_fork(void);
    
/** 
 * Execute a file.
 *
 * This functions shall replace the current process image with a new process 
 * image.
 *
 * The new image shall be constructed from a regular, executable file called 
 * the new process image file. There shall be no return from a successful 
 * apl_exec(), because the calling process image is overlaid by the new process 
 * image.
 *
 * @param[in]   apc_path    a pointer points to a pathname that identifies 
 *                          the new process image file.
 * @param[in]   appc_argv   the entry arguments of new program.
 * @param[in]   appc_env    an array of character pointers to null-terminated 
 *                          strings that constitute the environment for the 
 *                          new process image.
 * @retval -1   fail, errno indicate the error.
 * @retval 0    success.
 */
apl_pid_t apl_exec(
    char const*     apc_path,
    char * const*   appc_argv,
    char * const*   appc_env);

/**
 * Spawn a process.
 *
 * This functions shall create a new process (child process) from the 
 * specified process image. The new process image shall be constructed from a 
 * regular executable file called the new process image file.
 *
 * @param[out]  api_pid     pid of the new process
 * @param[in]   apc_path    a pathname that identifies the new process image 
 *                          file to execute.
 * @param[in]   appc_argv   an array of character pointers to null-terminated 
 *                          strings. The last member of this array shall be a 
 *                          null pointer.
 * @param[in]   appc_env    an array of character pointers to null-terminated 
 *                          strings. These strings constitute the environment 
 *                          for the new process image. The environment array 
 *                          is terminated by a null pointer.
 *
 * @retval -1   fail, errno indicate the error.
 * @retval 0    success.
 */
apl_int_t apl_spawn(
    apl_pid_t*          api_pid,
    char const*         apc_path,
    char * const*       appc_argv,
    char * const*       appc_env);

/** 
 * Terminate current process.
 *
 * This function shall terminate a process.
 *
 * @param[in]   ai_retcode  indicate the process terminate status, only the 
 *                          least significant 8 bits shall be available.
 */
void apl_exit(
    apl_uint8_t     ai_retcode);

/** 
 * Generate an abnormal process abort.
 *
 * This function shall cause abnormal process termination to occur.
 */
void apl_abort(void);


/* ------------------------------ */ APL_DECLARE_END /* ------------------------------ */

#endif /* APL_PROC_H */

