#ifndef LOG_SERVER_DEV
#define LOG_SERVER_DEV

#include "LogServerMsg.h"


/**
 * Log level of logserver logger
 */
#define LOG_LVL_SYSTEM  1
#define LOG_LVL_ERROR   2
#define LOG_LVL_INFO    3
#define LOG_LVL_WARN    4
#define LOG_LVL_DEBUG   5
#define LOG_LVL_TRACE   6
#define LOG_LVL_ANY     7



/**
 * Impl class of ILogServerConfiger.
 */
class ILogServerConfigerImpl;

/**
 * Interface to passing configure items in LogServer's config file to *.so
 *
 * The config item would pass to *.so in ILogServerConfiger
 * format when Initialize. e.g.
 *
 * [xxxxx.so]
 * foo=hello
 * bar=world
 *
 * would threat as strings pair Key:Value (foo:hello, bar:world).
 * ILogServerConfiger::GetCfgItem() gets the char pointer of Key
 * and Value. *.so should handle these config items themselves.
 */
class ILogServerConfiger
{
public:
    ILogServerConfiger();

    ~ILogServerConfiger();

    /**
     * Get the counts of config item
     *
     * @retval >=0 Successful
     * @retval <0  Failed
     */
    GetSize();

    /**
     * Get the key, value pair by index
     *
     * @param[out] appKey   SHOULD NOT NULL, will be fill with Key's pointer
     * @param[out] appValue SHOULD NOT NULL, will be fill with Value's pointer
     *
     * @retval =0 Successful
     * @retval <0  Failed
     */
    GetCfgItem(int aiIdx, char** appKey, char** appValue);

private:
    CLogServerConfigerImpl* mpoLogServerConfigerImpl;
};

/**
 * Impl class of logserver logger.
 */
class CLogServerLoggerImpl;

/**
 * Logger of Log Server,  for debug purpose. *.so could use it as a logger.
 */
class CLogServerLogger
{
public:
	CLogServerLogger();

	~CLogServerLogger();

    /**
     * Write logs in apcFormat to LogServer's log file. The log file is 
     * configured in LogServer's config file.
     *
     * @param[in] aiLevel   Log level in LOG_LVL_*
     * @param[in] apcFormat log format like printf.
     * @param[in] ...      the content of log 
     *
     * @retval >0. the number of elements successfully written 
     * @retval <=0. If fail
     */
    int Write(int aiLevel, char const* apcFormat, ... );
    
    /**
     * Write auLength bytes from apcPtr to LogServer's log file. The log 
     * file is configured in LogServer's config file.
     *
     * @param[in] aiLevel   Log level in LOG_LVL_*
     * @param[in] apcPtr    The pointer to log content
     * @param[in] auLength  The length of log content
     *
     * @retval >0. the number of elements successfully written 
     * @retval <=0. If fail
     */
    int WriteHex(int aiLevel, char const* apcPtr, size_t auLength );
            
private:
	CLogServerLoggerImpl* mpoLoggerImpl;
}

/**
 * Base class of secondary development 
 *
 */
class ILogServerPlugin
{
public:

    /**
     * Initialize function of *.so
     *
     * @param[in] apoLogger logger for debug purpose, *.so could use the
     *                      this for logging. 
     * @param[in] apoCfgList config item list for *.so from LogServer
     * @retval=0 Successful
     * @retval!=0 Failed, see logging for more details
     */
    virtual int LogServerPluginInitialize(
		    CLogServerLogger* apoLogger, 
		    ILogServerConfiger const& apoConfiger) = 0;

    /**
     * Release *.so function
     *
     * @retval 0    success
     * @retval !=0  error, see logging for more details
     */
    virtual int LogServerPluginRelease() = 0;

    /**
     * Log Server send request to *.so and wait for its reponse.
     * There are two kinds of requests at this point: Logging message 
     * and Bill message.
     *
     * @param[in] aoReq         contain the message to Log Server
     *
     * @retval >=0  call *.so function successful, retval means the response stats
     * @retval <0  	call *.so function failure
     */
    virtual int LogServerPluginOnNewRequest(CLogServerMsg const& aoReq) = 0;
};


typedef ILogServerPlugin* LOGSERVER_PLUGIN_CREATE_T();
typedef void LOGSERVER_PLUGIN_DESTROY_T(ILogServerPlugin*);

/**
 * Use for create object. 
 * create function name which LogServer will involve(dlopen)
 * extern "C" ILogServerPlugin* logserver_plugin_create();
 */

/**
 * Use for destroy object.
 * destroy function name which LogServer will involve(dlopen)
 * extern "C" void logserver_plugin_destroy(ILogServerPlugin* p);
 */


#if 0
/**
 * Example *.so development class. 
 * #################### START of example ####################
 */

/**
 * Impl class for CLogServerDev
 */
class CLogServerDevImpl;

/**
 * *.so should implement/finish all kinds of operation
 */
class CLogServerDev: public ILogServerPlugin
{
public:

    CLogServerDev();

    ~CLogServerDev();

    virtual int LogServerPluginInitialize(CLogServerLogger* apoLogger, ILogServerConfiger const& apoConfiger);

    virtual int LogServerPluginRelease();

    virtual int LogServerPluginOnNewRequest(CLogServerMsg const& aoReq); 

protect:
    CLogServerDevImpl* mpoLogServerDevImpl;
};


/**
 * use for create CLogServerDev object.
 */
extern "C" ILogServerPlugin* logserver_plugin_create() {
    return new CLogServerDev;
}

/**
 * use for destroy CLogServerDev object.
 */
extern "C" void logserver_plugin_destroy(ILogServerPlugin* p) {
    delete p;
}

/**
 * Example *.so development class. 
 * #################### END of example ####################
 */
#endif


/**
 * Example *.so config section in Log Server's config file:
 *
 * [xxxxx.so]
 * foo=hello
 * bar=world
 *
 * All of these parameters would be pass to *.so in Initialize()
 * via ILogServerConfiger
 */

#endif // LOG_SERVER_DEV
