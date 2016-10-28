#include <stdio.h>
#include <time.h>

#include "AILib.h"
#include "AIHTTPLayer.h"
#include "AITime.h"
#include "AIString.h"

int main(int argc, char const** argv)
{
    clsHTTPConnection   loConn;
    clsHTTPRequest      loReq;
    clsHTTPResponse     loResp;
    char lsBuf[1024];

    if (argc < 6)
    {
        printf("Usage: %s ip port method uri timeout\n", argv[0]);
    }


    char const*             lpcIP = argv[1];
    int const               liPort = StringAtoi(argv[2]);

    AITime_t const          liTimeout = StringAtoi(argv[5]) * AI_TIME_SEC;

    ssize_t                 liRet;

    
    printf("INFO: connect %s:%d\n", lpcIP, liPort);
    liRet = loConn.ConnectTo(lpcIP, liPort, liTimeout);
    if (liRet < 0)
    {
        printf("ERROR: failed to connect server: %d\n", liRet);
        return -1;
    } 

    loReq.SetMethod(argv[3]);
    loReq.SetUri(argv[4]);
    sprintf(lsBuf, "%s:%d", lpcIP, liPort);
    loReq.AddField(AI_HTTP_FIELD_HOST, lsBuf);
    loReq.AddField(AI_HTTP_FIELD_ACCEPT, "*/*");
    loReq.AddField(AI_HTTP_FIELD_PRAGMA, "no-cache");
    loReq.AddField(AI_HTTP_FIELD_USER_AGENT, "ailibEx/2.x");
    loReq.AddField("SoapAction", "");


    printf("INFO: request method: %s\n", loReq.GetMethod());
    printf("INFO: request uri: %s\n", loReq.GetUri());
    for (size_t i=0; i < loReq.GetFieldCount(); ++i)
    {
        printf("INFO: request header field: [%s:%s]\n", loReq.GetFieldName(i), loReq.GetFieldValue(i));
    }

    liRet = loConn.HandleClientRequest(&loReq, &loResp);
    if (liRet < 0)
    {
        printf("ERROR: handle request failed: %d\n", liRet);
        return -1;
    }

    printf("INFO: response status: %d\n", loResp.GetStatus());

    printf("INFO: response reason pharse: %s\n", loResp.GetReason());

    for (size_t i = 0; i < loResp.GetFieldCount(); ++i)
    {
        printf("INFO: response header field[%s:%s]\n", loResp.GetFieldName(i), loResp.GetFieldValue(i));
    }

    for (size_t i = 0; i < loResp.GetEntityCount(); ++i)
    {
        printf("INFO: response entity (%d):\n", loResp.GetEntitySize(i));
        write(STDOUT_FILENO, loResp.GetEntityPtr(i), loResp.GetEntitySize(i));
    }

    return 0;
}

