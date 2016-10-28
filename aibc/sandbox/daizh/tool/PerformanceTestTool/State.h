
#ifndef __STATE_H__
#define __STATE_H__

#include "AIMapFile.h"

typedef long long int64;

struct stState
{
    stState() : 
        ciCurrConnCnt(0),
        ciTotalErrConnCnt(0),
        ciTotalRevCnt(0),
        ciTotalErrRevCnt(0),
        ciTotalSendCnt(0),
        ciTotalErrSendCnt(0),
        ciTotalRequestCnt(0),
        ciTotalErrRequestCnt(0)
    {
    }
    
    int64 ciCurrConnCnt;
    int64 ciTotalErrConnCnt;
    int64 ciTotalRevCnt;
    int64 ciTotalErrRevCnt;
    int64 ciTotalSendCnt;
    int64 ciTotalErrSendCnt;
    int64 ciTotalRequestCnt;
    int64 ciTotalErrRequestCnt;
};

class clsAtomOp
{
public:
    
};

class clsMPShareState
{
public:
    int CreateShareState( int aiCount )
    {
        stState* lpoState = new stState[aiCount];
        
        AIFile loFile;
        
        if ( loFile.Open( "state.mf", O_CREAT | O_RDWR | O_TRUNC, 0600 ) != 0 )
        {
            return -1;
        }
        
        if ( (size_t)loFile.Write( lpoState, sizeof(stState) * aiCount ) != sizeof(stState) * aiCount )
        {
            return -1;
        }
        
        return 0;
    }
    
    int OpenShareState()
    {
        AIFile loFile;
        
        if ( this->coMapFile.Map( "state.mf", PROT_READ | PROT_WRITE, MAP_SHARED ) != 0 )
        {
            return -1;
        }
        
        return 0;
    }
    
    stState& operator [] ( int aiN )
    {
        return static_cast<stState*>(this->coMapFile.GetAddr())[aiN];
    }
    
    void Clear( stState& ao )
    {
        AISmartLock loLock(coLock);
        memset( &ao, 0, sizeof(ao) );
    }
    
    void Increase( int64& aiVal )
    {
        AISmartLock loLock(coLock);
        aiVal++;
    }
    void Decrease( int64& aiVal )
    {
        AISmartLock loLock(coLock);
        aiVal--;
    }
    
    bool IsOpen()
    {
        return this->coMapFile.IsMap();
    }
    
protected:
    AIMutexLock coLock;
    AIMapFile coMapFile;
};

void PrintState( int aiCount )
{
    static clsMPShareState loMPShareState;
    static stState loPrev;
    static AITimeMeter loTimeMeter;
    stState loState;
    
    if ( !loMPShareState.IsOpen() )
    {
        if ( loMPShareState.OpenShareState() != 0 )
        {
            printf( "ERROR : Open share state fail\n" );
            return;
        }
    }
    
    loTimeMeter.Snapshot();
    
    for ( int i = 0; i < aiCount; i++ )
    {
        loState.ciCurrConnCnt     += loMPShareState[i].ciCurrConnCnt;
        loState.ciTotalErrConnCnt += loMPShareState[i].ciTotalErrConnCnt;
        loState.ciTotalRevCnt     += loMPShareState[i].ciTotalRevCnt;
        loState.ciTotalErrRevCnt  += loMPShareState[i].ciTotalErrRevCnt;
        loState.ciTotalSendCnt    += loMPShareState[i].ciTotalSendCnt;
        loState.ciTotalErrSendCnt += loMPShareState[i].ciTotalErrSendCnt;
        loState.ciTotalRequestCnt += loMPShareState[i].ciTotalRequestCnt;
        loState.ciTotalErrRequestCnt += loMPShareState[i].ciTotalErrRequestCnt;
    }

    ::printf( 
        "====================================================\n"
        "   CurrConnCnt   = %lld\n"
        "   ErrConnCnt    = %lld\n"
        "   RevCnt        = %lld\n"
        "   RevCnt        = %lf/s\n"
        "   ErrRevCnt     = %lld\n"
        "   SendCnt       = %lld\n"
        "   ErrSendCnt    = %lld\n"
        "   RequestCnt    = %lld\n"
        "   ErrRequestCnt = %lld\n"
        "====================================================\n",
        loState.ciCurrConnCnt,
        loState.ciTotalErrConnCnt - loPrev.ciTotalErrConnCnt,
        loState.ciTotalRevCnt - loPrev.ciTotalRevCnt,
        (loState.ciTotalRevCnt - loPrev.ciTotalRevCnt)/loTimeMeter.GetTime(),
        loState.ciTotalErrRevCnt - loPrev.ciTotalErrRevCnt,
        loState.ciTotalSendCnt - loPrev.ciTotalSendCnt,
        loState.ciTotalErrSendCnt - loPrev.ciTotalErrSendCnt,
        loState.ciTotalRequestCnt - loPrev.ciTotalRequestCnt,
        loState.ciTotalErrRequestCnt - loPrev.ciTotalErrRequestCnt );
    loTimeMeter.Reset();
    loPrev = loState;
}

#endif //__STATE_H__
