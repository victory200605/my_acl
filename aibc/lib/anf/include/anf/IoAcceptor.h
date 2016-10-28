
#ifndef ANF_IOACCEPTOR_H
#define ANF_IOACCEPTOR_H

#include "acl/SockAddr.h"
#include "anf/IoService.h"

ANF_NAMESPACE_START
    
/**
 * Accepts incoming connection, communicates with clients, and fires events to
 * Threads accept incoming connections start automatically when
 * {@link #Bind()} is invoked, and stop when {@link #Unbind()} is invoked.
 */
class CIoAcceptor : public CIoService
{
public:
    /**
     * Default constructor
     */
    CIoAcceptor(void);
    
    /**
     * Default destructor
     */
    virtual ~CIoAcceptor(void);

    /**
     * Sets the default local address to bind when no argument is specified in
     * {@link #Bind()} method.  Please note that the default will not be used
     * if any local address is specified.
     */
    void SetDefaultLocalAddress( acl::CSockAddr const& aoLocalAddress );
    
    /**
     * Returns the default local address to bind when no argument is specified
     * in {@link #bind()} method.  Please note that the default will not be
     * used if any local address is specified.
     */
    void GetDefaultLocalAddress( acl::CSockAddr& aoLocalAddress );
    
    /**
     * Binds to the default local address and start to accept incoming
     * connections.
     *
     * @return If successful, shall return zero; otherwise -1 shall be returned, and apl_errno indicate the error.
     */
    virtual apl_int_t Bind(void);

    /**
     * Unbinds from the default local address and stop to accept incoming
     * connections.  All managed connections will be closed if
     * {@link #SetCloseOnDeactivation(bool) disconnectOnUnbind} property is
     * <tt>true</tt>.  This method returns silently if the default local
     * address is not bound yet.
     */
    virtual apl_int_t Unbind(void);
    
    /**
     * Binds to the specified local address and start to accept incoming
     * connections.
     *
     * @return If successful, shall return zero; otherwise -1 shall be returned, and apl_errno indicate the error.
     */
    apl_int_t Bind( acl::CSockAddr const& aoAddress );

    /**
     * Unbinds from the specified local address and stop to accept incoming
     * connections.
     *
     * @return If successful, shall return zero; otherwise -1 shall be returned, and apl_errno indicate the error.
     */
    apl_int_t Unbind( acl::CSockAddr const& aoAddress );

protected:
    virtual apl_int_t Bind0( acl::CSockAddr const& aoAddress ) = 0;

    virtual apl_int_t Unbind0( acl::CSockAddr const& aoAddress ) = 0;
        
private:
    acl::CSockAddr moLocalAddress;
};

ANF_NAMESPACE_END

#endif//ANF_IOACCEPTOR_H
