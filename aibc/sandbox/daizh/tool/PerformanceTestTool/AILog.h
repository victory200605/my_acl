
#ifndef __AI_LOG_H__
#define __AI_LOG_H__

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

class AILog
{
public:
    enum { INFO = 0, ERROR, DEBUG };
    
public:
    static AILog* Instance()
    {
        static AILog* slpoLog = new AILog;
        return slpoLog;
    }
    
    ~AILog()
    {
        fclose( cpFile );
    }
    
    void SetLevel( int aiLevel )           { ciLevel = aiLevel; }
    
    int Write( int aiLevel, const char* apcFormat, ... )
    {
        //Disable
        if ( aiLevel > ciLevel ) return 0;
        
        coFileLock.Lock();
        coMutexLock.Lock();

        static char slcBuff[4096];
        char    *lpcCurPtr = NULL;

        ::GetTimeString( slcBuff, 0, 1 );
        lpcCurPtr  = slcBuff + strlen(slcBuff);
        *lpcCurPtr++ = ':';

        va_list vl;
        va_start(vl, apcFormat);

        lpcCurPtr = lpcCurPtr + vsprintf( lpcCurPtr, apcFormat, vl );

        va_end(vl);
        if( ( lpcCurPtr-slcBuff ) > 0 && *( lpcCurPtr -1 ) != '\n' )
        {
           *lpcCurPtr++ = '\n';
           *lpcCurPtr = '\0';
        }
        fprintf( cpFile, "%s", slcBuff );
        fflush( cpFile );

        coMutexLock.Unlock();
        coFileLock.Unlock();
        
        return 0;
    }
    
protected:
    AILog() : coFileLock("__lock"), ciLevel(1)
    {
        cpFile = fopen( "syslog", "a+" );
        assert( cpFile != NULL );
    }
    
protected:
    AIFileLock  coFileLock;
    AIMutexLock coMutexLock;
    FILE*       cpFile;
    int         ciLevel;
};

#define AILOG_INFO( format, ... ) AILog::Instance()->Write( AILog::INFO, "INFO: " format, __VA_ARGS__ );
#define AILOG_ERROR( format, ... ) AILog::Instance()->Write( AILog::ERROR, "ERROR: " format, __VA_ARGS__ );
#define AILOG_DEBUG( format, ... ) AILog::Instance()->Write( AILog::DEBUG, "DEBUG: " format, __VA_ARGS__ );
    
#endif
