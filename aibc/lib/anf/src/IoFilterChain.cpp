
#include "anf/IoFilterChain.h"
#include "anf/IoHandler.h"
#include "anf/IoService.h"
#include "anf/IoProcessor.h"

ANF_NAMESPACE_START

////////////////////////////////////////////////////////////////////////////////////////////////
class CHeadFilter : public CIoFilter
{
public:
    virtual void FilterWrite(
        IoNextFilter* apoNextFilter,
        SessionPtrType& aoSession,
        CWriteFuture::PointerType& aoWriteFuture )
    {
        if (aoSession->IsClosing())
        {
            aoWriteFuture->Exception(E_SOCK_SEND, APL_EPIPE);//session already closed
            return;
        }

        apl_size_t liMaxQueue = aoSession->GetService()->GetConfiguration()->GetMaxQueue();

        if (liMaxQueue > 0 && aoSession->GetWriteRequestQueue().Size() >= liMaxQueue)//queue full
        {
            acl::CTimeValue loQueueTimeout = aoSession->GetService()->GetConfiguration()->GetQueueTimeout();

            if( loQueueTimeout.Nsec() <= 0 )
            {
                aoWriteFuture->Exception(E_QUEUE_FULL, 0);//return, no wait queue timeout
  
                apl_errprintf("Session(%s:%"APL_PRIdINT" -> %s:%"APL_PRIdINT") "
                    "AnfQueueFull(Current = %"APL_PRIuINT", Capacity = %"APL_PRIuINT", TimeWait = No!)\n",
                    aoSession->GetLocalAddress().GetIpAddr(), aoSession->GetLocalAddress().GetPort(),
                    aoSession->GetRemoteAddress().GetIpAddr(), aoSession->GetRemoteAddress().GetPort(),
                    aoSession->GetWriteRequestQueue().Size(), liMaxQueue);
                    
                return;
            }          

            acl::CTimestamp loExpired(acl::CTimestamp::PRC_NSEC);
            loExpired += loQueueTimeout;

            acl::TSmartLock<acl::CLock> loGuard(aoSession->GetWriteRequestLock());

            if( aoSession->GetWriteRequestCond().Wait(aoSession->GetWriteRequestLock(), loExpired) !=0 )
            {
                aoWriteFuture->Exception(E_QUEUE_FULL, 0);//queue timeout
  
                apl_errprintf("Session(%s:%"APL_PRIdINT" -> %s:%"APL_PRIdINT") "
                    "AnfQueueFull(Current = %"APL_PRIuINT", Capacity = %"APL_PRIuINT", TimeWait = Yes(%"APL_PRIdINT"ms))\n",
                    aoSession->GetLocalAddress().GetIpAddr(), aoSession->GetLocalAddress().GetPort(),
                    aoSession->GetRemoteAddress().GetIpAddr(), aoSession->GetRemoteAddress().GetPort(),
                    aoSession->GetWriteRequestQueue().Size(), liMaxQueue, loQueueTimeout.Msec());
				
                return;
            }

            if (aoSession->IsClosing())
            {
                aoWriteFuture->Exception(E_SOCK_SEND, APL_EPIPE);//session already closed
                return;
            }
        }

        apl_size_t liSize = aoSession->GetWriteRequestQueue().Push(aoWriteFuture);

        if (aoSession->IsWriteSuspended() && liSize == 1)
        {
            aoSession->GetProcessor()->Flush(aoSession);
        }
        return;
    }
   
    virtual void FilterClose(
        IoNextFilter* apoNextFilter,
        SessionPtrType& aoSession )
    {
        aoSession->GetProcessor()->Remove(aoSession);
    }
    
