#include "SegmentQueue.h"
#include "acl/Timestamp.h"
#include "acl/Number.h"
#include "acl/ThreadManager.h"

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

CSegmentQueue goStorage;
acl::TNumber<apl_size_t, acl::CLock> goWriteCount;
acl::TNumber<apl_size_t, acl::CLock> goReadCount;
apl_int_t giCount = 1000000;

void* writer( void* apvParam )
{
    apl_int_t liRetCode = 0;
    char lacBuffer[1024];

    for (apl_int_t liSegmentID = 0; liSegmentID < giCount; liSegmentID++)
    //for (apl_int_t liSegmentID = 0; ; liSegmentID++)
    {
        liSegmentID = liSegmentID % 1000;

        //apl_errprintf("write segment id = %"APL_PRIdINT"\n", liSegmentID);
        
        goStorage.Allocate(liSegmentID, "QueueName", apl_time() );

        for (apl_int_t liN = 0; liN < 10000; )
        {
            apl_snprintf(lacBuffer, sizeof(lacBuffer), "QWERTYUIOASDFGHJKZXCVBNM%"APL_PRIuINT, liN);
            
            if ( (liRetCode = goStorage.Put(liSegmentID, "QueueName", lacBuffer, sizeof(lacBuffer) ) ) == 0)
            {
                goWriteCount++;
                liN++;
            }
            else
            {
                apl_errprintf("Put error = %"APL_PRIdINT"\n", liRetCode);
            }
        }
    }

    return APL_NULL;
}

void* reader( void* apvParam )
{
    std::string loData;

    for (apl_int_t liSegmentID = 0; liSegmentID < giCount; liSegmentID++)
    //for (apl_int_t liSegmentID = 0; ; liSegmentID++)
    {
        liSegmentID = liSegmentID % 1000;
        
        //apl_errprintf("read segment id = %"APL_PRIdINT"\n", liSegmentID);
        
        for (apl_int_t liN = 0; liN < 10000; )
        {
            if (goStorage.Get(liSegmentID, "QueueName", 0, loData) == 0)
            {
                goReadCount++;
                liN++;
            }
        }
    }

    return APL_NULL;
}

void* snapshot( void* )
{
    apl_size_t luPrevWriteCount = 0;
    apl_size_t luPrevReadCount = 0;

    while(true)
    {
        apl_size_t luCurrWriteCount = goWriteCount;
        apl_size_t luCurrReadCount = goReadCount;

        apl_errprintf("------------------------------------------------------------\n");
        apl_errprintf("write count = %"APL_PRIuINT"\n", luCurrWriteCount - luPrevWriteCount);
        apl_errprintf("read count = %"APL_PRIuINT"\n", luCurrReadCount - luPrevReadCount);

        luPrevWriteCount = luCurrWriteCount;
        luPrevReadCount = luCurrReadCount;

        apl_sleep(APL_TIME_SEC);
    }

    return APL_NULL;
}

int main(int argc, char* argv[])
{
    CStorageMetadata loMetadata;

    if (*argv[1] == '1')
    {
        loMetadata.Set(METADATA_SEGMENT_COUNT, 1000);
        loMetadata.Set(METADATA_SEGMENT_CAPACITY, 10000);
        loMetadata.Set(METADATA_FILE_COUNT, 10);
        loMetadata.Set(METADATA_PAGE_COUNT, 10000);
        loMetadata.Set(METADATA_PAGE_SIZE, 16*1024);

        goStorage.Create("qdb", loMetadata);
    }
    
    if (goStorage.Open("qdb", "shm", 16*1024*1000) != 0)
    {
        apl_errprintf("Open storage fail\n");
        
        return -1;
    }

    acl::CThreadManager loManager;

    //writer(APL_NULL);
    //reader(APL_NULL);
    loManager.Spawn(writer, APL_NULL);
    loManager.Spawn(reader, APL_NULL);
    loManager.Spawn(snapshot, APL_NULL);

    getchar();

    return 0;
}
