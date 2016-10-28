
#ifndef __AILIB_MODULEX_H__
#define __AILIB_MODULEX_H__

#include <unistd.h>

#include "AILib.h"
#include "AIConfig.h"
#include "AIChunkEx.h"
#include "AISyncTcp.h"
#include "AIHashArray.h"
#include "AIArgument.h"

namespace AIM {
    
///AI handle define
#define AI_PHANDLE       pid_t
#define AI_THANDLE       pthread_t
#define AI_HANDLE        int
#define IS_VALID_PHANDLE( handle ) (handle>0)

///Define main function pointer type
typedef void (*TCommandHookRegFunc)( void* env );
typedef void (*TArgumentOptionRegFunc)( AIBC::AIArgument& aoArgument  );
typedef int  (*TInitializeFunc)( void );
typedef void (*TFinallyFunc)( void );
typedef int  (*TReloadFunc)( void );
extern TReloadFunc gpfReLoad;

///AI const define
#define  AIM_MAX_NAME_LEN          (128)
#define  AIM_MAX_VERSION_LEN       (20)
#define  AIM_MAX_TIME_LEN          (64)
#define  AIM_MAX_COMMAND_LEN       (128)
#define  AIM_MAX_PATH_LEN          (PATH_MAX)
#define  AIM_MAX_CHILD_CNT         (100)
#define  AIM_MAX_EXT_CNT           (100)
#define  AIM_MAX_LINE_LEN          (1024)
#define  AIM_IP_ADDR_LEN           (20)
#define  AIM_MAX_PTL_ARGC          (20)
#define  AIM_MAX_COMMAND_HOOK_CNT  (100)
#define  AIM_MAX_LISTEN_SOCKET_CNT (100)

#define  AIM_LOCKFILE_PATH         "./lock"
#define  AIM_COMMAND_HELLI         "HELLI"
#define  AIM_COMMAND_LOGOUT        "LOGOUT"
#define  AIM_COMMAND_HELLO         "HELLO"
#define  AIM_COMMAND_LRESPOK       "OK"
#define  AIM_COMMAND_LRESPFAIL     "FAIL"
#define  AIM_COMMAND_RELOAD        "RELOAD"
#define  AIM_COMMAND_SHUTDOWN      "SHUTDOWN"
#define  AIM_COMMAND_GETSOCKET     "GETSOCKET"
#define  AIM_COMMAND_TRACE         "TRACE"

///end define

///start define for hook register marco
#define BEGIN_COMMAND_HOOK( caller ) \
    namespace AIM{ extern AIM::TCommandHookRegFunc __gf##caller##HookRegFunc; } \
    void __##caller##CommandHookRegFunc( void* env ); \
    class __auto_##caller##_reg_hook \
    { \
    public:__auto_##caller##_reg_hook() { AIM::__gf##caller##HookRegFunc = __##caller##CommandHookRegFunc; } \
    } __go_auto_##caller##_reg_hook; \
    void __##caller##CommandHookRegFunc( void* env ) {

#define HOOK( command, func )  \
    if ( static_cast<AIM::AIModuleEnv*>(env)->FindCommandHook(command) == NULL ) \
    { \
        static_cast<AIM::AIModuleEnv*>(env)->RegisterCommandHook( command, (void*)func ); \
    } \
    else \
    { \
        AIM_ERROR( "%s command redefined", command ); assert( false ); \
    }
    
#define END_COMMAND_HOOK()  }
///end declare

///start define for argument option register marco
#define BEGIN_ARG_OPTION() \
    namespace AIM{ extern TArgumentOptionRegFunc __gfArgumentOptionRegFunc; } \
    void __ArgumentOptionRegFunc( AIBC::AIArgument& aoArgument ); \
    class __auto_argument_option_reg \
    { \
    public:__auto_argument_option_reg() { AIM::__gfArgumentOptionRegFunc = __ArgumentOptionRegFunc; } \
    } __go_auto_argument_option_reg; \
    void __ArgumentOptionRegFunc( AIBC::AIArgument& aoArgument ) { \
        AIM::DefaultArgumentOption(aoArgument);

#define OPTION( opt, ... ) aoArgument.AddOption( opt, __VA_ARGS__ );

#define END_ARG_OPTION() }
///

#define SET_MODULE_VERSION( module, version, time ) \
    static class __auto_##module##_version_reg \
    { \
    public: \
        __auto_##module##_version_reg() \
        { \
            AIM::AIModuleEnv::Instance()->AddModuleVersion( #module, version, time ); \
        } \
    } __go_auto_##module##_version_reg;
/////

///Define module log marco
#define AIM_LOG_INIT( name )     assert( AIM::AIModuleLog::Instance()->Open(name) == 0 )
#define AIM_ERROR( format, ... ) AIM::AIModuleLog::Instance()->Write( "ERROR: " format "\n", __VA_ARGS__ );
#define AIM_INFO( format, ... )  AIM::AIModuleLog::Instance()->Write( "INFO: " format "\n", __VA_ARGS__ );

////////////////////////////////////////// *ModuleLog* ///////////////////////////////////////////
class AIModuleLog
{
public:
    static AIModuleLog* Instance();
    static void Release();
    
    ~AIModuleLog();
    
    int Open( const char* apcFileName );
    void Close();
    
    int Write( const char *apcFormat, ... );

protected:
    AIModuleLog();
    
    static AIModuleLog* scpoInstance;
    
protected:
    int ciFileHandle;
    AIBC::AIMutexLock coLock;
};

////////////////////////////////////////// *ProcessManager* //////////////////////////////////////
class AIProcessManager
{
public:
    struct stProcessNode
    {
        int ciBookTime;
        int ciLoginTime;
        int ciLogoutTime;
        AI_PHANDLE ctPHandle;
    };
    
    enum CHILD_STATE { NORMAL = 0, LOGOUT, TIMEOUT };
    
public:
    AIProcessManager( const char* apcProcessName, size_t aiMaxChildCnt = AIM_MAX_CHILD_CNT );
    ~AIProcessManager();
    
    void SetMaxChildCnt( size_t aiMaxChildCnt );
    
    int LoadFromFile();
    int PersistentToFile();
    
    /// 
    int WaitAll();
    int StopAll();
    int KillAll();
    int ReloadAll();
    
    //// operation
    int  GetFreeNo();
    int  Find( AI_PHANDLE atPHandle );
    void ProcBook( size_t aiNo, AI_PHANDLE atPHandle );
    void ProcLogin( AI_PHANDLE atPHandle );
    void ProcLogout( AI_PHANDLE atPHandle );
    void RecycleRubbish();
    CHILD_STATE DetectState( int aiTimeOut );
    
    AI_PHANDLE operator [] ( size_t aiN );
    size_t GetCount();

    //// Handle function
    static bool IsProcessExisted( AI_PHANDLE atPHandle );
    static int  HandleStop( AI_PHANDLE atPHandle );
    static int  HandleKill( AI_PHANDLE atPHandle );
    static int  HandleReload( AI_PHANDLE atPHandle );
    
protected:
    stProcessNode* cpoProcessNode;
    char           csFileName[AIM_MAX_NAME_LEN];
    size_t         ciProcCnt;
    size_t         ciCapacity;
    AIBC::AIMutexLock    coLock;
};

////////////////////////////////////////// *Config* /////////////////////////////////////////////
class AIModuleConfig
{
public:
    struct stIntIterator
    {
        stIntIterator( AIBC::AIConfig* apoConfig, const char* apcField, const char* apcList, int aiDefault );
        
        bool IsEof()
        {
            return this->cpcFirst == this->cpcLast;
        }
        
        const char* CurrKey();
        
        void operator ++ ( int );
        
        int operator * ();
        
        const char* cpcFirst;
        const char* cpcLast;
        AIBC::AIConfig*   cpoConfig;
        const char* cpcField;
        int         ciDefault;
    };
    
    typedef stIntIterator TIntIter;
    
public:
    static AIModuleConfig* Instance();
    AIModuleConfig();
    int Load( const char* apcPath, const char* apcField );
    
    int GetIntFromConfig( const char* apcField, const char* apcKey, int aiDefault );
    int GetStrFromConfig( const char* apcField, const char* apcKey, char* apcValue, size_t aiSize );
    stIntIterator GetIntListFromConfig( const char* apcField, const char* apcListKey, int aiDefault );
    
    uint32_t    GetChildCount()           { return ciChildCnt; }
    uint32_t    GetListenPort()           { return ciListenPort; }
    const char* GetModuleID()             { return csModuleID; }
    const char* GetListenIpAddr()         { return csListenIpAddr; }
    const char* GetListenPassword()       { return csPassword; }

protected:
    uint32_t  ciChildCnt;
    uint32_t  ciListenPort;
    char      csModuleID[AIM_MAX_NAME_LEN];
    char      csListenIpAddr[AIM_IP_ADDR_LEN];
    char      csPassword[AIM_IP_ADDR_LEN];
    AIBC::AIConfig  coConfig;
};

////////////////////////////////////////// *Protocol* /////////////////////////////////////////////
///
class AIModuleProtocol
{
public:
    AIModuleProtocol();
    ~AIModuleProtocol();
    
    int Decode( const char* apcData, size_t aiSize );
    static int  Encode( AIBC::AIChunkEx& aoBuffer, const AIBC::AIChunkEx* apoAppendedData, const char* apcFormat, ... );
    
    bool        IsCommand( const char* apcCmd ) const;
    bool        IsHasAppended() const;
    
    const char* GetCommandLine() const;
    const char* GetCommand() const;
    const char* GetParam( size_t aiN ) const;
    size_t      GetParamCnt() const;
    const AIBC::AIChunkEx& GetAppendedData() const;

protected:
    AIBC::AIChunkEx coAppendedData;
    AIBC::AIChunkEx coCommandLine;
    AIBC::AIChunkEx coArgv[AIM_MAX_PTL_ARGC];
    size_t    ciArgc;
};

////////////////////////////////////////// *ModuleEnv* //////////////////////////////////////
class AIModuleEnv
{
public:
    struct stHookNode
    {
        typedef const char* key_type;
        const char* operator() ( const stHookNode& aoR ) const  { return aoR.csCommand; }
        bool operator () ( key_type apcL, key_type apcR ) const { return strcmp( apcL, apcR ) == 0 ? true : false; }
            
        char  csCommand[AIM_MAX_COMMAND_LEN];
        void* cfHookFunc;
    };
    
    struct stModuleVersion
    {
        char csModuleName[AIM_MAX_NAME_LEN];
        char csVersion[AIM_MAX_VERSION_LEN];
        char csTime[AIM_MAX_TIME_LEN];
        
        stModuleVersion* Next()             { return cpoNext; }
        
        stModuleVersion* cpoNext;
    };
    
    typedef stModuleVersion* TVersionHandle;
    
public:
    static AIModuleEnv* Instance();
    ~AIModuleEnv();
    
    int  Initialize( const char* apcProcessName );
    static int RunByDaemon();
    
    void  UpdateShutdown( bool aiFlag );
    void  UpdateReload( bool aiFlag );
    
    void  ClearHook();
    void  RegisterCommandHook( const char* apcCommand, void* afHook );
    void* FindCommandHook( const char* apcCommand );
    
    bool  IsShutdown();
    bool  IsReload();
    
    void  SetRootDirectory( const char* apcDirectory = NULL );
    void  SetHomeDirectory( const char* apcDirectory = NULL );
    
    const char* GetRootDirectory() const;
    const char* GetHomeDirectory() const;
    const char* GetProcessName() const;
    
    //////compatible aimodule last version
    void AddModuleVersion( const char* apcModuleName, const char* apcVersion, const char* apcTime );
    TVersionHandle FirstVersion();

protected:
    AIModuleEnv();

protected:
    char csRootDirectory[AIM_MAX_PATH_LEN];
    char csHomeDirectory[AIM_MAX_PATH_LEN];
    char csProcessName[AIM_MAX_NAME_LEN];
    bool cbIsShutdown;
    bool cbIsReload;
    AIBC::AIHashArray<stHookNode, AIM_MAX_COMMAND_HOOK_CNT, stHookNode> coHookNode;
    
    //////compatible aimodule last version
    TVersionHandle cpoFirst;
    TVersionHandle cpoLast;
};

////////////////////////////////////////// *ModuleConnServer* //////////////////////////////////////
class AIModuleConnServer
{
public:
    struct stServerNode
    {
        typedef const char* key_type;
        const char* operator() ( const stServerNode& aoR ) const { return aoR.csName; }
        bool operator () ( key_type apcL, key_type apcR ) const { return strcmp( apcL, apcR ) == 0 ? true : false; }
        
        int         ciPort;
        AI_HANDLE   ciSocket;
        char        csName[AIM_MAX_NAME_LEN];
        AIBC::AIMutexLock coLock;
    };
    
public:
    static AIModuleConnServer* Instance();
    ~AIModuleConnServer();

    void AddListenPort( const char* apcName, int aiPort );
    void Close();
    int  ListenAll( int* apiNodeId = NULL );
    
    AI_HANDLE GetListenSocket( const char* apcName );
    AI_HANDLE AcceptNewConn( const char* apcName, char* apcIpAddr, int* apiPort );

    int GetPort( int aiN )              { return coServerNode[aiN].ciPort; }
    const char* GetName( int aiN )      { return coServerNode[aiN].csName; }
    
protected:
    AIModuleConnServer();

protected:
    AIBC::AIHashArray<stServerNode, AIM_MAX_LISTEN_SOCKET_CNT, stServerNode> coServerNode;
};

class AIModule;
////////////////////////////////////////// *Parent* //////////////////////////////////////
// ServerChannel
class AIModuleParent;
class AIServerChannel : public AIBC::AICmdLineChannel
{
public:
    AIServerChannel();
    void SetModule( AIModuleParent* apModule );
    
    virtual int Input( const char* apcData, size_t aiSize );
    virtual int Error( AI_SYNCHANDLE atHandle, int aiErrno );
    virtual int Exit();

protected:
    int HandleExtInput( AIModuleProtocol& aoProtocol );
    int HandleChildInput( AIModuleProtocol& aoProtocol );

protected:
    int ciRole;
    AIModuleParent* cpoModule;
};
// Module
class AIModuleParent : public AIBC::AISyncServer<AIServerChannel>
{
public:
    typedef AIBC::AISyncServer<AIServerChannel> TBase;
    typedef int (*THandleFunc)( AIModuleParent*, AIServerChannel*, const AIModuleProtocol* );
    
public:
    static AIModuleParent* Instance();
    ~AIModuleParent();
    
    static int  Initialize();
    static void Finally();
    
    int Run( const AIBC::AIArgument& aoArgument, AIModule* apoModule, const char* apcPortList );
    void Close();
        
    //TODO Hook Handle funtion
    static int HandleChildLogin( AIModuleParent* apoModule, AIServerChannel* apoChannel, const AIModuleProtocol* apoProtocol );
    static int HandleChildLogout( AIModuleParent* apoModule, AIServerChannel* apoChannel, const AIModuleProtocol* apoProtocol );
    static int HandleExtLogin( AIModuleParent* apoModule, AIServerChannel* apoChannel, const AIModuleProtocol* apoProtocol );
    static int HandleExtLogout( AIModuleParent* apoModule, AIServerChannel* apoChannel, const AIModuleProtocol* apoProtocol );
    static int HandleExtTrace( AIModuleParent* apoModule, AIServerChannel* apoChannel, const AIModuleProtocol* apoProtocol );
    static int HandleChildTrace( AIModuleParent* apoModule, AIServerChannel* apoChannel, const AIModuleProtocol* apoProtocol );
    static int HandleReload( AIModuleParent* apoModule, AIServerChannel* apoChannel, const AIModuleProtocol* apoProtocol );
    static int HandleShutdown( AIModuleParent* apoModule, AIServerChannel* apoChannel, const AIModuleProtocol* apoProtocol );
    //Default handle
    static int HandleRequest( AIModuleParent* apoModule, AIServerChannel* apoChannel, const AIModuleProtocol* apoProtocol );
    static int HandleRequestForResponse( AIModuleParent* apoModule, AIServerChannel* apoChannel, const AIModuleProtocol* apoProtocol );
    
    //TODO Handle signal
    static void HandleSignalShutdown( int aiSignal );
    static void HandleSignalReload( int aiSignal );

protected:
    virtual int Accept( AIBC::AIBaseChannel* apoChannel );
    int PutChannel( AIServerChannel** apoChannelArray, size_t aiCount, AIServerChannel* apoChannel );
    int EraseChannel( AIServerChannel** apoChannelArray, size_t aiCount, AIServerChannel* apoChannel );
    int RequestToChild( const AIBC::AIChunkEx& aoRequest, AIBC::AIChunkEx& aoResponse );
    int RequestToChild( const AIBC::AIChunkEx& aoRequest );
    
    int ChildLogin( AIServerChannel* apoChannel, AI_PHANDLE atPHandle );
    int ChildLogout( AIServerChannel* apoChannel, AI_PHANDLE atPHandle );
    int ExtLogin( AIServerChannel* apoChannel, const char* apcPassword );
    int ExtLogout( AIServerChannel* apoChannel );
    int PutTraceChannel( AIServerChannel* apoChannel );
    int ResponseTrace( const char* apcType, const AIBC::AIChunkEx& aoBuffer );
    int DoRequest( const AIBC::AIChunkEx& aoRequest, AIBC::AIChunkEx& aoResponse );
    int DoRequest( const AIBC::AIChunkEx& aoRequest );

protected:
    AIModuleParent();
    
protected:
    AIProcessManager* cpoProcessManager;
    AIServerChannel*  cpoaChildChannel[AIM_MAX_CHILD_CNT];
    AIServerChannel*  cpoaExtChannel[AIM_MAX_EXT_CNT];
    AIServerChannel*  cpoaTraceChannel[AIM_MAX_EXT_CNT];
    AIBC::AIMutexLock       coHandleLock;
};
////////////////////////////////////////// *Child* //////////////////////////////////////
// ClientChannel
class AIModuleChild;
class AIClientChannel : public AIBC::AICmdLineChannel
{
public:
    AIClientChannel();
    void SetModule( AIModuleChild* apModule );
    
    virtual int  Input( const char* apcData, size_t aiSize );
    virtual int  Error( AI_SYNCHANDLE atHandle, int aiErrno );
    virtual int  Exit();
    
protected:
    AIModuleChild* cpoModule;
};
// Module
class AIModuleChild
{
public:
    typedef int (*THandleFunc)( AIModuleChild*, AIClientChannel*, const AIModuleProtocol* );
    
public:
    static AIModuleChild* Instance();
    ~AIModuleChild();
    
    int Run( const AIBC::AIArgument& aoArgument, AIModule* apoModule );
    void Close();
    
    //TODO Handle funtion
    static int HandleGetSocket( AIModuleChild* apoModule, AIClientChannel* apoChannel, const AIModuleProtocol* apoProtocol );
    static int HandleTrace( AIModuleChild* apoModule, AIClientChannel* apoChannel, const AIModuleProtocol* apoProtocol );
    static int HandleReload( AIModuleChild* apoModule, AIClientChannel* apoChannel, const AIModuleProtocol* apoProtocol );
    static int HandleShutdown( AIModuleChild* apoModule, AIClientChannel* apoChannel, const AIModuleProtocol* apoProtocol );
    static int HandleReRegister( AIModuleChild* apoModule );

    int RequestToParent( AIBC::AIChunkEx& aoRequest );

protected:
    AIModuleChild();
    
    int RegisterTo( const char* apcServerIp, int aiPort );
    
    static void* TraceServerThread( void* apvParam );
    static void* DynLogLevelServerThread( void* apvParam );
    
protected:
    AI_THANDLE       ctTraceServerThread;
    AIBC::AISyncClient<AIClientChannel> coClient;
    AIClientChannel  coChannel;
    AIBC::AIMutexLock      coHandleLock;
};

/////////////////////////////////////// *AIModule* ///////////////////////////////
class AIModule
{
public:
    virtual ~AIModule(void) {};
    virtual int Initialize();
    virtual int Main( int argc, char* argv[] ) = 0;
    virtual void Finally();
    
    virtual int Run( int argc, char* argv[], const char* apcPortList );
};
///AISimpleModule
class AISimpleModule : public AIModule
{
public:
    virtual ~AISimpleModule(void) {};
    virtual int Run( int argc, char* argv[], const char* apcPortList );
};

//////////////////////////////////////////////////////////////////////////
void DefaultArgumentOption( AIBC::AIArgument& aoArgument );

#define RUN_APP( app, portlist ) \
    int main( int argc, char* argv[] ) \
    { \
        app loApp; \
        return loApp.Run( argc, argv, portlist ); \
    }
    
#define HANDLE_MAIN_MACRO( mainfunc, portlist ) \
    class __MainApp : public AIM::AIModule \
    { \
    public: \
        virtual int Main( int argc, char* argv[] ) \
        { \
            return (*mainfunc)( argc, argv, ::getppid() ); \
        } \
    }; \
    RUN_APP( __MainApp, portlist )

//////////////////////////////////////// global function ///////////////////////////////
///Get process shutdown flag
extern bool IsShutdown();

///Update process shutdown flag
extern void UpdateShutdown( bool aiFlag );

///Get process reload flag
extern bool IsReload();

///Update process reload flag
extern void UpdateReload( bool aiFlag );

///Set reLoad hook
extern void SetReLoadHook( TReloadFunc apfFunc );

///Get module id from config file
extern const char* GetModuleID();

///Get current process name
extern const char* GetProcessName();

///Get current child process count
extern int GetChildCount();

///Get current root directory
extern const char* GetRootDir();

///Get current root directory
extern const char* GetHomeDir();

///Accept new connection
extern int AcceptNewConn( const char *apcListenName, char *apcIpAddr, int *apiPort );
////////////
};

///compatible aimodule last version
////////////////////////////////////////////////////////////////////////////////////
/////
///start namespace
AIBC_NAMESPACE_START

extern volatile int  giGlobalReLoad;
extern volatile int  giGlobalShutDown;
extern volatile int  giGlobalLogLevel;
extern char gsGlobalModuleID[];
extern char gsGlobalProgName[];
extern char gsGlobalStartTime[];

#if !defined(DECLARE_CMDOPT_BEGIN) 
#define DECLARE_CMDOPT_BEGIN()
#endif

#if !defined(DECLARE_CMDOPT_END) 
#define DECLARE_CMDOPT_END()
#endif

////////////////////////////////////////////////////////////////////////////////////
/////
extern int SetReLoad( AIM::TReloadFunc apfFunc );
extern int GetListenSocket( const char *apcListenName );
extern int GetNewConnection( const char *apcListenName, char *apcIpAddr, int *apiPort );
////////////////////////////////////////////////////////////////////////////////////

///end namespace
AIBC_NAMESPACE_END

#endif // __AILIB_MODULE_H__
