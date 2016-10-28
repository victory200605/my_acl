//=============================================================================
/**
* \file    clsFileReader.h
* \brief for AIFile read buffer
*
* Copyright (c) 2007 Asiainfo Technologies(China),Inc.
* RCS: $Id: FileReader.h,v 1.1 2011/01/05 08:34:16 daizh Exp $
*
* History
* 2008.01.18 first release by DZH
*/
//=============================================================================

#ifndef AI_FILEREADER_2008
#define AI_FILEREADER_2008

#include "AIFile.h"

template< class T, int BufferSize >
class clsFileReader
{
public:
    clsFileReader( AIFile& aoFile, off_t aiOffset = 0 ) : 
        coFile(aoFile),
        ciCurrWater(0),
        ciHighWater(0),
        ciOffset(aiOffset)
    {}
    
    int Read( T* atBuff )
    {
        if ( ciCurrWater == ciHighWater )
        {//read from file
            int liRet = 0;
            if ( ( liRet = coFile.Read( ciOffset, &loBuffer, sizeof(T) * BufferSize ) ) <= 0 )
            {
                return liRet;
            }
            
            ciHighWater = liRet / sizeof(T);
            ciOffset += ciHighWater * sizeof(T);
            ciCurrWater = 0;
        }
        memcpy( atBuff, &loBuffer[ciCurrWater++], sizeof(T) );
        return sizeof(T);
    }
    
protected:
    AIFile& coFile;
    
    int ciCurrWater;
    int ciHighWater;
    off_t ciOffset;
    
    T loBuffer[ BufferSize ];
};

#endif //AI_FILEREADER_2008
