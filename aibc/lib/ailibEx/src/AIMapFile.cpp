//=============================================================================
/**
 * \file    AIFileMap.h
 * \brief 
 *
 * Copyright (c) 2007 Asiainfo Technologies(China),Inc.
 * RCS: $Id: AIMapFile.cpp,v 1.4 2009/03/02 13:39:51 daizh Exp $
 *
 * History
 * 2008.01.18 first release by DZH
 */
//=============================================================================

#include "AIMapFile.h"

///start namespace
AIBC_NAMESPACE_START

// Default constructor
AIMapFile::AIMapFile() : ciSize(0), cpAddr(NULL) 
{
}

// Discontuctor
AIMapFile::~AIMapFile()
{
	this->Unmap( );
}

int AIMapFile::Map( const char* apsFileName, int aiProt, int aiFlag, size_t aiLen /* = 0 */, off_t aiOff /* = 0 */ )
{
    if ( coFile.Open( apsFileName, O_RDWR ) != 0 )
    {
        return  (AI_ERROR_OPEN_FILE);
    }

    if ( aiLen <= 0 )
    {
        aiLen = coFile.GetSize();
        aiOff = 0;
    }
    
    return Map( coFile.GetHandle(), aiProt, aiFlag, aiLen, aiOff );
}

int AIMapFile::Map( int aiFileHandle, int aiProt, int aiFlag, size_t aiLen, off_t aiOff /* = 0 */ )
{
    ///Check file size
    struct stat loBuff;
    if ( ::fstat( aiFileHandle, &loBuff ) != 0 )
    {
        return AI_ERROR_STAT_FILE;
    }
    
    if ( loBuff.st_size < aiOff + (off_t)aiLen )
    {
        return AI_ERROR_OUT_OF_RANGE;
    }
    
    // Unmap current segment
    this->Unmap( );
    
    // Map file here
    cpAddr = ::mmap( NULL, aiLen, aiProt, aiFlag, aiFileHandle, aiOff );
        
    if ( cpAddr == (void*)-1 )
    {
        cpAddr = NULL;
        return AI_ERROR_MAP_FILE;
    }
    
    ciSize = aiLen;
    
    return AI_NO_ERROR;
}

int AIMapFile::Sync( int aiFlag /*= MS_SYNC*/ )
{
    if ( cpAddr == NULL ) return AI_NO_ERROR;
    
    if ( ::msync( cpAddr, ciSize, aiFlag ) != 0 )
    {
        return AI_ERROR_MSYNC_FILE;
    }
    
    return AI_NO_ERROR;
}

int AIMapFile::Unmap( )
{
    if ( cpAddr == NULL ) return AI_NO_ERROR;
    
    // Unmap it here
    if ( ::munmap( (char*)cpAddr, ciSize ) != 0 )
    {
        return AI_ERROR_UNMAP_FILE;
    }
    
    cpAddr = NULL;
    ciSize = 0;
    coFile.Close();
    
    return AI_NO_ERROR;
}

///end namespace
AIBC_NAMESPACE_END