    virtual CIoFilter* Clone(void)
    {
        ACL_ASSERT(false);
        return NULL;
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////
class CTailFilter : public CIoFilter
{
public:
    virtual void SessionOpened( IoNextFilter* apoNextFilter, SessionPtrType& aoSession )
    {
        aoSession->GetHandler()->SessionOpened(aoSession);
    }

    virtual void SessionClosed( IoNextFilter* apoNextFilter, SessionPtrType& aoSession )
    {
        aoSession->GetHandler()->SessionClosed(aoSession);
    }

    virtual void SessionIdle( IoNextFilter* apoNextFilter, SessionPtrType& aoSession, apl_int_t aiStatus)
    {
        aoSession->GetHandler()->SessionIdle(aoSession, aiStatus);
    }
    
    //virtual void Exception( IoNextFilter* apoNextFilter, CIoService* apoService, CThrowable const& aoCause )
    //{
    //    apoService->GetHandler()->Exception(aoCause);
    //}
    
    virtual void SessionException(
        IoNextFilter* apoNextFilter,
        SessionPtrType& aoSession,
        CThrowable const& aoCause )
    {
        aoSession->GetHandler()->SessionException(aoSession, aoCause);
    }

    virtual void MessageReceived(
        IoNextFilter* apoNextFilter,
        SessionPtrType& aoSession,
        acl::CAny const& aoMessage )
    {
        aoSession->GetHandler()->MessageReceived(aoSession, aoMessage);
    }

    virtual void MessageFinished(
        IoNextFilter* apoNextFilter,
        SessionPtrType& aoSession,
        CWriteFuture::PointerType& aoWriteFuture )
    {
        aoSession->GetHandler()->MessageFinished(aoSession, aoWriteFuture);
    }

    virtual void FilterWrite(
        IoNextFilter* apoNextFilter,
        SessionPtrType& aoSession,
        CWriteFuture::PointerType& aoWriteFuture )
    {
        apoNextFilter->FilterWrite(aoSession, aoWriteFuture);
    }

    virtual void FilterClose( IoNextFilter* apoNextFilter, SessionPtrType aoSession )
    {
        apoNextFilter->FilterClose(aoSession);
    }
    
    virtual CIoFilter* Clone(void)
    {
        ACL_ASSERT(false);
        return NULL;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CIoFilterChain::CEntry::CEntry( char const* apcName, CIoFilter* apoFilter )
    : mpoFilter(apoFilter)
{
    ACL_RING_ELEM_INIT(this, moLink);
    apl_strncpy(this->macName, apcName, sizeof(this->macName) );
}

CIoFilterChain::CIoFilterChain(void)
{
    CIoFilter* lpoFilter = NULL;
    CEntry*    lpoEntry = NULL;
    
    ACL_RING_INIT(&this->moHead, CEntry, moLink);
    
    //add head filter
    ACL_NEW_ASSERT(lpoFilter, CHeadFilter);
    ACL_NEW_ASSERT(lpoEntry, CEntry("Head", lpoFilter) );
    ACL_RING_INSERT_HEAD(&this->moHead, lpoEntry, CEntry, moLink);
    
    //add tail filter
    ACL_NEW_ASSERT(lpoFilter, CTailFilter);
    ACL_NEW_ASSERT(lpoEntry, CEntry("Tail", lpoFilter) );
    ACL_RING_INSERT_TAIL(&this->moHead, lpoEntry, CEntry, moLink);
}

CIoFilterChain::~CIoFilterChain(void)
{
    CEntry* lpoCurr = NULL;
    CEntry* lpoNext = NULL;
    
    ACL_RING_FOREACH_SAFE(lpoCurr, lpoNext, &this->moHead, CEntry, moLink)
    {
        lpoCurr->mpoFilter->Destroy();

        ACL_DELETE(lpoCurr->mpoFilter);
        ACL_DELETE(lpoCurr);
    }
}

void CIoFilterChain::AddFirst( char const* apcName, CIoFilter* apoFilter )
{
    CEntry*    lpoEntry  = NULL;
    CIoFilter* lpoFilter = apoFilter;

    lpoFilter->Init();

    //add head filter
    ACL_NEW_ASSERT(lpoEntry, CEntry(apcName, lpoFilter) );
    ACL_RING_INSERT_AFTER( ACL_RING_FIRST(&this->moHead), lpoEntry, moLink);
}

void CIoFilterChain::AddLast( char const* apcName, CIoFilter* apoFilter )
{
    CEntry*    lpoEntry  = NULL;
    CIoFilter* lpoFilter = apoFilter;

    lpoFilter->Init();
    
    //add tail filter
    ACL_NEW_ASSERT(lpoEntry, CEntry(apcName, lpoFilter) );
    ACL_RING_INSERT_BEFORE( ACL_RING_LAST(&this->moHead), lpoEntry, moLink);
}

void CIoFilterChain::CopyTo( CIoFilterChain* apoDest )
{
    CEntry* lpoFirst = ACL_RING_FIRST(&this->moHead);
    CEntry* lpoLast = ACL_RING_LAST(&this->moHead);
    
    for (lpoFirst = ACL_RING_NEXT(lpoFirst, moLink);
         lpoFirst != lpoLast;
         lpoFirst = ACL_RING_NEXT(lpoFirst, moLink))
    {
        apoDest->AddLast(lpoFirst->macName, lpoFirst->mpoFilter->Clone() );
    }
}

void CIoFilterChain::FireSessionOpened( SessionPtrType& aoSession )
{
    CIoFilter* lpoFilter = ACL_RING_FIRST(&this->moHead)->mpoFilter;
    IoNextFilter* lpoNextFilter = ACL_RING_FIRST(&this->moHead);
    
    lpoFilter->SessionOpened(lpoNextFilter, aoSession);
}

void CIoFilterChain::FireSessionClosed( SessionPtrType& aoSession )
{
    CIoFilter* lpoFilter = ACL_RING_FIRST(&this->moHead)->mpoFilter;
    IoNextFilter* lpoNextFilter = ACL_RING_FIRST(&this->moHead);
    
    lpoFilter->SessionClosed(lpoNextFilter, aoSession);
}

void CIoFilterChain::FireSessionIdle( SessionPtrType& aoSession, apl_int_t aiStatus )
{
    CIoFilter* lpoFilter = ACL_RING_FIRST(&this->moHead)->mpoFilter;
    IoNextFilter* lpoNextFilter = ACL_RING_FIRST(&this->moHead);
    
    lpoFilter->SessionIdle(lpoNextFilter, aoSession, aiStatus);
}

void CIoFilterChain::FireMessageReceived( SessionPtrType& aoSession, acl::CAny const& aoMessage )
{
    CIoFilter* lpoFilter = ACL_RING_FIRST(&this->moHead)->mpoFilter;
    IoNextFilter* lpoNextFilter = ACL_RING_FIRST(&this->moHead);
    
    lpoFilter->MessageReceived(lpoNextFilter, aoSession, aoMessage);
}

void CIoFilterChain::FireMessageFinished( SessionPtrType& aoSession, CWriteFuture::PointerType& aoWriteFuture )
{
    CIoFilter* lpoFilter = ACL_RING_FIRST(&this->moHead)->mpoFilter;
    IoNextFilter* lpoNextFilter = ACL_RING_FIRST(&this->moHead);
    
    lpoFilter->MessageFinished(lpoNextFilter, aoSession, aoWriteFuture);
}

//void CIoFilterChain::FireException( CIoService* apoService, CThrowable const& aoCause )
//{
//    CIoFilter* lpoFilter = ACL_RING_FIRST(&this->moHead)->mpoFilter;
//    IoNextFilter* lpoNextFilter = ACL_RING_FIRST(&this->moHead);
//    
//    lpoFilter->Exception(lpoNextFilter, apoService, aoCause);
//}
    
void CIoFilterChain::FireSessionException( SessionPtrType& aoSession, CThrowable const& aoCause )
{
    CIoFilter* lpoFilter = ACL_RING_FIRST(&this->moHead)->mpoFilter;
    IoNextFilter* lpoNextFilter = ACL_RING_FIRST(&this->moHead);
    
    lpoFilter->SessionException(lpoNextFilter, aoSession, aoCause);
}

void CIoFilterChain::FireFilterWrite( SessionPtrType& aoSession, CWriteFuture::PointerType& aoWriteFuture )
{
    CIoFilter* lpoFilter = ACL_RING_LAST(&this->moHead)->mpoFilter;
    IoNextFilter* lpoNextFilter = ACL_RING_LAST(&this->moHead);
    
    lpoFilter->FilterWrite(lpoNextFilter, aoSession, aoWriteFuture);
}

void CIoFilterChain::FireFilterClose( SessionPtrType& aoSession )
{
    CIoFilter* lpoFilter = ACL_RING_LAST(&this->moHead)->mpoFilter;
    IoNextFilter* lpoNextFilter = ACL_RING_LAST(&this->moHead);
    
    lpoFilter->FilterClose(lpoNextFilter, aoSession);
}

ANF_NAMESPACE_END

