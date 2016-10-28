/**
 * @file proc.h
 * @author  $Author: hezk $
 * @date    $Date: 2009/08/18 09:31:11 $
 * @version $Revision: 1.5 $
 */
#ifndef APL_SIGNAL_H
#define APL_SIGNAL_H

#include "aibc_config.h"
#include "apl/sys.h"
#include "apl/inttypes.h"
#include "apl/proc.h"

/* ------------------------------ */ APL_DECLARE_BEGIN /* ------------------------------ */

/**
 * Process abort signal.
 */
#define APL_SIGABRT     (SIGABRT)

/**
 * Alarm clock.
 */
#define APL_SIGALRM     (SIGALRM)

/**
 * Access to an undefined portion of a memory object.
 */
#define APL_SIGBUS      (SIGBUS)

/**
 * Child process terminated, stoppe,or continued.
 */
#define APL_SIGCHLD     (SIGCHLD)

/**
 * Continue executing, if stopped.
 */
#define APL_SIGCONT     (SIGCONT)

/**
 * Erroneous arithmetic operation.
 */
#define APL_SIGFPE      (SIGFPE)

/**
 * Hangup.
 */
#define APL_SIGHUP      (SIGHUP)

/**
 * Illegal instruction.
 */
#define APL_SIGILL      (SIGILL)

/**
 * Terminal interrupt signal.
 */
#define APL_SIGINT      (SIGINT)

/**
 * Kill.
 */
#define APL_SIGKILL     (SIGKILL)

/**
 * Write on a pipe with no one to read it.
 */
#define APL_SIGPIPE     (SIGPIPE)

/**
 * Terminal quit signal.
 */
#define APL_SIGQUIT     (SIGQUIT)

/**
 * Invalid memory reference.
 */
#define APL_SIGSEGV     (SIGSEGV)

/**
 * Stop executing.
 */
#define APL_SIGSTOP     (SIGSTOP)

/**
 * Termination signal.
 */
#define APL_SIGTERM     (SIGTERM)

/**
 * Terminal stop signal.
 */
#define APL_SIGTSTP     (SIGTSTP)

/**
 * Background process attempting read.
 */
#define APL_SIGTTIN     (SIGTTIN)

/**
 * Background process attempting write.
 */
#define APL_SIGTTOU     (SIGTTOU)

/**
 * User-defined signal 1.
 */
#define APL_SIGUSR1     (SIGUSR1)

/**
 * User-defined signal 2.
 */
#define APL_SIGUSR2     (SIGUSR2)

/**
 * Pollable event.
 */
#define APL_SIGPOLL     (SIGPOLL)

/**
 * Profiling timer expired.
 */
#define APL_SIGPROF     (SIGPROF)

/**
 * Bad system call.
 */
#define APL_SIGSYS      (SIGSYS)

/**
 * Trace/breakpoint trap.
 */
#define APL_SIGTRAP     (SIGTRAP)

/**
 * High bandwidth data is available at a socket.
 */
#define APL_SIGURG      (SIGURG)

/**
 * Virtual timer expired.
 */
#define APL_SIGVTALRM   (SIGVTALRM)

/**
 * CPU time limit exceeded.
 */
#define APL_SIGXCPU     (SIGXCPU)

/**
 * File size limit exceeded.
 */
#define APL_SIGXFSZ     (SIGXFSZ)

/**
 * Request for default signal handling.
 */
#define APL_SIG_DFL     (SIG_DFL)

/**
 * Request that signal be ignored.
 */
#define APL_SIG_IGN     (SIG_IGN)

/**
 * Return value from apl_signal() in case of error.
 */
#define APL_SIG_ERR     (SIG_ERR)


typedef void (*apl_sig_func_ptr)(int);



/** 
 * Signal management.
 *
 * When a signal which equal to ai_sig, the function pointed by apf_func will 
 * execute.
 *
 * @param[in]   ai_sig      the signal number which need handle.
 * @param[in]   apf_func    a pointer points to the function which process the 
 *                          signal.
 * @retval apf_func     The request can be honored.
 * @retval APL_SIG_ERR  failure, and a positive value shall be stored in errno.
 */
apl_sig_func_ptr apl_signal(
    apl_int_t           ai_sig,
    apl_sig_func_ptr    apf_func);


/** 
 * Send a signal to a process or a group of processes.
 *
 * This function shall send a signal to a process or a group of processes 
 * specified by pid.
 * 
 * @param[in]   ai_pid  The pid of a process
 *                      - >0 \n
 *                        ai_sig shall be sent to the process whose process 
 *                        ID is equal to pid. 
 *                      - ==0 \n
 *                        sig shall be sent to all processes (excluding an 
 *                        unspecified set of system processes) whose process 
 *                        group ID is equal to the process group ID of the 
 *                        sender, and for which the process has permission to 
 *                        send a signal.
 *                      - <0 \n
 *                        sig shall be sent to all processes (excluding an 
 *                        unspecified set of system processes) for which the 
 *                        process has permission to send that signal.
 * @param[in]   ai_sig  The signal to send.
 *                      - !=0 \n
 *                        the signal number which need to send.
 *                      - == 0 \n
 *                        error checking is performed but no signal is 
 *                        actually sent.
 * @retval 0    success. 
 * @retval -1   failure, and errno set to indicate the error.
 */
apl_int_t apl_kill(
    apl_pid_t           ai_pid,
    apl_int_t           ai_sig);

/* ------------------------------ */ APL_DECLARE_END /* ------------------------------ */

#endif /* APL_SIGNAL_H */

