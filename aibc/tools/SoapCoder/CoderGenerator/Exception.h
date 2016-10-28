//=============================================================================
/**
 * \file    Exception.h
 * \brief �쳣�ദ��
 *
 * Copyright (c) 2007 Asiainfo Technologies(China),Inc.
 * RCS: $Id: Exception.h,v 1.1 2009/02/23 03:48:19 daizh Exp $
 *
 * History
 * 2008.01.30 first release by DZH
 */
//=============================================================================
#ifndef AI_EXCEPTION_H_2008
#define AI_EXCEPTION_H_2008

#include <string>
#include <stdarg.h>

#define AI_MAX_BUFF_SIZE 4096

/**
* \brief ״̬���࣬����һЩ���ڲ��������п��ܲ���һЩ����򾯸�
*/
class clsException
{
public:
     /* ö��״̬���� */
     enum { eError   = -1, eSuccess = 0, eWarning };

     clsException():ciStat(eError)                             {}
     clsException( const char* acpcMsg ) :ciStat(eError)
     {
          SetMsg( acpcMsg );
     }
     clsException( const clsException* acpoMsg ) : ciStat(eError)
     {
          SetMsg( acpoMsg );
     }
     clsException( int aiStat, const char* acpcMsg )   
     {
          SetMsg( aiStat, acpcMsg );  
     }
     const char* SetMsg( const char* acpcMsg )         
     { 
          coResultMsg = acpcMsg; 
          return acpcMsg; 
     }
     const char* SetMsg( const clsException* acpoMsg )    
     { 
          coResultMsg = acpoMsg->coResultMsg; 
          ciStat = acpoMsg->ciStat; 
          return coResultMsg.c_str(); 
     }
     const char* SetMsg( int aiStat, const char* acpcMsg )   
     { 
          coResultMsg = acpcMsg; 
          ciStat = aiStat; 
          return acpcMsg; 
     }
     const char* GetMsg()                              
     { 
          return coResultMsg.c_str();
     }
     const char* What()                              
     { 
          return coResultMsg.c_str();
     }
     int  GetStat()                                    
     { 
          return ciStat; 
     }

protected:
     std::string    coResultMsg;
     int          ciStat;
};

inline const char* Format( char* pFormat, ... )
{
     static char szStr[AI_MAX_BUFF_SIZE];
     va_list vl;
     va_start(vl, pFormat);
     //����ʽ�����ַ���
     vsprintf(szStr, pFormat, vl);

     va_end(vl);

     return szStr;
}

#endif //AI_EXCEPTION_H_2008
