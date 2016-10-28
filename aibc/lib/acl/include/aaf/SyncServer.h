
#ifndef AAF_SYNCSERVER_H
#define AAF_SYNCSERVER_H

#include "aaf/Server.h"
#include "acl/FileLog.h"
#include "acl/Reactor.h"
#include "acl/Synch.h"
#include "acl/SockHandle.h"
#include "acl/Singleton.h"
#include "acl/ThreadManager.h"

AAF_NAMESPACE_START

class CSyncServer : public IServer, public acl::IEventHandler
{
public:
    //empty base
    class CEmpty {};

    //empty type define
    typedef CEmpty EmptyType;
    typedef void (CEmpty::*MFuncType)(void);
        
    //request node
    struct CRequest
    {
        EmptyType* mpoBase;
        MFuncType  mpfFunc;
        unsigned char mcMask;
    };
    
public:
    CSyncServer(void);
    
    virtual ~CSyncServer(void);
    
    virtual apl_int_t Startup( char const* apcConfigFileName );
    
    virtual apl_int_t Restart(void);
    
    virtual apl_int_t Shutdown(void);
    
    void SetReactor( acl::CReactor* apoReactor );
    
    template<typename T, typename FuncType>
    apl_int_t Sync( T* apoObj, FuncType apfMFunc )
    {
        return this->Request(reinterpret_cast<EmptyType*>(apoObj), reinterpret_cast<MFuncType>(apfMFunc));
    }

protected:
    apl_int_t Request( EmptyType* apoBase, MFuncType apfMFunc );
    
    virtual apl_int_t HandleInput( apl_handle_t aiHandle );
    
    virtual apl_int_t HandleOutput( apl_handle_t aiHandle );

private:
    acl::CReactor* mpoReactor;
    apl_int_t      miEvents;
    
    acl::CSockHandlePair moHandlePair;
        
    acl::CMemoryBlock moBuffer;
        
    CRequest moRequest;
    
    acl::CLock moLock;
};

AAF_NAMESPACE_END

#endif //AAF_SYNCSERVER_H
