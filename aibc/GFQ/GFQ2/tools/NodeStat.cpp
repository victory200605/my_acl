
#include "acl/GetOpt.h"
#include "acl/SString.h"
#include "gfq2/GFQueueClient.h"

void PrintUsage(void)
{
    printf("Usage:\n");
    printf("    -i --ip=<ip address>: master server ip address\n");
    printf("    -p --port=<port> : master server port\n");
    printf("    -s --servernode=<servernode [|servernode ...]> : server node name\n");
    printf("    -n --interval=<second> : display interval\n");
    printf("    -c --count=<num> : display count\n");
    printf("    -f --field=<field [|field ...] > : display field config, option value:\n");
    printf("       name     :server node name\n");
    printf("       group    :server node attaching group\n");
    printf("       type     :server node storage type\n");
    printf("       server   :server node server ip and port\n");
    printf("       conn     :server node connection num\n");
    printf("       fsegment :server node free segment count\n");
    printf("       usegment :server node used segment count\n");
    printf("       rwtimes  :server node read/write times\n");
    printf("       fpage    :server node free page count\n");
    printf("       upage    :server node used page count\n");
    printf("       cfpage   :server node cache free page count\n");
    printf("       cupage   :server node cache used page count\n");
    printf("       crtimes  :server node cache read times\n");
    printf("       cwtimes  :server node cache write times\n");
    printf("       crmtimes :server node cache read missed times\n");
    printf("       cwmtimes :server node cache write missed times\n");
    printf("       cstimes  :server node cache sync times\n");
    printf("    -h --help\n");
}

apl_int_t    giPort = 0;
apl_int_t    giInterval = 0;
apl_int_t    giCount = 0;
std::string  goIpAddress("127.0.0.1");

/////////////////////////////////////////////////////////////////////////////////////////////////
//start field output define
class CFieldOutput
{
public:
    CFieldOutput(void)
        : mpoNext(APL_NULL)
    {
    }

    CFieldOutput( CFieldOutput* apoNext )
        : mpoNext(apoNext)
    {
    }

    virtual ~CFieldOutput(void)
    {
        ACL_DELETE(this->mpoNext);
    }

    void SetNext( CFieldOutput* apoNext )
    {
        this->mpoNext = apoNext;
    }

    CFieldOutput* GetNext(void)
    {
        return this->mpoNext;
    }

    virtual void WriteSeparatorLine(void) = 0;

    virtual void WriteTitle(void) = 0;

    virtual void WriteContext( gfq2::CServerNodeInfo& aoInfo ) = 0;
        
private:
    CFieldOutput* mpoNext;
};

#define DECLEAR_CONSTRUCTOR(name) \
    name(void) {} \
    name( CFieldOutput* apoNext ) : CFieldOutput(apoNext) {}

class CFieldStart : public CFieldOutput
{
public:
    DECLEAR_CONSTRUCTOR(CFieldStart);

    void WriteSeparatorLine(void)
    {
        printf("-");

        this->GetNext()->WriteSeparatorLine();
    }

    void WriteTitle(void)
    {
        printf("|");
        
        this->GetNext()->WriteTitle();
    }

    void WriteContext( gfq2::CServerNodeInfo& aoInfo )
    {
        printf("|");
        
        this->GetNext()->WriteContext(aoInfo);
    }
};

class CFieldName : public CFieldOutput
{
public:
    DECLEAR_CONSTRUCTOR(CFieldName);

    void WriteSeparatorLine(void)
    {
        std::string loLine(21, '-');

        printf("%s", loLine.c_str() );

        this->GetNext()->WriteSeparatorLine();
    }

    void WriteTitle(void)
    {
        printf("%-20s|", "name");
        
        this->GetNext()->WriteTitle();
    }

    void WriteContext( gfq2::CServerNodeInfo& aoInfo )
    {
        printf("%-20s|", aoInfo.GetServerNodeName() );
        
        this->GetNext()->WriteContext(aoInfo);
    }
};

