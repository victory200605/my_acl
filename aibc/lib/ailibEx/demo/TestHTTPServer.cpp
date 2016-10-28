#include <stdio.h>
#include <time.h>

#include "AILib.h"
#include "AIHTTPLayer.h"
#include "AITime.h"
#include "AIString.h"

int main(int argc, char const** argv)
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s ip port\n", argv[0]);
        return -1;
    }

    char const*     lpcIP = argv[1];
    int const       liPort = StringAtoi(argv[2]);
    int             liRet;
    
    clsTcpSocket    loSocket;

    liRet = loSocket.ListenOn(lpcIP, liPort);
    if (liRet < 0)
    {
        fprintf(stderr, "ERROR: failed to listen\n");
        return -1;
    } 

    printf("INFO: listening on %s:%d\n", lpcIP, liPort);

    char    lsNewIP[20];
    int     liNewPort;

    liRet = loSocket.Accept(lsNewIP, &liNewPort, AI_TIME_SEC*20);

    if (liRet < 0)
    {
        fprintf(stderr, "ERROR: failed to accept\n");
        return -1;
    }

    clsHTTPRequest      loReq;
    clsHTTPConnection   loConn(liRet);

    liRet = loConn.RecvHeader(&loReq);

    if (liRet < 0)
    {
        fprintf(stderr, "ERROR: failed to recv request header\n");
        return -1;
    }

    for (size_t i = 0; i < loReq.GetFieldCount(); ++i)
    {
        printf("INFO: response header field[%s:%s]\n", loReq.GetFieldName(i), loReq.GetFieldValue(i));
    }

    liRet = loConn.RecvBody(&loReq);

    if (liRet < 0)
    {
        fprintf(stderr, "ERROR: failed to recv request body\n");
        return -1;
    }

    for (size_t i = 0; i < loReq.GetEntityCount(); ++i)
    {
        printf("\nINFO: response entity (%d):\n", loReq.GetEntitySize(i));
        write(STDOUT_FILENO, loReq.GetEntityPtr(i), loReq.GetEntitySize(i));
    }
 
    clsHTTPResponse     loResp;

    loResp.AddField(AI_HTTP_FIELD_SERVER, argv[0]);
    loResp.AddField(AI_HTTP_FIELD_DATE, "Sun, 06 Nov 1994 08:49:37 GMT");
    loResp.AddField(AI_HTTP_FIELD_CONTENT_TYPE, "text/plain");
    loResp.AddField("SoapAction", loReq.GetFieldValue("SoapAction"));

    char const* const CONTENT = "hello_world!\r\n";

    loResp.AddEntity(CONTENT, strlen(CONTENT));

    liRet = loConn.SendHeader(&loResp);

    if (liRet < 0)
    {
        fprintf(stderr, "ERROR: failed to send response header");
        return -1;
    }

    liRet = loConn.SendBody(&loResp);

    if (liRet < 0)
    {
        fprintf(stderr, "ERROR: failed to send response body");
        return -1;
    }

    fprintf(stderr, "INFO: success to send response");

    return 0;
}

