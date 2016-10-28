
#ifndef AIBC_GFQ2_UTILITY_H
#define AIBC_GFQ2_UTILITY_H

#include "apl/apl.h"

#define AIBC_GFQ_NAMESPACE_START namespace gfq2 {
#define AIBC_GFQ_NAMESPACE_END }

// start namespace
//AIBC_GFQ_NAMESPACE_START
namespace gfq2 {

    const apl_size_t INVALID_ID = ~(apl_size_t)0;

    enum EOptionType
    {
        IGNORE_QUEUE_EXISTED = 1,
        IGNORE_CONNECT_FAIL
    };

    enum EErrorType
    {
        NO_ERROR = 0,

        ERROR_OPEN_DIR             = -80101,
        ERROR_OPEN_FILE            = -80102,
        ERROR_READ_FILE            = -80103,
        ERROR_WRITE_FILE           = -80104,
        ERROR_SYNC_FILE            = -80105,
        ERROR_MAP_FILE             = -80106,
        ERROR_INVALID_FILE         = -80107,
        ERROR_INVALID_DATA         = -80108,
        ERROR_LOAD_SHLIB           = -80109,
        ERROR_SOCKET_SEND          = -80110,
        ERROR_SOCKET_RECV          = -80111,
        ERROR_BROKEN_SESSION       = -80112,
        ERROR_CONNECT_REJECT       = -80113,
        ERROR_TIMEDOUT             = -80114,
        ERROR_SYSTEM               = -80115,

        ERROR_INVALID_MESSAGE      = -80125,
        ERROR_NODE_EXISTED         = -80126,
        ERROR_NODE_UNEXISTED       = -80127,
        ERROR_QUEUE_EXISTED        = -80128,
        ERROR_SEGMENT_EXISTED      = -80129,
        ERROR_SEGMENT_UNEXISTED    = -80130,
        ERROR_OUT_OF_RANGE         = -80131,
        ERROR_NO_ENOUGH_SPACE      = -80132,
        ERROR_INVALID_QUEUE        = -80133,
        ERROR_INVALID_SEGMENT      = -80134,
        ERROR_INVLID_METADATA      = -80136,
        ERROR_GROUP_EXISTED        = -80137,
        ERROR_GROUP_UNEXISTED      = -80138,
        ERROR_INVALID_TIMESTAMP    = -80139,
        ERROR_UNIMPLEMENTED        = -80135,

        //warning
        ERROR_EMPTY_OF_SEGMENT     = -80200,
        ERROR_END_OF_SEGMENT       = -80201,
        ERROR_END_OF_QUEUE         = -80202,
        ERROR_PERMISSION_DENIED    = -80203,
        ERROR_TIMEDOUT_UNEXISTED   = -80204, 
        ERROR_QUEUE_UNEXISTED      = -80205
    };

    inline bool IsWarning( int aiErrNo )
    {
        if (aiErrNo / 100 == -802 )
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    inline const char* StrError( int aiErrNo )
    {
        switch(aiErrNo)
        {
            ///
            case NO_ERROR:                            return "No Error";
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
            case ERROR_SOCKET_SEND:                   return "Send data fail";
            case ERROR_SOCKET_RECV:                   return "Recv data fail";
            case ERROR_BROKEN_SESSION:                return "Broken session";
            case ERROR_CONNECT_REJECT:                return "Connect reject";
            case ERROR_TIMEDOUT:                      return "Timedout";
            case ERROR_SYSTEM:                        return "System error";

            case ERROR_INVALID_MESSAGE:               return "Invalid message";
            case ERROR_NODE_EXISTED:                  return "Server node group existed";
            case ERROR_NODE_UNEXISTED:                return "Server node unexisted";
            case ERROR_QUEUE_EXISTED:                 return "Queue existed";
            case ERROR_SEGMENT_EXISTED:               return "Segment existed";
            case ERROR_SEGMENT_UNEXISTED:             return "Segment unexisted";
            case ERROR_OUT_OF_RANGE:                  return "Out of range";
            case ERROR_NO_ENOUGH_SPACE:               return "No enough space";
            case ERROR_INVALID_QUEUE:                 return "Invalid queue name";
            case ERROR_INVALID_SEGMENT:               return "Invalid segment id";
            case ERROR_INVLID_METADATA:               return "Invalid metadata";
            case ERROR_GROUP_EXISTED:                 return "Server node existed";
            case ERROR_GROUP_UNEXISTED:               return "Server node group unexisted";
            case ERROR_INVALID_TIMESTAMP:             return "Invalid timestamp";
            case ERROR_UNIMPLEMENTED:                 return "Unimplemented";
            
            case ERROR_EMPTY_OF_SEGMENT:              return "Empty of segment";
            case ERROR_END_OF_SEGMENT:                return "End of segment";
            case ERROR_END_OF_QUEUE:                  return "End of queue";
            case ERROR_PERMISSION_DENIED:             return "Permission denied";
            case ERROR_TIMEDOUT_UNEXISTED:            return "No found timeout record";
            case ERROR_QUEUE_UNEXISTED:               return "Queue unexisted";

            default:                                  return "Unknow error";
        }
    }

#define GFQ_MEMBER_METHOD_I( name ) \
    apl_int_t Get##name(void) { return this->mi##name; }\
    void Set##name( apl_int_t ai##name ) { this->mi##name = ai##name; }

#define GFQ_MEMBER_METHOD_I32( name ) \
    apl_int32_t Get##name(void) { return this->mi##name; }\
    void Set##name( apl_int32_t ai##name ) { this->mi##name = ai##name; }

#define GFQ_MEMBER_METHOD_I64( name ) \
    apl_int64_t Get##name(void) { return this->mi64##name; }\
    void Set##name( apl_int64_t ai64##name ) { this->mi64##name = ai64##name; }

#define GFQ_MEMBER_METHOD_U16( name ) \
    apl_uint16_t Get##name(void) { return this->mu16##name; }\
    void Set##name( apl_uint16_t au16##name ) { this->mu16##name = au16##name; }

#define GFQ_MEMBER_METHOD_U( name ) \
    apl_size_t Get##name(void) { return this->mu##name; }\
    void Set##name( apl_size_t au##name ) { this->mu##name = au##name; }

#define GFQ_MEMBER_METHOD_U64( name ) \
    apl_uint64_t Get##name(void) { return this->mu64##name; }\
    void Set##name( apl_uint64_t au64##name ) { this->mu64##name = au64##name; }

#define GFQ_MEMBER_METHOD_STR( name ) \
    char const* Get##name(void) { return this->mo##name.c_str(); }\
    void Set##name( std::string const& ao##name ) { this->mo##name = ao##name; }

#define GFQ_MEMBER_METHOD_CSTR( name ) \
    char const* Get##name(void) { return this->mac##name; }\
    void Set##name( char const* apc##name ) { apl_strncpy(this->mac##name, apc##name, sizeof(this->mac##name) ); }

//end namespace
//AIBC_GFQ_NAMESPACE_END
}

#endif //AIBC_GFQ_UTILITY_H