class CFieldGroup : public CFieldOutput
{
public:
    DECLEAR_CONSTRUCTOR(CFieldGroup);

    void WriteSeparatorLine(void)
    {
        std::string loLine(21, '-');

        printf("%s", loLine.c_str() );

        this->GetNext()->WriteSeparatorLine();
    }

    void WriteTitle(void)
    {
        printf("%-20s|", "group");
        
        this->GetNext()->WriteTitle();
    }

    void WriteContext( gfq2::CServerNodeInfo& aoInfo )
    {
        printf("%-20s|", aoInfo.GetGroupName() );
        
        this->GetNext()->WriteContext(aoInfo);
    }
};

class CFieldType : public CFieldOutput
{
public:
    DECLEAR_CONSTRUCTOR(CFieldType);

    void WriteSeparatorLine(void)
    {
        std::string loLine(11, '-');

        printf("%s", loLine.c_str() );

        this->GetNext()->WriteSeparatorLine();
    }

    void WriteTitle(void)
    {
        printf("%-10s|", "type");
        
        this->GetNext()->WriteTitle();
    }

    void WriteContext( gfq2::CServerNodeInfo& aoInfo )
    {
        printf("%-10s|", aoInfo.GetStorageType() );
        
        this->GetNext()->WriteContext(aoInfo);
    }
};

class CFieldServer : public CFieldOutput
{
public:
    DECLEAR_CONSTRUCTOR(CFieldServer);

    void WriteSeparatorLine(void)
    {
        std::string loLine(16, '-');

        printf("%s", loLine.c_str() );

        this->GetNext()->WriteSeparatorLine();
    }

    void WriteTitle(void)
    {
        printf("%-15s|", "server");
        
        this->GetNext()->WriteTitle();
    }

    void WriteContext( gfq2::CServerNodeInfo& aoInfo )
    {
        char lacTemp[64];
        
        apl_snprintf(lacTemp, sizeof(lacTemp), "%s:%"APL_PRIuINT, aoInfo.GetServerIp(), (apl_size_t)aoInfo.GetServerPort() );
        
        printf("%-15s|", lacTemp);
        
        this->GetNext()->WriteContext(aoInfo);
    }
};

class CFieldConn : public CFieldOutput
{
public:
    DECLEAR_CONSTRUCTOR(CFieldConn);

    void WriteSeparatorLine(void)
    {
        std::string loLine(11, '-');

        printf("%s", loLine.c_str() );

        this->GetNext()->WriteSeparatorLine();
    }

    void WriteTitle(void)
    {
        printf("%-10s|", "conn");
        
        this->GetNext()->WriteTitle();
    }

    void WriteContext( gfq2::CServerNodeInfo& aoInfo )
    {
        printf("%-10"APL_PRIuINT"|", (apl_size_t)aoInfo.GetConnectionCount() );
        
        this->GetNext()->WriteContext(aoInfo);
    }
};

class CFieldFSegment : public CFieldOutput
{
public:
    DECLEAR_CONSTRUCTOR(CFieldFSegment);

    void WriteSeparatorLine(void)
    {
        std::string loLine(11, '-');

        printf("%s", loLine.c_str() );

        this->GetNext()->WriteSeparatorLine();
    }

    void WriteTitle(void)
    {
        printf("%-10s|", "fsegment");
        
        this->GetNext()->WriteTitle();
    }

    void WriteContext( gfq2::CServerNodeInfo& aoInfo )
    {
        printf("%-10"APL_PRIuINT"|", (apl_size_t)aoInfo.GetFreeSegmentCount() );
        
        this->GetNext()->WriteContext(aoInfo);
    }
};

class CFieldUSegment : public CFieldOutput
{
public:
    DECLEAR_CONSTRUCTOR(CFieldUSegment);

    void WriteSeparatorLine(void)
    {
        std::string loLine(11, '-');

        printf("%s", loLine.c_str() );

        this->GetNext()->WriteSeparatorLine();
    }

