
#ifndef AAF_DAEMONSERVICE_H
#define AAF_DAEMONSERVICE_H

#include "aaf/Service.h"
#include "acl/Process.h"
#include "acl/GetOpt.h"
#include "acl/FileLog.h"
#include "acl/stl/string.h"
#include "acl/stl/map.h"

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
class CDaemonService : public IService, public acl::CProcess
{
    struct CUdfOption
    {
        bool mbIsHited;
        bool mbIsHasValue;
        std::string moValue;
        std::string moLongOption;
        std::string moDesc;
    };

    typedef std::map<apl_int_t, CUdfOption> UdfOptionMapType;

public:
    CDaemonService(void);
    
    virtual ~CDaemonService(void);
    
    virtual apl_int_t Startup( char const* apcConfigFileName );
    
    virtual apl_int_t Restart(void);
    
    virtual apl_int_t Shutdown(void);
    
    void SetArg( apl_int_t argc, char* argv[] );
    
    void UpdateShutdown( bool abIsShutdown );
    
    bool IsShutdown(void);
    
    bool SetOption( char acOpt, bool abIsHasValue, char const* apcLongOpt = NULL, char const* apcDesc = NULL );

    bool IsHasOption( char acOpt );
    
    char const* GetOptArg( char acOpt );
    
    apl_int_t GetLogLevel(void);
    
    char const* GetProcessName(void);
    
    apl_ssize_t KillAll( apl_int_t aiSignal );
    
    void PrintUsage(void);

    template<typename FormatterType>
    apl_int_t WriteLog( FormatterType const& aoFormatter )
    {
        return this->moLog.Write(aoFormatter.GetOutput(), aoFormatter.GetLength() );
    }

    //Hook method
    virtual apl_int_t Initialize(void);

    virtual void Finalize(void);

protected:
    void MakeOptionString( std::string& aoOptString );
    
    void SetLongOption( acl::CGetOpt& aoOpt );

    bool CheckAppendOption( apl_int_t aiOpt, char* apcOptArg );

    void RunByDaemon(void);
    
    void SaveProcessID( const char* apcProcessName, apl_int_t aiFlag );
    
    void ClearProcessID( const char* apcProcessName );
    
    apl_ssize_t KillAll( const char* apcProcessName, apl_int_t aiSignal );
    
    static void HandleSignalShutdown( int aiSignal );
    
private:
    apl_int_t miArgc;
    
    char** mppcArgv;

    std::string moOptString;

    UdfOptionMapType moUdfOptions;

    bool mbIsParseArgFail;
    
    char macProcessName[APL_NAME_MAX];
    
    apl_int_t miLogLevel;
    
    bool mbIsShutdown;
    
    acl::CFileLog moLog;
};

AAF_NAMESPACE_END

#endif //AAF_DAEMONSERVER_H
