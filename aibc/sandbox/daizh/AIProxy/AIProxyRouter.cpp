
#include "AISynch.h"
#include "AITypeObject.h"
#include "AIProxyUtility.h"
#include "AIProxyRouter.h"
#include "AIProxyServer.h"

START_AIPROXY_NAMESPACE

///////////////////////////////////// Msg Table ///////////////////////////////////////
#define S_STR( str ) (str==NULL ? "": str)
////msg item
clsSequenceNumItem::clsSequenceNumItem( const char* apcSender, uint32_t aiInnerSeqNum, uint32_t aiOuterSeqNum ) :
    ciInnerSeqNum(aiInnerSeqNum),
    ciOuterSeqNum(aiOuterSeqNum),
    ciTimestamp( time(NULL) )
{
    ::StringCopy( this->csSender, S_STR(apcSender), sizeof(this->csSender) );
}
#undef S_STR

bool clsSequenceNumTable::Insert( uint32_t aiKey, const TItem& aoItem )
{
    {
        ///save msgid map
        AISmartLock loLock( this->coMapLock );
        this->coMap.insert( TMap::value_type(aiKey, aoItem) );
        return true;
    }
}

bool clsSequenceNumTable::Get( uint32_t aiKey, TItem& aoItem )
{
    {
        AISmartLock loLock( this->coMapLock );
        TMap::iterator loIter = this->coMap.find(aiKey);
        if ( loIter != this->coMap.end() )
        {///
            aoItem = loIter->second;
            this->coMap.erase(loIter);
            return true;
        }
        else
        {
            return false;
        }
    }
}

void clsSequenceNumTable::ClearTimeOut( int aiTimeOut )
{
    int liTimestamp = time(NULL);
    
    {
        AISmartLock loLock( this->coMapLock );
        TMap::iterator loIter = this->coMap.begin();
        for ( ; loIter != this->coMap.end(); loIter++ )
        {///
            if ( liTimestamp - loIter->second.GetTimestamp() > aiTimeOut )
            {
                this->coMap.erase( loIter );
            }
            else
            {
                break;
            }
        }
    }
}

///////////////////////////////////// routing ///////////////////////////////////////
int AIDefaultInnerRouting::Error( int aiErrno, const char* apcSender, const char* apcData, size_t aiSize )
{
    AI_PROXY_ERROR( "Input data fail, may be client disconnected, [Sender=%s]/[Inner Routing]", apcSender );
    return 0;
}

int AIDefaultInnerRouting::Route( const char* apcData, size_t aiSize, char* apcAddressee, size_t aiBufferSize )
{
    AI_PROXY_ERROR( "Default Inner Routing unprocessed, may be request timeout, [DataSize=%d][Inner Routing]", aiSize );
    return AI_ERROR_ROUTING;
}

///////////////////////////////////// singleton router ///////////////////////////////////////
AIRouter::AIRouter() : 
    cpoInnerServer(NULL),
    cpoOuterServer(NULL),
    cpoInnerRouting(NULL),
    cpoOuterRouting(NULL),
    ciOuterSequenceNum(0),
    ciInnerSequenceNum(0)
{}

AIRouter* AIRouter::Instance()
{
    static AIRouter* slpoRouter = new AIRouter;
    return slpoRouter;
}

int AIRouter::Initialize()
{
    return 0;
}

bool AIRouter::PutToInputTable( uint32_t aiKey, const TSeqNumItem& aoItem )
{
    return this->coInnerSeqNumTable.Insert( aiKey, aoItem );
}

bool AIRouter::GetFromInputTable( uint32_t aiKey, TSeqNumItem& aoItem )
{
    return this->coInnerSeqNumTable.Get( aiKey, aoItem );
}

bool AIRouter::PutToOutputTable( uint32_t aiKey, const TSeqNumItem& aoItem )
{
    return this->coOuterSeqNumTable.Insert( aiKey, aoItem );
}

bool AIRouter::GetFromOutputTable( uint32_t aiKey, TSeqNumItem& aoItem )
{
    return this->coOuterSeqNumTable.Get( aiKey, aoItem );
}

