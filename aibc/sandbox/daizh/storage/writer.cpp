#include "SegmentQueue.h"
#include "acl/Timestamp.h"

using namespace gfq2;

#define START_LOOP( count ) \
{ \
    acl::CTimestamp __loStart, __loEnd; \
    __loStart.Update(acl::CTimestamp::PRC_USEC); \
    apl_int_t __liCount = count; \
    for( apl_int_t __liN = 0; __liN < __liCount; __liN++ )

#define END_LOOP() \
    __loEnd.Update(acl::CTimestamp::PRC_USEC); \
    double __ldUseTime = double( __loEnd.Msec() - __loStart.Msec() ) / 1000; \
    printf("Use time=%lf, %lf/pre\n", __ldUseTime, double(__liCount) / __ldUseTime); \
}

int main(int argc, char* argv[])
{
    apl_int_t liCount = apl_strtoi32(argv[1], APL_NULL, 10);
    CStorageMetadata loMetadata;
    CSegmentQueue loStorage;

#if 0
    loMetadata.Set(METADATA_SEGMENT_COUNT, 1000);
    loMetadata.Set(METADATA_SEGMENT_CAPACITY, 10000);
    loMetadata.Set(METADATA_FILE_COUNT, 10);
    loMetadata.Set(METADATA_PAGE_COUNT, 10000);
    loMetadata.Set(METADATA_PAGE_SIZE, 16*1024);
    loMetadata.Set(METADATA_CACHE_SIZE, 16*1024*1000);

    loStorage.Create("qdb", loMetadata);
#endif
    
    if (loStorage.Open("qdb", "shm", 16*1024*10) != 0)
    {
        apl_errprintf("Open storage fail\n");
        
        return -1;
    }

    char lacBuffer[1024];

    //loStorage.Clear();
    
    //for (apl_int_t liN = 0; liN < liCount; liN++)
    apl_int_t liSegmentID = -1;
    START_LOOP(liCount)
    {
        apl_snprintf(lacBuffer, sizeof(lacBuffer), "QWERTYUIOASDFGHJKZXCVBNM%"APL_PRIuINT, __liN);

        if (__liN % 10000 == 0)
        {
            liSegmentID++;

            loStorage.Allocate(liSegmentID, "QueueName", apl_time() );
        }

        loStorage.Put(liSegmentID, "QueueName", lacBuffer, sizeof(lacBuffer) );
    }
    END_LOOP();

    loStorage.Close();

    return 0;
}
