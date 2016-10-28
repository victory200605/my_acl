
#ifndef ACL_SOCKHANDLE_H
#define ACL_SOCKHANDLE_H

#include "Utility.h"
#include "SockAddr.h"

ACL_NAMESPACE_START

class CSockHandle
{
public:
    enum EIOption //Int <value> option
    {
        /**
         * <getsockopt/setsockopt> system call with SOL_SOCKET and SO_BROADCAST 
         */
        OPT_BROADCAST = 1,
        
        /**
         * <getsockopt/setsockopt> system call with SOL_SOCKET and SO_DEBUG
         */
        OPT_DEBUG,
        
        /**
         * <getsockopt/setsockopt> system call with SOL_SOCKET and SO_DONTROUTE
         */
        OPT_DONTROUTE,
        
        /**
         * <getsockopt/setsockopt> system call with SOL_SOCKET and SO_KEEPALIVE
         */
        OPT_KEEPALIVE,
        
        /**
         * <getsockopt/setsockopt> system call with SOL_SOCKET and SO_OOBINLINE
         */
        OPT_OOBINLINE,
        
        /**
         * <getsockopt/setsockopt> system call with SOL_SOCKET and SO_RCVBUF
         */
        OPT_RCVBUF,
        
        /**
         * <getsockopt/setsockopt> system call with SOL_SOCKET and SO_SNDBUF
         */
        OPT_SNDBUF,
        
        /**
         * <getsockopt/setsockopt> system call with SOL_SOCKET and SO_RCVLOWAT
         */
        OPT_RCVLOWAT,
        
        /**
         * <getsockopt/setsockopt> system call with SOL_SOCKET and SO_SNDLOWAT
         */
        OPT_SNDLOWAT,
        
        /**
         * <getsockopt/setsockopt> system call with SOL_SOCKET and SO_REUSEADDR
         */
        OPT_REUSEADDR,
        
        /**
         * <getsockopt/setsockopt> system call with SOL_SOCKET and SO_TCPNODELAY
         */
        OPT_TCPNODELAY
    };
    
    enum EI64Option
    {
        /**
         * <getsockopt/setsockopt> system call with SOL_SOCKET and SO_RCVTIMEO
         */
        OPT_RCVTIMEO,
        
        /**
         * <getsockopt/setsockopt> system call with SOL_SOCKET and SO_SNDTIMEO
         */
        OPT_SNDTIMEO,

    };
    
    enum EOptionLinger
    {
        /**
         * <getsockopt/setsockopt> system call with SOL_SOCKET and SO_LINGER
         */
        OPT_LINGER,
    };
    
    enum EIGOOption //Int <value> and get only option
    {
        /**
         * <getsockopt> system call with SOL_SOCKET and SO_ERROR
         */
        OPT_ERROR = 1,
        
        /**
         * <getsockopt> system call with SOL_SOCKET and SO_TYPE
         */
        OPT_TYPE,
        
        /**
         * <getsockopt> system call with SOL_SOCKET and SO_ACCEPTCONN
         */
        OPT_ACCEPTCONN
    };
    
    enum ESWOption
    {
        /**
         * non-blocking I/O APL_O_NONBLOCK
         */
        OPT_NONBLOCK = 1,
        
        /**
         * close-on-exec APL_CLOEXEC
         */
        OPT_CLOEXEC
    };
    
public:
    CSockHandle(void);
    
    void SetHandle( apl_handle_t aiHandle );
    
    apl_handle_t GetHandle(void) const;
    
    apl_int_t Open( apl_int_t aiFamily, apl_int_t aiType, apl_int_t aiProtocol = 0 );
    
    void Close(void);

    apl_int_t Disable( ESWOption aeOpt ) const;
    
    apl_int_t Enable( ESWOption aeOpt ) const;
    
    apl_int_t GetLocalAddr( CSockAddr& aoAddr ) const;

    apl_int_t GetRemoteAddr( CSockAddr& aoAddr ) const;
    
    apl_int_t GetOption( EIOption aeOpt, apl_int_t* apiOptVal ) const;

    apl_int_t SetOption( EIOption aeOpt, apl_int_t aiOptVal ) const;

    apl_int_t GetOption( EIGOOption aeOpt, apl_int_t* apiOptVal ) const;
    
    apl_int_t GetOption( EI64Option aeOpt, apl_int64_t* apiOptVal ) const;

    apl_int_t SetOption( EI64Option aeOpt, apl_int64_t aiOptVal ) const;
    
    apl_int_t GetOption( EOptionLinger aeOpt, apl_int_t* apiOnoff, apl_int_t* apiLonger ) const;

    apl_int_t SetOption( EOptionLinger aeOpt, apl_int_t apiOnoff, apl_int_t apiLonger ) const;

protected:
    apl_handle_t ciHandle;
};

ACL_NAMESPACE_END

#endif//ACL_SOCKHANDLE_H
