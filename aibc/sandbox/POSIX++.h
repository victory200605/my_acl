#ifndef __AILIBEX__POSIX++_H__
#define __AILIBEX__POSIX++_H__

// io
class CBaseFile
{
public:
    ssize_t write(void const*, size_t);
    ssize_t read(...);
    int close();
    int fileno();
    int fcntl();

    int setblocking(...);

protected:
    CBaseFile()
    int miFileDesc;
};


class CFile
    :public CBaseFile
{
public:
    CFile()
    {
    }
    int open(...);
    off_t seek(...); 
    ssize_t pwrite(...);
    ssize_t pread(...);
};

class CSocket
    :public CBaseFile
{
public:
    int socket(...);

    int shutdown(...);
    int atmark();
    int getpeername();
    int getsockname();
    int setopt(...);
    int getopt(...);

    int connect(...);
    int accept(...);
    int bind(...);
    int listen(...);
};

// time
class CTime
{
private:
    time_t  mtTime;
};

class CTimeValue
{
private:
    struct timeval mtTimeValue
};

class CTimeSpec
{
private:
    struct timespec mtTimeSpec
};

class CDateTime
{
private:
    struct tm   mtDateTime;
};

// thread
class CMutexAttr
{
public:
    CMutexAttr()
    {
        pthread_mutexattr_init(&mtMutexAttr);
    }

    ~CMutexAttr()
    {
        pthread_mutexattr_destroy(&mtMutexAttr);
    }

    int getpshared(int* apiPShared)
    {
        return pthread_mutexattr_getpshared(&mtMutexAttr, apiPShared);
    }

    int setpshared(int aiPShared)
    {
        return pthread_mutexattr_setpshared(&mtMutexAttr, aiPShared);
    }

    int getprioceiling(int* apiPrioceiling)
    {
        return pthread_mutexattr_getprioceiling(&mtMutexAttr, apiPrioceiling);
    }

    int setprioceiling(int aiPrioceiling)
    {
        return pthread_mutexattr_setprioceiling(&mtMutexAttr, aiPrioceiling)
    }

    int getprotocol(int* apiProtocol)
    {
        return pthread_mutexattr_getprotocol(&mtMutexAttr, apiProtocol);
    }

    int setprotocol(int aiProtocol)
    {
        return pthread_mutexattr_setprotocol(&mtMutexAttr, aiProtocol);
    }
    
    int gettype(int* apiType);
    {
        return pthread_mutexattr_gettype(&mtMutexAttr, apiType);
    }

    int settype(int)
    {
        return pthread_mutexattr_settype(&mtMutexAttr, aiType);
    }
private:
    friend class CMutex;
    pthread_mutexattr_t mtMutexAttr;
};

class CMutex
{
public:
    int init(CMutexAttr* apoMutexAttr=NULL)
    {
        return pthread_mutex_init(&mtMutex, apoMutexAttr);
    }

    int destroy()
    {
        return pthread_mutex_destroy(&mtMutex);
    }

    int lock()
    {
        return pthread_mutex_lock(&mtMutex);
    }

    int unlock()
    {
        return pthread_mutex_unlock(&mtMutex);
    }

private:
    pthread_mutex_t mtMutex;
};


class CCondAttr
{
public:
    int getpshared(int* apiPShared)
    {
        return pthread_condattr_getpshared(&mtCondAttr, apiPShared);
    }

    int setpshared(int aiPShared)
    {
        return pthread_condattr_setpshared(&mtCondAttr, aiPShared);
    }

    int getclock(clockid_t* aptClockId);
    {
        return pthread_condattr_getclock(&mtCondAttr, aptClockId);
    }

    int setclock(clockid_t atClockId);
    {
        return pthread_condattr_setclock(&mtCondAttr, atClockId);
    }

private:
    friend class CCond;
    pthread_condattr_t  mtCondAttr; 
};


class CThread
{
public:
    int cancel()
    {
        return pthread_cancel(&mtThreadId);
    }

    int kill()
    {
        return pthread_kill(&mtThreadId);
    }

    int join(void** appValuePtr)
    {
        return pthread_join(&mtThreadId, appValuePtr);
    }

    int detach()
    {
        return pthread_detach(&mtThreadId);
    }

public:
    int Sigmask();
    int AtFork();
    int SetState();

private:
    pthread_t   mtThreadId; 
};

class CBaseSemaphore
{
public:
    int getvalue(int* apiSVal)
    {
        return sem_getvalue(&mtSem, apiSVal);
    }

    int post()
    {
        return sem_post(&mtSem);
    }

    int wait()
    {
        return sem_wait(&mtSem);
    }

protected:
    CBaseSemaphore() {};
    sem_t  mtSem;
};


class CUnnamedSemaphore
    :public CBaseSemaphore
{
public:
    int init(int aiPShared, unsigned auValue)
    {
        return sem_init(&mtSem, aiPShared, auValue);
    }

    int destroy()
    {
        return sem_destroy(&mtSem);
    }
};


class CNamedSemaphore
    :public CBaseSemaphore
{
public:
    int open(char const* apcName, int aiOFlag, ...)
    {
        return sem_open(&mtSema, apcName, aiOFlag, ...);
    }

    int close()
    {
        return sem_close(&mtSema);
    }

public:
    static int Unlink(char const* apcName)
    {
        return sem_unlink(apcName);
    }
};

// str
class CStr
{
public:
    CStr()
        :mpcStr(NULL)
    {
    };

    ~CStr()
    {
        free(mpcStr);
    }

    int copy(char* apcDest, size_t auSize)
    {
    }

    int cat(CStr const& aoStr)
    {
    }

    int printf(...);

private:
    char*   mpcStr
};

// mem
class CMem
{
};

// process
class CProcess
{
public:
    int getpid()
    {
        return mtProcessId;
    }

    int getppid()
    {
        return mtParentProcessId;
    }

public:
    static CProcess* GetCurrentProcess();
private:
    pid_t   mtProcessId;
    pid_t   mtParentProcessId;
};


#endif //__AILIBEX__INTERFACE_H__

