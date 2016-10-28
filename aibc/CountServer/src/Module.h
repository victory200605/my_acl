
#ifndef COUNTSERVER_MODULE_H
#define COUNTSERVER_MODULE_H

#include "acl/Process.h"
#include "acl/GetOpt.h"
#include "acl/FileLog.h"

class CModuleEnv
{
public:
    static CModuleEnv* Instance(void);
    
    void UpdateShutdown( bool abIsShutdown );
    
    bool IsShutdown(void);
    
private:
    bool mbIsShutdown;
};

///////////////////////////////////////////////////////////////////////////////////
class CModule : public acl::CProcess
{
public:
    CModule(void);
    
    virtual ~CModule(void);
    
    apl_int_t Run( apl_int_t argc, char* argv[] );
    
    char const* GetProcessName(void);
    
    apl_int_t GetLogLevel(void);

protected:
    void RunByDaemon(void);
    
    void SaveProcessID( const char* apcProcessName, apl_int_t aiFlag );
    
    apl_ssize_t KillAll( const char* apcProcessName, apl_int_t aiSignal );
    
    static void HandleSignalShutdown( int aiSignal );
    
    virtual void Child(void);
    
protected:
    virtual apl_int_t Main( apl_int_t argc, char* argv[] ) = 0;
    
private:
    apl_int_t miLogLevel;
    
    apl_int_t miArgc;
    
    char** mppcArgv;
    
    char macProcessName[APL_NAME_MAX];
    
    acl::CFileLog moLog;
};

//////////////////////////////////////////////////////////////////////////////////////
void UpdateShutdown( bool abIsShutdown );

bool IsShutdown(void);

#define RUN_APP( module ) \
    int main( int argc, char* argv[] ) \
    { \
        module loModule; \
        loModule.Run(argc, argv); \
        return 0;\
    }

#endif //COUNTSERVER_MODULE_H
