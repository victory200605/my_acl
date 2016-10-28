
#ifndef __AI_FILE_CACHE_ERRNO_H__
#define __AI_FILE_CACHE_ERRNO_H__

#include "Config.h"

// start namespace
AI_CACHE_NAMESPACE_START

// error number define
enum
{
    AI_NO_ERROR                   =  0,
    
    // Cache error
    AI_WARN_NO_FOUND              =  80200,
    AI_ERROR_OUT_OF_RANGE         = -80201,
    AI_ERROR_DEL_FREE_NODE        = -80202,
    AI_ERROR_NO_ENOUGH_SPACE      = -80203,
    AI_ERROR_UNOPENED             = -80204,
    AI_ERROR_DUP_RECORD           = -80205,
    AI_ERROR_UNCOMPATIBLE_VERSION = -80206,
    
    // File system
    AI_ERROR_OPEN_FILE            = -80210,
    AI_ERROR_WRITE_FILE           = -80211,
    AI_ERROR_READ_FILE            = -80212,
    AI_ERROR_SYNC_FILE            = -80213,
    AI_ERROR_INVALID_FILE         = -80214,
    AI_ERROR_INVALID_DATA         = -80215,
    AI_ERROR_MAP_FILE             = -80216,
    
    // Network
    AI_ERROR_SOCKET_SEND          = -80220,
    AI_ERROR_SOCKET_RECV          = -80221,
    AI_ERROR_INVALID_REQUST       = -80222,
    AI_ERROR_INVALID_RESPONSE     = -80223,
    
    // System
    AI_ERROR_THREAD_POOL_FULL     = -80230,  // thread pool full
	AI_ERROR_SYSTEM               = -80231   // system error
};

inline bool IsWarning( int aiErrno ) { return ( AI_WARN_NO_FOUND == aiErrno )? true : false; };
// end define

inline const char* StrError( int aiErrNo )
{
    switch(aiErrNo)
    {
        case AI_NO_ERROR:                            return "No Error";
	    case AI_WARN_NO_FOUND:                       return "Record unexisted";
	    case AI_ERROR_OPEN_FILE:                     return "Open file fail";
	    case AI_ERROR_WRITE_FILE:                    return "Write file fail";
	    case AI_ERROR_READ_FILE:                     return "Read file fail";
	    case AI_ERROR_SYNC_FILE:                     return "Sync file fail";
	    case AI_ERROR_INVALID_FILE:                  return "Invalid file";
	    case AI_ERROR_INVALID_DATA:                  return "Invalid data node";
	    case AI_ERROR_MAP_FILE:                      return "Map file fail";
	    case AI_ERROR_UNCOMPATIBLE_VERSION:          return "uncompatible version";
	        
	    case AI_ERROR_OUT_OF_RANGE:                  return "Out of range";
	    case AI_ERROR_DEL_FREE_NODE:                 return "Can't delete free node";
	    case AI_ERROR_NO_ENOUGH_SPACE:               return "No enough space";
	    case AI_ERROR_UNOPENED:                      return "Cache unopened";
	    case AI_ERROR_DUP_RECORD:                    return "Record existed";
	    case AI_ERROR_SOCKET_SEND:                   return "Send data fail";
		case AI_ERROR_SOCKET_RECV:                   return "Rev data fail";
		case AI_ERROR_INVALID_REQUST:                return "Invalid protocol request";
		case AI_ERROR_INVALID_RESPONSE:              return "Invalid protocol response";
		    
	    case AI_ERROR_THREAD_POOL_FULL:              return "Thread pool full";
	    case AI_ERROR_SYSTEM:                        return "System error";
	    default:                                     return "UnKnow Error";
    }
}

//end namespace
AI_CACHE_NAMESPACE_END

#endif //__AI_FILE_CACHE_ERRNO_H__
