#ifndef __AILIBEX__AIINDEXTREE_H__
#define __AILIBEX__AIINDEXTREE_H__

#include "AIDualLink.h"
#include "AILib.h"

///start namespace
AIBC_NAMESPACE_START

typedef enum INDXTYPE {BYTE,WORD,DWORD,STRING,BINARY};

typedef struct INDXMETA
{
	INDXTYPE		ciIndexType;
	unsigned int	ciFieldOffset;
	unsigned int	ciFieldLength;
} INDXMETA;

typedef struct INDXINFO
{
	int				ciFieldCount;
	INDXMETA		coIndexMeta[4];
	
	LISTHEAD		coIdxHead[AIT_MAX_HASH_SIZE];
} INDXINFO;

typedef struct _INDXCHILD
{
	LISTHEAD		coChild;
	LISTHEAD		coIdxChild[16];
} _INDXCHILD;

class clsIndexTree
{
private:	
	int				ciIdxCount;
	LISTHEAD		coNodeHead;
	INDXINFO		*cpoIdxInfo[16];
};

///end namespace
AIBC_NAMESPACE_END

#endif // __AILIBEX__AIINDEXTREE_H__

