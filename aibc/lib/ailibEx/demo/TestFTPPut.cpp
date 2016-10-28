#include "AIFTPLayer.h"
#include "AIString.h"
#include "AITime.h"

#define ERR_RETURN_IF(ret, ftp, msg, expr) if(expr) { printf("ERR [%d] %s: %d %s\n", ret, msg, ftp.GetReplyCode(), ftp.GetReplyMsg()); return ret;}

int main(int argc, char const** argv)
{
    clsFTPConnection loFtp;
    ssize_t liRet;

    if (argc < 8)
    {
        printf("Usage: %s ip port user pass remote_dir local_file timeout\n", argv[0]);
        return 0;
    }


    liRet = loFtp.ConnectTo(argv[1], StringAtoi(argv[2]), StringAtoi(argv[7]) * AI_TIME_SEC);

    ERR_RETURN_IF(liRet, loFtp, "connect", (liRet < 0));

    liRet = loFtp.Login(argv[3], argv[4], StringAtoi(argv[7]) * AI_TIME_SEC);

    ERR_RETURN_IF(liRet, loFtp, "login", (liRet < 0));

    liRet = loFtp.PutFile(argv[5], argv[6], StringAtoi(argv[7]) * AI_TIME_SEC);

    ERR_RETURN_IF(liRet, loFtp, "put file", (liRet < 0));

    printf("OK\n");

    return 0;      
}

