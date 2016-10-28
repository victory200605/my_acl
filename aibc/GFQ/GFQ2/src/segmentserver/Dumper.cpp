
#include "gfq2/Utility.h"
#include "acl/GetOpt.h"
#include "acl/SString.h"
#include "acl/stl/string.h"
#include "vlr/VLRSegmentQueue.h"
#include "flr/FLRSegmentQueue.h"
#include "aaf/LogService.h"

using namespace gfq2;

void PrintUsage(void)
{
    printf("Usage:\n");
    printf("    -t --type=[vlr|flr] : storage type vlr(var-lenght-record)|flr(fixed-length-record)\n");
    printf("    -d --path=<path> : storage path\n");
    printf("    -h --help\n");
}

int main( int argc, char* argv[] )
{
    acl::CGetOpt loOpt(argc, argv, "t:d:q:h");
    loOpt.LongOption("type", 't', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("path", 'd', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("help", 'h', acl::CGetOpt::NO_ARG);

    apl_int_t liOpt = 0;
    std::string loType;
    std::string loPath;

    while( (liOpt = loOpt() ) != -1)
    {
        switch(liOpt)
        {
            case 't': loType = loOpt.OptArg(); break;
            case 'd': loPath = loOpt.OptArg(); break;
            default: PrintUsage(); return 0;
        };
    }

    if (loPath.length() == 0)
    {
        PrintUsage();

        return 0;
    }

    if (loType == "vlr")
    {
        CVLRStorage loStorage;

        //Open storage
        if (loStorage.Open(loPath.c_str() ) != 0)
        {
            apl_errprintf("VLR storage open fail\n");

            return -1;
        }

        //Dump storage info
        printf("%s\n", "-------------------------------------------------------------------------------------");
        printf("|-%-40s :%-40s|\n", "version", loStorage.GetVersion() );
        printf("|-%-40s :%-40s|\n", "type", loStorage.GetType() );
        printf("|-%-40s :%-40"APL_PRIuINT"|\n", "segment-count", loStorage.GetSegmentCount() );
        printf("|-%-40s :%-40"APL_PRIuINT"|\n", "segment-capacity", loStorage.GetSegmentCapacity() );
        printf("|-%-40s :%-40"APL_PRIuINT"|\n", "file-count", loStorage.GetFileCount() );
        printf("|-%-40s :%-40"APL_PRIuINT"|\n", "page-count", loStorage.GetPageCount() );
        printf("|-%-40s :%-40"APL_PRIuINT"|\n", "total-page-count", loStorage.GetPageCount() * loStorage.GetFileCount() );
        printf("|-%-40s :%-40"APL_PRIuINT"|\n", "page-size", loStorage.GetPageSize() );
        printf("|-%-40s :%-40s|\n", "create-time", loStorage.GetCreateTime() );
        printf("%s\n", "-------------------------------------------------------------------------------------");

        loStorage.Close();
    }
    else if (loType == "flr")
    {
        CFLRStorage loStorage;

        //Open storage
        if (loStorage.Open(loPath.c_str() ) != 0)
        {
            apl_errprintf("VLR storage open fail\n");

            return -1;
        }

        //Dump info
        printf("%s\n", "-------------------------------------------------------------------------------------");
        printf("|-%-40s :%-40s|\n", "version", loStorage.GetVersion() );
        printf("|-%-40s :%-40s|\n", "type", loStorage.GetType() );
        printf("|-%-40s :%-40"APL_PRIuINT"|\n", "slot-capacity", loStorage.GetSlotCapacity() );
        printf("|-%-40s :%-40"APL_PRIuINT"|\n", "segment-count", loStorage.GetSegmentCount() );
        printf("|-%-40s :%-40"APL_PRIuINT"|\n", "segment-capacity", loStorage.GetSegmentCapacity() );
        printf("|-%-40s :%-40"APL_PRIuINT"|\n", "file-count", loStorage.GetFileCount() );
        printf("|-%-40s :%-40s|\n", "create-time", loStorage.GetCreateTime() );
        printf("%s\n", "-------------------------------------------------------------------------------------");
        
        loStorage.Close();
    }
    else
    {
        apl_errprintf("Unknown type\n");
        
        return -1;
    }
    
    return 0;
}
