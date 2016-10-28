#ifndef __AILIBEX__AIGBKUNICODE_H__
#define __AILIBEX__AIGBKUNICODE_H__

#include "AILib.h"

///start namespace
AIBC_NAMESPACE_START

extern int GBToUnicode( const char *apcGB, const int aiGBSize, char *apcUnicode );
extern int GBToUnicode( const char *apcGB, const int aiGBSize, char *apcUnicode, int *apiUniSize );
extern int UnicodeToGB( const char *apcUnicode, const int aiUniSize, char *apcGB );
extern int UnicodeToGB( const char *apcUnicode, const int aiUniSize, char *apcGB, int *apiGBSize );
extern int GBToUTF8( const char *apcGB, const int aiGBSize, char *apcUtf8 );

extern int BIG5ToUnicode( const char *apcBIG5, const int aiBIG5Size, char *apcUnicode );
extern int BIG5ToUnicode( const char *apcBIG5, const int aiBIG5Size, char *apcUnicode, int *apiUniSize );
extern int UnicodeToBIG5( const char *apcUnicode, const int aiUniSize, char *apcBIG5 );
extern int UnicodeToBIG5( const char *apcUnicode, const int aiUniSize, char *apcBIG5, int *apiBIG5Size );

///end namespace
AIBC_NAMESPACE_END

#endif // __AILIBEX__AIGBKUNICODE_H__

