
#include "gfq2/Utility.h"
#include "acl/GetOpt.h"
#include "acl/SString.h"
#include "acl/stl/string.h"
#include "vlr/VLRSegmentQueue.h"
#include "flr/FLRSegmentQueue.h"

using namespace gfq2;

void PrintUsage(void)
{
    printf("Usage:\n");
    printf("    -t --type=[vlr|flr] : storage type vlr(var-lenght-record)|flr(fixed-length-record)\n");
    printf("    -d --path=<path> : storage path\n");
    printf("    -q --queue=<queue name [| queue name] > : specified queue name\n");
    printf("    -h --help\n");
}

int main( int argc, char* argv[] )
{
    acl::CGetOpt loOpt(argc, argv, "t:d:q:h");
    loOpt.LongOption("type", 't', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("path", 'd', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("queue", 'q', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("help", 'h', acl::CGetOpt::NO_ARG);

    apl_int_t liOpt = 0;
    std::string loType;
    std::string loPath;
    acl::CSpliter loSpliter("|");

    while( (liOpt = loOpt() ) != -1)
    {
        switch(liOpt)
        {
            case 't': loType = loOpt.OptArg(); break;
            case 'd': loPath = loOpt.OptArg(); break;
            case 'q':
            {
                loSpliter.Parse(loOpt.OptArg() );

                break;
            }
            default: PrintUsage(); return 0;
        };
    }

    if (loPath.length() == 0)
    {
        PrintUsage();

        return 0;
    }

    apl_int_t liRetCode = 0;
    ISegmentQueue* lpoSegmentQueue = APL_NULL;

    if (loType == "vlr")
    {
        ACL_NEW_ASSERT(lpoSegmentQueue, CVLRSegmentQueue(loPath.c_str(), "fim", 1024*1024) );
    }
    else if (loType == "flr")
    {
        ACL_NEW_ASSERT(lpoSegmentQueue, CFLRSegmentQueue(loPath.c_str() ) );
    }
    else
    {
        apl_errprintf("Unknown type\n");
        
        return -1;
    }

    if ( (liRetCode = lpoSegmentQueue->Open() ) != 0)
    {
        apl_errprintf("Open storage fail, (Path=%s,RetCode=%"APL_PRIdINT")\n", loPath.c_str(), liRetCode );
    
        ACL_DELETE(lpoSegmentQueue);
        
        return -1;
    }

    if (loSpliter.GetSize() > 0)
    {
        for (apl_size_t luN = 0; luN < loSpliter.GetSize(); luN++)
        {
            if (lpoSegmentQueue->Clear(loSpliter.GetField(luN) ) == 0)
            {
                printf("Storage clear queue %s success\n", loSpliter.GetField(luN) );
            }
            else
            {
                apl_errprintf("Storage clear queue %s fail\n", loSpliter.GetField(luN) );
            }
        }
    }
    else
    {
        if (lpoSegmentQueue->ClearAll() == 0)
        {
            printf("Storage clear all queue success\n");
        }
        else
        {
            apl_errprintf("Storage clear all queue fail\n");
        }
    }

    lpoSegmentQueue->Close();

    ACL_DELETE(lpoSegmentQueue);

    return 0;
}
