
#ifndef ACL_SOCKCONNECTOR_H
#define ACL_SOCKCONNECTOR_H

#include "Utility.h"
#include "TimeValue.h"
#include "SockAddr.h"
#include "SockStream.h"

ACL_NAMESPACE_START

class CSockConnector
{
public:
	apl_int_t Connect( 
	    CSockStream& aoStream, 
	    CSockAddr const& aoRemoteAddr,
	    CTimeValue const& aoTimeout = CTimeValue::MAXTIME,
	    CSockAddr const* apoLocalAddr = NULL,
	    apl_int_t aiProtocol = 0 );
	    
};

ACL_NAMESPACE_END

#endif//ACL_SOCKCONNECTOR_H
