
#ifndef AIBC_GFQCONFIG_H
#define AIBC_GFQCONFIG_H

#include "apl/apl.h"

#define AIBC_GFQ_NAMESPACE_START namespace gfq {
#define AIBC_GFQ_NAMESPACE_END }

// start namespace
//AIBC_GFQ_NAMESPACE_START
namespace gfq {

#include "GFQConfig.inc"

// const variable define
// max file name length
const size_t MAX_NAME_LEN  = 128;
const size_t MAX_FILENAME_LEN = APL_NAME_MAX;

// max data buffer length
const size_t MAX_DATA_LEN  = GFQ_DATA_LEN;

// max ip address length
const size_t IP_ADDR_LEN   = 20;

// max ip address length
const size_t MAX_INT_LEN   = 25;

// module name hash size
const size_t MODULE_HASH_SIZE  = 100000;

// queue name hash size
const size_t QUEUE_HASH_SIZE   = 100000;
// single file max size

const size_t MAX_S_FILE_SIZE   = 0x7FFFFFFF; //2GB
// end variable define

enum {
	NO_ERROR = 0,
	ERROR_UNKNOW               = -80100, // unknow error

	ERROR_OPEN_DIR             = -80110, // open dir fail
	ERROR_OPEN_FILE            = -80111, // open file fail
	ERROR_READ_FILE            = -80112, // read file fail
	ERROR_WRITE_FILE           = -80113, // open file fail
	ERROR_SYNC_FILE            = -80114, // sync file fail
	ERROR_MAP_FILE             = -80115, // map file fail
	ERROR_INVALID_FILE         = -80116, // invalid file fail
	ERROR_INVALID_DATA         = -80117, // invalid data
	ERROR_LOAD_SHLIB           = -80118, // load share lib fail

	ERROR_CONNECT_REJECT       = -80120, // connect reject
	ERROR_SOCKET_SEND          = -80121, // socket send data fail
	ERROR_SOCKET_RECV          = -80122, // socket rev data fail
	ERROR_OPEN_SERVER          = -80123, // open socket server fail
	ERROR_INVALID_REQUST       = -80124, // invalid request
	
	ERROR_OUT_OF_RANGE         = -80130, // out of range
	ERROR_NO_FOUND_QUEUE       = -80132, // queue unexisted
	ERROR_NO_FOUND_MODULE      = -80133, // module unexisted
	ERROR_DUP_MODULE           = -80134, // dup module
	ERROR_DUP_QUEUE            = -80135, // dup queue
	ERROR_NO_ENOUGH_SPACE      = -80136, // queue no enough space
	ERRRO_INVALID_QUEUE        = -80138,

	ERROR_THREAD_POOL_FULL     = -80140, // thread pool full
	ERROR_SYSTEM               = -80141,  // system error
	
	ERROR_QUEUEINFO_LOAD_FUNC_UNDEFINED = -80150, // queue info load func undefined
	ERROR_PERMISION_FUNC_UNDEFINED      = -80151, // permision func undefined
	ERROR_QUEUEINFO_LOAD                = -80152, // queue info load fail
	ERROR_HEARTBEAT_TIMEOUT             = -80153,
	ERROR_SERVER_DENIED                 = -80154,
	
	//warning
	ERROR_EMPTY_OF_SEGMENT     = -80200, // empty of segment
	ERROR_END_OF_SEGMENT       = -80201, // end of segment
	ERROR_END_OF_QUEUE         = -80202, // end of queue
	ERROR_PERMISION_DENIED     = -80203, // permision denied
	ERROR_NO_FOUND_OUTTIME     = -80204  // no found outtime node
};

inline bool IsWarning( int aiErrNo )
{
    if (ERROR_NO_FOUND_QUEUE == aiErrNo || aiErrNo / 100 == -802 )
    {
        return true;
    }
    return false;
}

inline const char* StrError( int aiErrNo )
{
    switch(aiErrNo)
    {
        ///
        case NO_ERROR:                            return "No Error";
        case ERROR_UNKNOW:                        return "Unknow error";
        ///
	    case ERROR_OPEN_FILE:                     return "Open file fail";
	    case ERROR_OPEN_DIR:                      return "Open dir fail";
	    case ERROR_WRITE_FILE:                    return "Write file fail";
	    case ERROR_READ_FILE:                     return "Read file fail";
	    case ERROR_SYNC_FILE:                     return "Sync file fail";
	    case ERROR_MAP_FILE:                      return "Map file fail";
	    case ERROR_LOAD_SHLIB:                    return "Load share lib fail";
	    case ERROR_INVALID_FILE:                  return "Invalid file";
	    case ERROR_INVALID_DATA:                  return "Invalid data";
	    ////
		case ERROR_CONNECT_REJECT:                return "Connect rejected";
		case ERROR_SOCKET_SEND:                   return "Send data fail";
		case ERROR_SOCKET_RECV:                   return "Rev data fail";
		case ERROR_OPEN_SERVER:                   return "Open socket server fail";
		case ERRRO_INVALID_QUEUE:                 return "Invalid queue create request";
		///
	    case ERROR_END_OF_QUEUE:                  return "End of queue";
	    case ERROR_END_OF_SEGMENT:                return "End of segment";
	    case ERROR_EMPTY_OF_SEGMENT:              return "Empty of segment";
	    case ERROR_NO_FOUND_OUTTIME:              return "No found timeout record";
	    case ERROR_NO_FOUND_QUEUE:                return "Queue unexisted";
	    case ERROR_NO_FOUND_MODULE:               return "Module unexisted";
	    case ERROR_OUT_OF_RANGE:                  return "Out of range";
	    case ERROR_DUP_MODULE:                    return "Store Module existed";
	    case ERROR_PERMISION_DENIED:              return "Permision denied";
	    case ERROR_DUP_QUEUE:                     return "Queue existed";
	    case ERROR_NO_ENOUGH_SPACE:               return "No enough space";
	    case ERROR_INVALID_REQUST:                return "Invalid protocol request";
	    ////
	    case ERROR_QUEUEINFO_LOAD_FUNC_UNDEFINED: return "Queue info load function undefined";
	    case ERROR_PERMISION_FUNC_UNDEFINED:      return "Permision function undefined";
	    case ERROR_QUEUEINFO_LOAD:                return "Queue info load fail";
	    case ERROR_HEARTBEAT_TIMEOUT:             return "Heart beat timeout";
	    case ERROR_SERVER_DENIED:                 return "Server denied";
	        
	    ////
	    case ERROR_THREAD_POOL_FULL:              return "Thread pool full";
	    case ERROR_SYSTEM:                        return "System error";
	    
	    default:                                     return "Unknow error";
    }
}

//end namespace
//AIBC_GFQ_NAMESPACE_END
}

#endif //AIBC_AFQ_CONFIG_H

