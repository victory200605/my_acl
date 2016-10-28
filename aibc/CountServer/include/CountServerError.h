
#ifndef ISMG_CSVRERROR_H
#define ISMG_CSVRERROR_H

enum
{
    CNTSVR_NO_ERROR                 = 0,
    CNTSVR_ERROR_CONNRESET          = -81001,
    CNTSVR_ERROR_NO_ENOUGH_CONNNODE = -81002,
    CNTSVR_ERROR_TIMEOUT            = -81003,
    CNTSVR_ERROR_INVALID_PTL        = -81004,
    CNTSVR_ERROR_NO_FOUND           = -81005,
    CNTSVR_ERROR_LIMITED            = -81006
};

inline bool IsWarning( int aiErrNo )
{
    return false;
}

inline const char* StrError( int aiErrNo )
{
    switch(aiErrNo)
    {
        ///
        case CNTSVR_NO_ERROR:          return "No error";
        case CNTSVR_ERROR_CONNRESET:   return "Connection reset";
        case CNTSVR_ERROR_NO_ENOUGH_CONNNODE: return "No enough connection node";
        case CNTSVR_ERROR_TIMEOUT:     return "Time out";
        case CNTSVR_ERROR_INVALID_PTL: return "Invalid protocol";
        case CNTSVR_ERROR_NO_FOUND:    return "No found";
        case CNTSVR_ERROR_LIMITED:     return "Limited request";
        default:                       return "Unknow error";
    }
}

#endif//ISMG_CSVRERROR_H
