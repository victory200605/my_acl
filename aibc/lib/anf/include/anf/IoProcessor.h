
#ifndef ANF_IOPROCESSOR_H
#define ANF_IOPROCESSOR_H

#include "anf/Utility.h"

ANF_NAMESPACE_START

class CIoConfiguration;

class IoHandler;

class IoProcessor
{
public:
    virtual ~IoProcessor(void) {};

    /**
     * Set the Configuration which provides I/O service to this processor
     */
    virtual void SetConfiguration( CIoConfiguration* apoConfiguration ) = 0;
    
    /**
     * Set the Handler which provides I/O service to this processor
     */
    virtual void SetHandler( IoHandler* apoHandler ) = 0;
    
    /**
     * Dispose the processor and releases all related resources
     */
    virtual apl_int_t Dispose( bool abIsWait = true ) = 0;
    
    /**
     * Adds the specified session to the I/O processor so that
     * the I/O processor starts to perform any I/O operations related
     * with the session.
     */
    virtual apl_int_t Add( SessionPtrType& aoSession ) = 0;

    /**
     * Flushes the internal write request queue of the specified session.
     */
    virtual apl_int_t Flush( SessionPtrType& aoSession ) = 0;

    /**
     * Controls the traffic of the specified session 
     */
    virtual apl_int_t UpdateTrafficControl( SessionPtrType& aoSession ) = 0;

    /**
     * Removes and closes the specified session from the I/O
     * processor so that the I/O processor closes the connection
     * associated with the session and releases any other related
     * resources.
     */
    virtual apl_int_t Remove( SessionPtrType& aoSession ) = 0;

    /** 
     * Return session size of processor
     */
    virtual apl_size_t GetSize(void) = 0;
};

ANF_NAMESPACE_END

#endif//ANF_IOPROCESSOR_H