    void WriteTitle(void)
    {
        printf("%-10s|", "usegment");
        
        this->GetNext()->WriteTitle();
    }

    void WriteContext( gfq2::CServerNodeInfo& aoInfo )
    {
        printf("%-10"APL_PRIuINT"|", (apl_size_t)aoInfo.GetUsedSegmentCount() );
        
        this->GetNext()->WriteContext(aoInfo);
    }
};

class CFieldRWTimes : public CFieldOutput
{
public:
    DECLEAR_CONSTRUCTOR(CFieldRWTimes);

    void WriteSeparatorLine(void)
    {
        std::string loLine(11, '-');

        printf("%s", loLine.c_str() );

        this->GetNext()->WriteSeparatorLine();
    }

    void WriteTitle(void)
    {
        printf("%-10s|", "rwtimes");
        
        this->GetNext()->WriteTitle();
    }

    void WriteContext( gfq2::CServerNodeInfo& aoInfo )
    {
        printf("%-10"APL_PRIuINT"|", (apl_size_t)(aoInfo.GetReadTimes() + aoInfo.GetWriteTimes() ) );
        
        this->GetNext()->WriteContext(aoInfo);
    }
};

class CFieldFPage : public CFieldOutput
{
public:
    DECLEAR_CONSTRUCTOR(CFieldFPage);

    void WriteSeparatorLine(void)
    {
        std::string loLine(11, '-');

        printf("%s", loLine.c_str() );

        this->GetNext()->WriteSeparatorLine();
    }

    void WriteTitle(void)
    {
        printf("%-10s|", "fpage");
        
        this->GetNext()->WriteTitle();
    }

    void WriteContext( gfq2::CServerNodeInfo& aoInfo )
    {
        printf("%-10"APL_PRIuINT"|", (apl_size_t)(aoInfo.GetFreePageCount() ) );
        
        this->GetNext()->WriteContext(aoInfo);
    }
};

class CFieldUPage : public CFieldOutput
{
public:
    DECLEAR_CONSTRUCTOR(CFieldUPage);

    void WriteSeparatorLine(void)
    {
        std::string loLine(11, '-');

        printf("%s", loLine.c_str() );

        this->GetNext()->WriteSeparatorLine();
    }

    void WriteTitle(void)
    {
        printf("%-10s|", "upage");
        
        this->GetNext()->WriteTitle();
    }

    void WriteContext( gfq2::CServerNodeInfo& aoInfo )
    {
        printf("%-10"APL_PRIuINT"|", (apl_size_t)(aoInfo.GetUsedPageCount() ) );
        
        this->GetNext()->WriteContext(aoInfo);
    }
};

class CFieldCFPage : public CFieldOutput
{
public:
    DECLEAR_CONSTRUCTOR(CFieldCFPage);

    void WriteSeparatorLine(void)
    {
        std::string loLine(11, '-');

        printf("%s", loLine.c_str() );

        this->GetNext()->WriteSeparatorLine();
    }

    void WriteTitle(void)
    {
        printf("%-10s|", "cfpage");
        
        this->GetNext()->WriteTitle();
    }

    void WriteContext( gfq2::CServerNodeInfo& aoInfo )
    {
        printf("%-10"APL_PRIuINT"|", (apl_size_t)(aoInfo.GetCacheFreePageCount() ) );
        
        this->GetNext()->WriteContext(aoInfo);
    }
};

class CFieldCUPage : public CFieldOutput
{
public:
    DECLEAR_CONSTRUCTOR(CFieldCUPage);

    void WriteSeparatorLine(void)
    {
        std::string loLine(11, '-');

        printf("%s", loLine.c_str() );

        this->GetNext()->WriteSeparatorLine();
    }

    void WriteTitle(void)
    {
        printf("%-10s|", "cupage");
        
        this->GetNext()->WriteTitle();
    }

    void WriteContext( gfq2::CServerNodeInfo& aoInfo )
    {
        printf("%-10"APL_PRIuINT"|", (apl_size_t)(aoInfo.GetCacheUsedPageCount() ) );
        
        this->GetNext()->WriteContext(aoInfo);
    }
};

