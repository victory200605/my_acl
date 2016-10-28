
#ifndef __FREQUENCY_H__
#define __FREQUENCY_H__

class clsFrequency
{
public:
    clsFrequency() : ciFrequency(0), ciCounter(0)
    {
    }
    void Set(int aiFrequency )
    {
        ciFrequency = aiFrequency;
        ciCounter   = aiFrequency;
    }
    int GetFrequency()
    {
        return ciFrequency;
    }
    int Get()
    {        
        int liDelayTime = 0;
        do
        {
            if ( ciFrequency == -1 ) break;
                
            if ( ciCounter-- == ciFrequency )
            {
                gettimeofday( &coTimevl, NULL );
            }
            else
            {
                struct timeval loTimevl;
                gettimeofday( &loTimevl, NULL );
                liDelayTime = 1000000 * ( loTimevl.tv_sec - coTimevl.tv_sec ) + ( loTimevl.tv_usec - coTimevl.tv_usec );
                
            }
            if ( liDelayTime < 0 || liDelayTime / 1000000 > 1 )
            {
                ciCounter = ciFrequency;
                break;
            }
            
            if ( ciCounter <= 0 )
            {
                AISleepFor( AI_TIME_USEC * ( 1000000 - liDelayTime ) );
                ciCounter = ciFrequency;
            }
        }
        while( false );
        
        return ciCounter;
    }
protected:
    int ciFrequency;
    int ciCounter;
    struct timeval coTimevl;
};

#endif //
