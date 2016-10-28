
#ifndef AAF_DAEMONSERVER_H
#define AAF_DAEMONSERVER_H

#include "aaf/Server.h"
#include "acl/Process.h"
#include "acl/GetOpt.h"
#include "acl/FileLog.h"

AAF_NAMESPACE_START

class CModuleEnv
{
public:
    CModuleEnv(void);
    
    void UpdateShutdown( bool abIsShutdown );
    
    bool IsShutdown(void);
    
    void SetProcessName( char const* apcName );
    
    void SetLogLevel( apl_int_t aiLevel );
    
    apl_int_t GetLogLevel(void);
    
    char const* GetProcessName(void);
    
private:
    char macProcessName[APL_NAME_MAX];
    
    apl_int_t miLogLevel;
    
    bool mbIsShutdown;
};

///////////////////////////////////////////////////////////////////////////////////
class CDaemonServer : public IServer, public acl::CProcess
{
public:
    CDaemonServer(void);
    
    virtual ~CDaemonServer(void);
    
    virtual apl_int_t Startup( char const* apcConfigFileName );
    
    virtual apl_int_t Restart(void);
    
    virtual apl_int_t Shutdown(void);
    
    void SetArg( apl_int_t argc, char* argv[] );
    
    void UpdateShutdown( bool abIsShutdown );
    
    bool IsShutdown(void);
    
    apl_int_t GetLogLevel(void);
    
    char const* GetProcessName(void);
    
    apl_ssize_t KillAll( apl_int_t aiSignal );

protected:
    void RunByDaemon(void);
    
    void SaveProcessID( const char* apcProcessName, apl_int_t aiFlag );
    
    void ClearProcessID( const char* apcProcessName );
    
    apl_ssize_t KillAll( const char* apcProcessName, apl_int_t aiSignal );
    
    static void HandleSignalShutdown( int aiSignal );
    
private:
    apl_int_t miArgc;
    
    char** mppcArgv;
    
    char macProcessName[APL_NAME_MAX];
    
    apl_int_t miLogLevel;
    
    bool mbIsShutdown;
    
    acl::CFileLog moLog;
};

AAF_NAMESPACE_END

#endif //AAF_DAEMONSERVER_H