class CFieldCRTimes : public CFieldOutput
{
public:
    DECLEAR_CONSTRUCTOR(CFieldCRTimes);

    void WriteSeparatorLine(void)
    {
        std::string loLine(11, '-');

        printf("%s", loLine.c_str() );

        this->GetNext()->WriteSeparatorLine();
    }

    void WriteTitle(void)
    {
        printf("%-10s|", "crtimes");
        
        this->GetNext()->WriteTitle();
    }

    void WriteContext( gfq2::CServerNodeInfo& aoInfo )
    {
        printf("%-10"APL_PRIuINT"|", (apl_size_t)(aoInfo.GetCacheReadTimes() ) );
        
        this->GetNext()->WriteContext(aoInfo);
    }
};

class CFieldCRMTimes : public CFieldOutput
{
public:
    DECLEAR_CONSTRUCTOR(CFieldCRMTimes);

    void WriteSeparatorLine(void)
    {
        std::string loLine(11, '-');

        printf("%s", loLine.c_str() );

        this->GetNext()->WriteSeparatorLine();
    }

    void WriteTitle(void)
    {
        printf("%-10s|", "crmtimes");
        
        this->GetNext()->WriteTitle();
    }

    void WriteContext( gfq2::CServerNodeInfo& aoInfo )
    {
        printf("%-10"APL_PRIuINT"|", (apl_size_t)(aoInfo.GetCacheReadMissedTimes() ) );
        
        this->GetNext()->WriteContext(aoInfo);
    }
};

class CFieldCWTimes : public CFieldOutput
{
public:
    DECLEAR_CONSTRUCTOR(CFieldCWTimes);

    void WriteSeparatorLine(void)
    {
        std::string loLine(11, '-');

        printf("%s", loLine.c_str() );

        this->GetNext()->WriteSeparatorLine();
    }

    void WriteTitle(void)
    {
        printf("%-10s|", "cwtimes");
        
        this->GetNext()->WriteTitle();
    }

    void WriteContext( gfq2::CServerNodeInfo& aoInfo )
    {
        printf("%-10"APL_PRIuINT"|", (apl_size_t)(aoInfo.GetCacheWriteTimes() ) );
        
        this->GetNext()->WriteContext(aoInfo);
    }
};

class CFieldCWMTimes : public CFieldOutput
{
public:
    DECLEAR_CONSTRUCTOR(CFieldCWMTimes);

    void WriteSeparatorLine(void)
    {
        std::string loLine(11, '-');

        printf("%s", loLine.c_str() );

        this->GetNext()->WriteSeparatorLine();
    }

    void WriteTitle(void)
    {
        printf("%-10s|", "cwmtimes");
        
        this->GetNext()->WriteTitle();
    }

    void WriteContext( gfq2::CServerNodeInfo& aoInfo )
    {
        printf("%-10"APL_PRIuINT"|", (apl_size_t)(aoInfo.GetCacheWriteMissedTimes() ) );
        
        this->GetNext()->WriteContext(aoInfo);
    }
};

class CFieldCSTimes : public CFieldOutput
{
public:
    DECLEAR_CONSTRUCTOR(CFieldCSTimes);

    void WriteSeparatorLine(void)
    {
        std::string loLine(11, '-');

        printf("%s", loLine.c_str() );

        this->GetNext()->WriteSeparatorLine();
    }

    void WriteTitle(void)
    {
        printf("%-10s|", "cstimes");
        
        this->GetNext()->WriteTitle();
    }

    void WriteContext( gfq2::CServerNodeInfo& aoInfo )
    {
        printf("%-10"APL_PRIuINT"|", (apl_size_t)(aoInfo.GetCacheSyncTimes() ) );
        
        this->GetNext()->WriteContext(aoInfo);
    }
};

