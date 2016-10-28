
#include "gfq2/Utility.h"
#include "acl/GetOpt.h"
#include "vlr/VLRSegmentQueue.h"

using namespace gfq2;

void PrintUsage(void)
{
    printf("Usage:\n");
    printf("    -d --path=<path> : storage path\n");
    printf("    -c --check       : storage check\n");
    printf("    -h --help\n");
}

int main( int argc, char* argv[] )
{
    acl::CGetOpt loOpt(argc, argv, "d:c:h");
    loOpt.LongOption("path", 'd', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("check", 'c', acl::CGetOpt::NO_ARG);
    loOpt.LongOption("help", 'h', acl::CGetOpt::NO_ARG);

    apl_int_t liOpt = 0;
    std::string loPath;
    bool lbIsCheck = false;

    while( (liOpt = loOpt() ) != -1)
    {
        switch(liOpt)
        {
            case 'd': loPath = loOpt.OptArg(); break;
            case 'c': lbIsCheck = true; break;
            default: PrintUsage(); return 0;
        };
    }

    if ( loPath.size() == 0)
    {
        PrintUsage();
        return 0;
    }

    CVLRSegmentQueue loSegmentQueue(loPath.c_str(), "fim", 1024*1024);

    if (loSegmentQueue.Open() != 0)
    {
        apl_errprintf("Open storage fail\n");
    }

    if (lbIsCheck)
    {
        loSegmentQueue.Check();
    }
    
    if (loSegmentQueue.Recover() != 0)
    {
        apl_errprintf("Recover storage fail\n");
    }

    loSegmentQueue.Close();

    return 0;
}
