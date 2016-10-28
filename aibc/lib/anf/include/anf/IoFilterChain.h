
#ifndef ANF_IoFilterChain_H
#define ANF_IoFilterChain_H

#include "acl/Any.h"
#include "acl/Ring.h"
#include "anf/IoFilter.h"

ANF_NAMESPACE_START

class CIoService;

class CIoFilterChain
{
public:
    CIoFilterChain(void);
    
    virtual ~CIoFilterChain(void);
    
    /**
     * Adds the specified filter with the specified name at the beginning of this chain.
     */
    void AddFirst( char const* apcName, CIoFilter* apoFilter );

    /**
     * Adds the specified filter with the specified name at the end of this chain.
     */
    void AddLast( char const* apcName, CIoFilter* apoFilter );
    
    /**
     * Copy the all filter to the specified filter chain.
     */
    void CopyTo( CIoFilterChain* apoDest );
    
    /**
     * Fires a IoHandler#SessionOpened(...) event.
     */
    void FireSessionOpened( SessionPtrType& aoSession );

    /**
     * Fires a IoHandler#SessionClosed(...) event.
     */
    void FireSessionClosed( SessionPtrType& aoSession );

    /**
     * Fires a IoHandler#SessionIdle(...) event.
     */
    void FireSessionIdle( SessionPtrType& aoSession, apl_int_t aiStatus );

    /**
     * Fires a IoHandler#MessageReceived(...) event.
     */
    void FireMessageReceived( SessionPtrType& aoSession, acl::CAny const& aoMessage );

    /**
     * Fires a IoHandler#MessageFinished(...) event.
     */
    void FireMessageFinished( SessionPtrType& aoSession, CWriteFuture::PointerType& aoWriteFuture );

    /**
     * Fires a IoHandler#Exception(...) event.
     */
    void FireException( CIoService* apoService, CThrowable const& aoCause );
    
    /**
     * Fires a IoHandler#SessionException(...) event.
     */
    void FireSessionException( SessionPtrType& aoSession, CThrowable const& aoCause );

    /**
     * Fires a CIoSession#Write(...) event.
     */
    void FireFilterWrite( SessionPtrType& aoSession, CWriteFuture::PointerType& aoWriteFuture );

    /**
     * Fires a CIoSession#Close(...) event.
     */
    void FireFilterClose( SessionPtrType& aoSession );
    
private:
    struct CEntry : public IoNextFilter
    {
        CEntry( char const* apcName, CIoFilter* apoFilter );
        
        char macName[32];
        
        CIoFilter* mpoFilter;
        
        ACL_RING_ENTRY(CEntry) moLink;
        
        virtual void SessionOpened( SessionPtrType& aoSession )
        {
            CIoFilter*    lpoFilter = ACL_RING_NEXT(this, moLink)->mpoFilter;
            IoNextFilter* lpoNextFilter = ACL_RING_NEXT(this, moLink);
            lpoFilter->SessionOpened(lpoNextFilter, aoSession);
        }
        
        virtual void SessionClosed( SessionPtrType& aoSession )
        {
            CIoFilter*    lpoFilter = ACL_RING_NEXT(this, moLink)->mpoFilter;
            IoNextFilter* lpoNextFilter = ACL_RING_NEXT(this, moLink);
            lpoFilter->SessionClosed(lpoNextFilter, aoSession);
        }
        
        virtual void SessionIdle( SessionPtrType& aoSession, apl_int_t aiStatus )
        {
            CIoFilter*    lpoFilter = ACL_RING_NEXT(this, moLink)->mpoFilter;
            IoNextFilter* lpoNextFilter = ACL_RING_NEXT(this, moLink);
            lpoFilter->SessionIdle(lpoNextFilter, aoSession, aiStatus);
        }
        
        //virtual void Exception( CIoService* apoService, CThrowable const& aoCause )
        //{
        //    CIoFilter*    lpoFilter = ACL_RING_NEXT(this, moLink)->mpoFilter;
        //    IoNextFilter* lpoNextFilter = ACL_RING_NEXT(this, moLink);
        //    lpoFilter->Exception(lpoNextFilter, apoService, aoCause);
        //}
        
        virtual void SessionException( SessionPtrType& aoSession, CThrowable const& aoCause )
        {
            CIoFilter*    lpoFilter = ACL_RING_NEXT(this, moLink)->mpoFilter;
            IoNextFilter* lpoNextFilter = ACL_RING_NEXT(this, moLink);
            lpoFilter->SessionException(lpoNextFilter, aoSession, aoCause);
        }
        
        virtual void MessageReceived( SessionPtrType& aoSession, acl::CAny const& aoMessage )
        {
            CIoFilter*    lpoFilter = ACL_RING_NEXT(this, moLink)->mpoFilter;
            IoNextFilter* lpoNextFilter = ACL_RING_NEXT(this, moLink);
            lpoFilter->MessageReceived(lpoNextFilter, aoSession, aoMessage);
        }

        virtual void MessageFinished( SessionPtrType& aoSession, CWriteFuture::PointerType& aoWriteFuture )
        {
            CIoFilter*    lpoFilter = ACL_RING_NEXT(this, moLink)->mpoFilter;
            IoNextFilter* lpoNextFilter = ACL_RING_NEXT(this, moLink);
            lpoFilter->MessageFinished(lpoNextFilter, aoSession, aoWriteFuture);
        }
        
        virtual void FilterWrite( SessionPtrType& aoSession, CWriteFuture::PointerType& aoWriteFuture )
        {
            CIoFilter*    lpoFilter = ACL_RING_PREV(this, moLink)->mpoFilter;
            IoNextFilter* lpoNextFilter = ACL_RING_PREV(this, moLink);
            lpoFilter->FilterWrite(lpoNextFilter, aoSession, aoWriteFuture);
        }
        
        virtual void FilterClose( SessionPtrType& aoSession )
        {
            CIoFilter*    lpoFilter = ACL_RING_PREV(this, moLink)->mpoFilter;
            IoNextFilter* lpoNextFilter = ACL_RING_PREV(this, moLink);
            lpoFilter->FilterClose(lpoNextFilter, aoSession);
        }
    };
    
    ACL_RING_HEAD(MyHead, CEntry) moHead;
};

ANF_NAMESPACE_END

#endif //ANF_IoFilterChain_H