int AIRouter::Input( const char* apcSender, const char* apcData, size_t aiSize )
{
    int       liRetCode = 0;
    char      lsAddressee[AI_MAX_NAME_LEN]   = {0};
    uint32_t  liInnerSeqNum = 0;
    uint32_t  liOuterSeqNum = 0;
    AIChunkEx loBuffer(0);
    AIChunkEx loTraceBuffer(0);
    AIProtocol*         lpoInnerProtocol = this->cpoInnerServer->GetProtocol();
    AIProtocol*         lpoOuterProtocol = this->cpoOuterServer->GetProtocol();
    AIProtocol::THeader loInnerHeader;
    AIProtocol::THeader loOuterHeader;
        
    AI_PROXY_TRACE( "Trace Input data, [Size=%d]/[Package=\n%s]", aiSize, UStringToHex( apcData, aiSize, loTraceBuffer ) );
       
    do
    {
        if ( lpoOuterProtocol->Decode( apcData, aiSize, loOuterHeader ) != 0 )
        {
            AI_PROXY_ERROR( "Input invalid data and decode fail, [Router]/[Size=%d]/[Package=\n%s]", 
                aiSize, UStringToHex( apcData, aiSize, loTraceBuffer ) );
                
            liRetCode = AI_ERROR_PTL_DECODE;
            break;
        }

        if ( loOuterHeader.GetType() == AIProtocol::RESPONSE )
        {
            AIRouter::TSeqNumItem loItem;
            if ( this->GetFromOutputTable( loOuterHeader.GetSequenceNum(), loItem ) )
            {
                ::snprintf( lsAddressee, sizeof(lsAddressee), "%s", loItem.GetSender() );
                liInnerSeqNum = loItem.GetInnerSeqNum();
                liOuterSeqNum = loItem.GetOuterSeqNum();
            }
            else
            {
                //ERROR timeout
                liRetCode = AI_ERROR_TIMEOUT;
                break;
            }
            
            loOuterHeader.SetSequenceNum(liOuterSeqNum);
            
            AIChunkEx loTmpBuffer(0);
            if ( ( liRetCode = lpoOuterProtocol->Encode( loOuterHeader, loTmpBuffer ) ) != 0 )
            {
                liRetCode = AI_ERROR_PTL_ENCODE;
                break;
            }
            
            loInnerHeader.SetSequenceNum(liInnerSeqNum);
            loInnerHeader.SetType(AIProtocol::RESPONSE);
            loInnerHeader.SetLength(loTmpBuffer.GetSize());
            loInnerHeader.SetDataPtr(loTmpBuffer.BasePtr());
            
            if ( ( liRetCode = lpoInnerProtocol->Encode( loInnerHeader, loBuffer ) ) != 0 )
            {
                liRetCode = AI_ERROR_PTL_ENCODE;
                break;
            }
        }
        else
        {
            liInnerSeqNum = ciInnerSequenceNum++;
            liOuterSeqNum = loOuterHeader.GetSequenceNum();
            
            if ( ( liRetCode = this->cpoInnerRouting->Route( apcData, aiSize, lsAddressee, sizeof(lsAddressee) ) ) != 0 )
            {
                break;
            }

            loInnerHeader.SetSequenceNum( liInnerSeqNum );
            loInnerHeader.SetType( AIProtocol::REQUEST );
            loInnerHeader.SetLength( loOuterHeader.GetLength() );
            loInnerHeader.SetDataPtr( loOuterHeader.GetDataPtr() );
            
            if ( ( liRetCode = lpoInnerProtocol->Encode( loInnerHeader, loBuffer ) ) != 0 )
            {
                liRetCode = AI_ERROR_PTL_ENCODE;
                break;
            }
            
            AIRouter::TSeqNumItem loItem( apcSender, liInnerSeqNum, liOuterSeqNum );
            if ( !this->PutToInputTable( liInnerSeqNum, loItem ) )
            {
                liRetCode = AI_ERROR_MSGID_DUAL;
                break;
            }
        }

        AI_PROXY_TRACE( "Input from %s to %s", apcSender, lsAddressee );
        
        if ( ( liRetCode = this->cpoInnerServer->Send( lsAddressee, loBuffer.BasePtr(), loBuffer.GetSize() ) ) != 0 )
        {
            break;
        }
            
        return 0;
        
    }while(false);
    
    this->cpoInnerRouting->Error( liRetCode, apcSender, apcData, aiSize );
    
    return liRetCode;
}

