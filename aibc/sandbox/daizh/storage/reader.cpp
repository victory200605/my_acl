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
    CSegmentQueue loStorage;

#if 0
    loMetadata.SetSegmentCount(1000);
    loMetadata.SetSegmentCapacity(10000);
    loMetadata.SetFileCount(10);
    loMetadata.SetFilePageCount(10000);
    loMetadata.SetPageSize(16*1024);

    loStorage.Create("qdb", loMetadata);
#endif
    
    if (loStorage.Open("qdb", "shm", 16*1024*10) != 0)
    {
        apl_errprintf("Open storage fail\n");
        
        return -1;
    }
    
    std::string loData;

    apl_int_t liSegmentID = -1;
    START_LOOP(liCount)
    {
        if (__liN % 10000 == 0)
        {
            liSegmentID++;
        }

        apl_int_t liRetCode = loStorage.Get(liSegmentID, "QueueName", 0, loData);
        if (liRetCode != 0)
        {
            printf("SegmentID = %"APL_PRIdINT", %"APL_PRIdINT"\n", liSegmentID, __liN);
            ACL_ASSERT(false);
        }
    }
    END_LOOP();
        
    loStorage.Close();

    return 0;
}
