
#include "gfq2/Utility.h"
#include "acl/GetOpt.h"
#include "vlr/VLRSegmentQueue.h"
#include "flr/FLRSegmentQueue.h"

using namespace gfq2;

void PrintUsage(void)
{
    printf("Usage:\n");
    printf("    -t --type=[vlr|flr] : storage type vlr(var-lenght-record)|flr(fixed-length-record)\n");
    printf("    -d --path=<path> : storage path\n");
    printf("    -c --segment-capacity=<num> : segment capacity of record\n");
    printf("    -n --segment-count=<num> : segment count in single storage file\n");
    printf("    -f --file-count=<num> : storage file count\n");
    printf("    -h --help\n");
    printf("--type=vlr special option\n");
    printf("    -x --file-page-count=<num> : page count in the one file only\n");
    printf("    -p --page-size=<num [k|m]> : page size, exp. -p 16k\n");
    printf("--type=flr special option\n");
    printf("    -s --slot-capacity=<num> : segment slot capacity and storage record length can't over it\n");
}

int main( int argc, char* argv[] )
{
    acl::CGetOpt loOpt(argc, argv, "t:d:c:n:f:x:p:s:h");
    loOpt.LongOption("type", 't', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("path", 'd', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("slot-capacity", 's', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("segment-capacity", 'c', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("segment-count", 'n', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("file-count", 'f', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("file-page-count", 'x', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("page-size", 'p', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("help", 'h', acl::CGetOpt::NO_ARG);

    apl_int_t liOpt = 0;
    apl_int_t liRetCode = 0;
    std::string loType;
    std::string loPath;
    apl_int_t liSlotCapacity = 0;
    apl_int_t liSegmentCapacity = 0;
    apl_int_t liSegmentCount = 0;
    apl_int_t liFileCount = 0;
    apl_int_t liFilePageCount = 0;
    apl_int_t liPageSize = 0;

    while( (liOpt = loOpt() ) != -1)
    {
        switch(liOpt)
        {
            case 't': loType = loOpt.OptArg(); break;
            case 'd': loPath = loOpt.OptArg(); break;
            case 's': liSlotCapacity = apl_strtoi32(loOpt.OptArg(), APL_NULL, 10); break;
            case 'c': liSegmentCapacity = apl_strtoi32(loOpt.OptArg(), APL_NULL, 10); break;
            case 'n': liSegmentCount = apl_strtoi32(loOpt.OptArg(), APL_NULL, 10); break;
            case 'f': liFileCount = apl_strtoi32(loOpt.OptArg(), APL_NULL, 10); break;
            case 'x': liFilePageCount = apl_strtoi32(loOpt.OptArg(), APL_NULL, 10); break;
            case 'p':
            {
                char* lpcNext = APL_NULL;
                liPageSize = apl_strtoi32(loOpt.OptArg(), &lpcNext, 10);
                if (lpcNext != APL_NULL && lpcNext[0] == 'k')
                {
                    liPageSize *= 1024;
                }
                else if(lpcNext != APL_NULL && lpcNext[0] == 'm')
                {
                    liPageSize *= (1024 * 1024);
                }

                break;
            }
            default: PrintUsage(); return 0;
        };
    }

    if ( loPath.size() == 0
        || liSegmentCapacity == 0
        || liSegmentCount == 0
        || (loType == "vlr" && (liFileCount == 0 || liFilePageCount == 0 || liPageSize == 0) ) )
    {
        PrintUsage();
        return 0;
    }
    
    if (loType == "vlr")
    {
        printf("%s\n", "-------------------------------------------------------------------------------------");
        printf("|-%-40s :%-40s|\n", "storage type", "vlr");
        printf("|-%-40s :%-40s|\n", "storage path", loPath.c_str() );
        printf("|-%-40s :%-40"APL_PRIdINT"|\n", "segment-capacity", liSegmentCapacity);
        printf("|-%-40s :%-40"APL_PRIdINT"|\n", "segment-count", liSegmentCount);
        printf("|-%-40s :%-40"APL_PRIdINT"|\n", "file-count", liFileCount);
        printf("|-%-40s :%-40"APL_PRIdINT"|\n", "file-page-count", liFilePageCount);
        printf("|-%-40s :%-40"APL_PRIdINT"|\n", "page-size", liPageSize);
        printf("%s\n", "-------------------------------------------------------------------------------------");

        liRetCode = CVLRSegmentQueue::Create(
            loPath.c_str(), 
            liSegmentCapacity,
            liSegmentCount,
            liFileCount,
            liFilePageCount,
            liPageSize );
    }
    else if (loType == "flr")
    {
        printf("%s\n", "-------------------------------------------------------------------------------------");
        printf("|-%-40s :%-40s|\n", "storage type", "flr");
        printf("|-%-40s :%-40s|\n", "storage path", loPath.c_str() );
        printf("|-%-40s :%-40"APL_PRIdINT"|\n", "slot-capacity", liSlotCapacity);
        printf("|-%-40s :%-40"APL_PRIdINT"|\n", "segment-capacity", liSegmentCapacity);
        printf("|-%-40s :%-40"APL_PRIdINT"|\n", "segment-count", liSegmentCount);
        printf("|-%-40s :%-40"APL_PRIdINT"|\n", "file-count", liFileCount);
        printf("%s\n", "-------------------------------------------------------------------------------------");

        liRetCode = CFLRSegmentQueue::Create(
            loPath.c_str(),
            liSlotCapacity,
            liSegmentCapacity,
            liSegmentCount,
            liFileCount );
    }
    else
    {
        apl_errprintf("storage type unimplemented\n");
    }

    if (liRetCode == 0)
    {
        printf("storage create success\n");
    }
    else
    {
        apl_errprintf("Storage create fail (RetCode=%"APL_PRIdINT")\n", liRetCode);
    }

    return 0;
}
