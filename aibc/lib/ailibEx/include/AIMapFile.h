//=============================================================================
/**
* \file    AIFileMap.h
* \brief 
*
* Copyright (c) 2007 Asiainfo Technologies(China),Inc.
* RCS: $Id: AIMapFile.h,v 1.3 2009/03/02 13:39:51 daizh Exp $
*
* History
* 2008.01.18 first release by DZH
*/
//=============================================================================
#ifndef __AILIBEX__AIFILEMAP_H__
#define __AILIBEX__AIFILEMAP_H__

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include "AIFile.h"

///start namespace
AIBC_NAMESPACE_START

class AIMapFile
{
public:
    enum
    {
        AI_NO_ERROR           = 0,
        AI_ERROR_OPEN_FILE    = -1,
        AI_ERROR_STAT_FILE    = -2,
        AI_ERROR_MAP_FILE     = -3,
        AI_ERROR_UNMAP_FILE   = -4,
        AI_ERROR_MSYNC_FILE   = -5,
        AI_ERROR_OUT_OF_RANGE = -6,
    };
    
public:
    AIMapFile();
    ~AIMapFile();
    
    // Use mmap file segment to memory, fail and return error
    int Map( const char* apsFileName, int aiProt, int aiFlag, size_t aiLen = 0, off_t aiOff = 0 );
    int Map( int aiFileHandle, int aiProt, int aiFlag, size_t aiLen, off_t aiOff = 0 );
    int Sync( int aiFlag = MS_SYNC );
    
    // Unmap file segment
    int Unmap( );
    
    // Check is mapped
    bool IsMap() const       { return cpAddr != NULL; }
    
    void*  GetAddr() const   { return cpAddr; }
    size_t GetSize() const   { return ciSize; }
    
protected:
    AIFile coFile;
    
    size_t ciSize;
    
    void* cpAddr;
};

///end namespace
AIBC_NAMESPACE_END

#endif //AI_FILEMAP_H_2008
