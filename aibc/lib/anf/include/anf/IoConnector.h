
#ifndef ANF_IOCONNECTORR_H
#define ANF_IOCONNECTORR_H

#include "acl/SockAddr.h"
#include "anf/IoService.h"
#include "anf/future/ConnectFuture.h"

ANF_NAMESPACE_START
    
/**
 * Connects to endpoint, communicates with the server, and fires events to IoHandler.
 * You should connect to the desired socket address to start communication,
 * and then events for incoming connections will be sent to the specified default IoHandler.
 */
class CIoConnector : public CIoService
{
public:
    /**
     * Default constructor
     */
    CIoConnector(void);
    
    /**
     * Default destructor
     */
    virtual ~CIoConnector(void);
    
    void SetConnectTimeout( acl::CTimeValue const& aoTimeout );
        
    acl::CTimeValue& GetConnectTimeout(void);

    /**
     * Sets the default remote address to connect when no argument is specified in
     * Connect() method.  Please note that the default will not be used
     * if any local address is specified.
     */
    void SetDefaultRemoteAddress( acl::CSockAddr const& aoLocalAddress );
    
    /**
     * Returns the default remote address to connect when no argument is specified
     * in Connect() method.  Please note that the default will not be
     * used if any local address is specified.
     */
    void GetDefaultRemoteAddress( acl::CSockAddr& aoLocalAddress );
    
    /**
     * Connect to the default remote address and start to accept incoming
     * connections.
     *
     * @return connect future object, application can wait for completion synchronism
     */
    CConnectFuture::PointerType Connect(void);

    /**
     * Connect to the specified local address and start to accept incoming
     * connections.
     *
     * @return connect future object, application can wait for completion synchronism
     */
    CConnectFuture::PointerType Connect( acl::CSockAddr const& aoAddress );

protected:
    virtual CConnectFuture::PointerType Connect0( acl::CSockAddr const& aoAddress ) = 0;
        
private:
    acl::CTimeValue moConnectTimeout;
    
    acl::CSockAddr moRemoteAddress;
};

ANF_NAMESPACE_END

#endif//ANF_IOACCEPTOR_H