int AIRouter::Output( const char* apcSender, const char* apcData, size_t aiSize )
{
    int       liRetCode = 0;
    char      lsAddressee[AI_MAX_NAME_LEN]   = {0};
    uint32_t  liInnerSeqNum = 0;
    uint32_t  liOuterSeqNum = 0;
    uint32_t  liOldOuterSeqNum = 0;
    AIChunkEx loBuffer(0);
    AIChunkEx loTraceBuffer(0);
    AIProtocol::THeader loInnerHeader;
    AIProtocol::THeader loOuterHeader;
    AIProtocol*         lpoInnerProtocol = this->cpoInnerServer->GetProtocol();
    AIProtocol*         lpoOuterProtocol = this->cpoOuterServer->GetProtocol();
    
    AI_PROXY_TRACE( "Trace Output data, [Size=%d]/[Package=\n%s]", aiSize, UStringToHex( apcData, aiSize, loTraceBuffer ) );
    
    do
    {
        if ( lpoInnerProtocol->Decode( apcData, aiSize, loInnerHeader ) != 0 )
        {
            AI_PROXY_ERROR( "Output invalid data and decode fail, [Router]/[Size=%d]/[Package=\n%s]", 
                aiSize, UStringToHex( apcData, aiSize, loTraceBuffer ) );
            liRetCode = AI_ERROR_PTL_DECODE;
            break;
        }

        if ( loInnerHeader.GetType() == AIProtocol::RESPONSE )
        {
            AIRouter::TSeqNumItem loItem;
            if ( this->GetFromInputTable( loInnerHeader.GetSequenceNum(), loItem ) )
            {
                ::snprintf( lsAddressee, sizeof(lsAddressee), "%s", loItem.GetSender() );
                liInnerSeqNum = loItem.GetInnerSeqNum();
                liOuterSeqNum = loItem.GetOuterSeqNum();
            }
            else
            {
                //ERROR timeout
                liRetCode = AI_ERROR_TIMEOUT;
                break;
            }
            
            loOuterHeader.SetSequenceNum( liOuterSeqNum );
            loOuterHeader.SetType( AIProtocol::RESPONSE );
            loOuterHeader.SetLength( loInnerHeader.GetLength() );
            loOuterHeader.SetDataPtr( loInnerHeader.GetDataPtr() );
            
            if ( ( liRetCode = lpoOuterProtocol->Encode( loOuterHeader, loBuffer ) ) != 0 )
            {
                liRetCode = AI_ERROR_PTL_ENCODE;
                break;
            }
        }
        else
        {
            liInnerSeqNum = loInnerHeader.GetSequenceNum();
            liOuterSeqNum = ciOuterSequenceNum++;
            
            if ( ( liRetCode = this->cpoOuterRouting->Route( apcData, aiSize, lsAddressee, sizeof(lsAddressee) ) ) != 0 )
            {
                break;
            }
            
            if ( ( liRetCode = lpoOuterProtocol->Decode( 
                loInnerHeader.GetDataPtr(), loInnerHeader.GetLength(), loOuterHeader ) ) != 0 )
            {
                liRetCode = AI_ERROR_PTL_DECODE;
                break;
            }
            
            liOldOuterSeqNum = loOuterHeader.GetSequenceNum();
            loOuterHeader.SetSequenceNum( liOuterSeqNum );
            loOuterHeader.SetType( AIProtocol::REQUEST );
            
            if ( ( liRetCode = lpoOuterProtocol->Encode( loOuterHeader, loBuffer ) ) != 0 )
            {
                liRetCode = AI_ERROR_PTL_ENCODE;
                break;
            }
            
            AIRouter::TSeqNumItem loItem( apcSender, liInnerSeqNum, liOldOuterSeqNum );
            if ( !AIRouter::Instance()->PutToOutputTable( liOuterSeqNum, loItem ) )
            {
                liRetCode = AI_ERROR_MSGID_DUAL;
                break;
            }
        }

        AI_PROXY_TRACE( "Output from %s to %s", apcSender, lsAddressee );

        if ( ( liRetCode = this->cpoOuterServer->Send( lsAddressee, loBuffer.BasePtr(), loBuffer.GetSize() ) ) != 0 )
        {
            break;
        }
            
        return 0;
        
    }while(false);
    
    this->cpoOuterRouting->Error( liRetCode, apcSender, apcData, aiSize );
        
    return liRetCode;
}
    
void* AIRouter::TimeOutHandleThread( void* apvParam )
{
    AIRouter* lpoRouter = static_cast<AIRouter*>(apvParam);
    
    pthread_detach(pthread_self());
    
    while( true )
    {
        AISleepFor( AI_TIME_SEC * 100 );
        lpoRouter->coInnerSeqNumTable.ClearTimeOut( 100 );
        lpoRouter->coOuterSeqNumTable.ClearTimeOut( 100 );
    }
    
    return NULL;
}

////////////////////////////////////////// Global interface ////////////////////////////////////////
AIRouter* GetRouter()      { return AIRouter::Instance(); }
AIServer* GetOuterServer() { return AIRouter::Instance()->OuterServer(); }
AIServer* GetInnerServer() { return AIRouter::Instance()->InnerServer(); }

END_AIPROXY_NAMESPACE