class CFieldEnd : public CFieldOutput
{
public:
    void WriteSeparatorLine(void)
    {
        printf("\n");
    }

    void WriteTitle(void)
    {
        printf("\n");
    }

    void WriteContext( gfq2::CServerNodeInfo& aoInfo )
    {
        printf("\n");
    }
};

//end field output define
///////////////////////////////////////////////////////////////////////////////////////////////

int main( int argc, char* argv[] )
{
    std::vector<std::string> loServerNodeList;
    acl::CGetOpt loOpt(argc, argv, "i:p:s:n:c:f:h");
    loOpt.LongOption("ip", 'i', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("port", 'p', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("servernode", 's', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("interval", 'n', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("count", 'c', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("field", 'f', acl::CGetOpt::ARG_REQUIRED);
    loOpt.LongOption("help", 'h', acl::CGetOpt::ARG_REQUIRED);
    
    apl_int_t liOptChar = 0;
    apl_int_t liRetCode = 0;
    CFieldOutput* lpoOutput = APL_NULL;

    while( (liOptChar = loOpt()) != -1)
	{
		switch (liOptChar)
		{
            case 'i':
                goIpAddress = loOpt.OptArg();
                break;
            case 'p':
                giPort = apl_strtoi32(loOpt.OptArg(), APL_NULL, 10);
                break;
            case 's':
            {
                acl::CSpliter loSpliter("|");

                loSpliter.Parse(loOpt.OptArg() );

                for (apl_size_t luN = 0; luN < loSpliter.GetSize(); luN++)
                {
                    loServerNodeList.push_back(loSpliter.GetField(luN) );
                }

                break;
            }
            case 'f':
            {
                CFieldOutput* lpoLastOutput = lpoOutput = new CFieldStart;
                
                acl::CSpliter loSpliter("|");

                loSpliter.Parse(loOpt.OptArg() );

                for (apl_size_t luN = 0; luN < loSpliter.GetSize(); luN++)
                {
                    CFieldOutput* lpoNew = APL_NULL;

                    if (apl_strcmp(loSpliter.GetField(luN), "name") == 0)
                    {
                        lpoNew = new CFieldName;
                    }
                    else if (apl_strcmp(loSpliter.GetField(luN), "group") == 0)
                    {
                        lpoNew = new CFieldGroup;
                    }
                    else if (apl_strcmp(loSpliter.GetField(luN), "type") == 0)
                    {
                        lpoNew = new CFieldType;
                    }
                    else if (apl_strcmp(loSpliter.GetField(luN), "server") == 0)
                    {
                        lpoNew = new CFieldServer;
                    }
                    else if (apl_strcmp(loSpliter.GetField(luN), "conn") == 0)
                    {
                        lpoNew = new CFieldConn;
                    }
                    else if (apl_strcmp(loSpliter.GetField(luN), "fsegment") == 0)
                    {
                        lpoNew = new CFieldFSegment;
                    }
                    else if (apl_strcmp(loSpliter.GetField(luN), "usegment") == 0)
                    {
                        lpoNew = new CFieldUSegment;
                    }
                    else if (apl_strcmp(loSpliter.GetField(luN), "rwtimes") == 0)
                    {
                        lpoNew = new CFieldRWTimes;
                    }
                    else if (apl_strcmp(loSpliter.GetField(luN), "fpage") == 0)
                    {
                        lpoNew = new CFieldFPage;
                    }
                    else if (apl_strcmp(loSpliter.GetField(luN), "upage") == 0)
                    {
                        lpoNew = new CFieldUPage;
                    }
                    else if (apl_strcmp(loSpliter.GetField(luN), "cfpage") == 0)
                    {
                        lpoNew = new CFieldCFPage;
                    }
                    else if (apl_strcmp(loSpliter.GetField(luN), "cupage") == 0)
                    {
                        lpoNew = new CFieldCUPage;
                    }
                    else if (apl_strcmp(loSpliter.GetField(luN), "crtimes") == 0)
                    {
                        lpoNew = new CFieldCRTimes;
                    }
                    else if (apl_strcmp(loSpliter.GetField(luN), "crmtimes") == 0)
                    {
                        lpoNew = new CFieldCRMTimes;
                    }
                    else if (apl_strcmp(loSpliter.GetField(luN), "cwtimes") == 0)
                    {
                        lpoNew = new CFieldCWTimes;
                    }
                    else if (apl_strcmp(loSpliter.GetField(luN), "cwmtimes") == 0)
                    {
                        lpoNew = new CFieldCWMTimes;
                    }
                    else if (apl_strcmp(loSpliter.GetField(luN), "cstimes") == 0)
                    {
                        lpoNew = new CFieldCSTimes;
                    }
                    else
                    {
                        apl_errprintf("invalid field name (%s)\n", loSpliter.GetField(luN) );
                        return -1;
                    }
                    
                    lpoLastOutput->SetNext(lpoNew);

                    lpoLastOutput = lpoNew;
                }

                lpoLastOutput->SetNext(new CFieldEnd);

                break;
            }
            case 'n':
            {
                giInterval = apl_strtoi32(loOpt.OptArg(), APL_NULL, 10);

                if (giCount == 0)
                {
                    giCount = APL_INT_MAX;
                }

                break;
            }
            case 'c':
                giCount = apl_strtoi32(loOpt.OptArg(), APL_NULL, 10);
                break;
            default:
                PrintUsage();
                return 0;
        }
    }
    
    if (giPort == 0 || goIpAddress.length() == 0)
    {
        PrintUsage();
        return 0;
    }

    if (lpoOutput == APL_NULL)
    {
        //Set defualt output
        lpoOutput = new CFieldStart(
                        new CFieldName(
                            new CFieldGroup(
                                new CFieldServer(
                                    new CFieldFSegment(
                                        new CFieldUSegment(
                                            new CFieldConn(
                                                new CFieldRWTimes(
                                                    new CFieldEnd() ) ) ) ) ) ) ) );
    }
    
    acl::CSockAddr loRemote(giPort, goIpAddress.c_str() );
    gfq2::CGFQueueClient loClient;
    if (loClient.Initialize("main", loRemote, 1, 10) != 0)
	{
	    printf( "Client initialize fail, may be connect rejected - %s\n", apl_strerror(apl_get_errno() ) );
	    return -1;
	}
    
    for (apl_int_t i = 0; i < giCount; i++)
    {
        std::vector<gfq2::CServerNodeInfo> loResult;
        
        if (loServerNodeList.size() > 0)
        {
            gfq2::CServerNodeInfo loServerNodeInfo;

            for (std::vector<std::string>::iterator loIter = loServerNodeList.begin();
                 loIter != loServerNodeList.end(); ++loIter)
            {
                if ( (liRetCode = loClient.GetServerNodeInfo(loIter->c_str(), loServerNodeInfo) ) != 0)
                {
                    printf( "Get %s server node info fail, MSGCODE:%"APL_PRIdINT"\n", loIter->c_str(), liRetCode);
                    return -1;
                }

                loResult.push_back(loServerNodeInfo);
            }
        }
        else
        {
            if ( (liRetCode = loClient.GetAllServerNodeInfo(loResult) ) != 0)
            {
                printf( "Get all server node info fail, MSGCODE:%"APL_PRIdINT"\n", liRetCode);
                return -1;
            }
        }

        lpoOutput->WriteSeparatorLine();
        lpoOutput->WriteTitle();
        lpoOutput->WriteSeparatorLine();

        for (std::vector<gfq2::CServerNodeInfo>::iterator loIter = loResult.begin();
             loIter != loResult.end(); ++loIter)
        {
            lpoOutput->WriteContext(*loIter);
        }
        lpoOutput->WriteSeparatorLine();

        if (giInterval > 0 && i != giCount - 1)
        {
            apl_sleep(giInterval * APL_TIME_SEC);
        }
    }

    ACL_DELETE(lpoOutput);

    return 0;
}
