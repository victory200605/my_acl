#include "apl/apl.h"

int main(int argc, char* argv[])
{
    apl_handle_t liFd = atoi(argv[1]);

    char *lpcVal1 = getenv("key1");
    char *lpcVal2 = getenv("key2");

    char lsBuf[40];
    apl_memset(lsBuf, 0, sizeof(lsBuf));
    snprintf(lsBuf, sizeof(lsBuf), "key1=%s,key2=%s", lpcVal1, lpcVal2);
    apl_write(liFd, lsBuf, apl_strlen(lsBuf));
    return 0;
}
