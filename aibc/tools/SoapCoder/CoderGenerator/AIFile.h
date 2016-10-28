//=============================================================================
/**
 * \file    AIFile.h
 * \brief 
 *
 * Copyright (c) 2007 Asiainfo Technologies(China),Inc.
 * RCS: $Id: AIFile.h,v 1.1 2009/02/23 03:48:19 daizh Exp $
 *
 * History
 * 2008.02.15 first release by DZH
 */
//=============================================================================
#ifndef AI_FILE_2008
#define AI_FILE_2008

#include <stdio.h>
#include <string>

#if !defined(MAX_BUFF_SIZE)
#define MAX_BUFF_SIZE 4096
#endif

class clsFileName
{
public:
     clsFileName( const char* apsFileName )
     {
          Parser( apsFileName );
     }
     const char* GetFileName()    { return coStrFileName.c_str(); }
     const char* GetFullName()    { return coStrFullName.c_str(); }
     const char* GetBaseName()    { return coStrBaseName.c_str(); }
     const char* GetExtName()     { return coStrExtName.c_str(); }

protected:
     void Parser( const char* apsFileName )
     {
          coStrFullName = apsFileName;
		  std::string::size_type pos = coStrFullName.rfind("/");
          if ( pos == std::string::npos )
          {
               pos = coStrFullName.rfind("\\");
               if ( pos == std::string::npos )
               {
                    coStrFileName = coStrFullName;
               }
               else coStrFileName = coStrFullName.substr( pos + 1, coStrFullName.length() - pos -1 );
          }
          pos = coStrFileName.rfind(".");
          if ( pos == std::string::npos )
          {
               coStrBaseName = coStrFileName;
               coStrExtName = "";
          }
          else 
          {
               coStrBaseName = coStrFileName.substr( 0, pos );
               coStrExtName = coStrFileName.substr( pos + 1, coStrFileName.length() - pos -1 );
          }
     }

protected:
     std::string coStrFullName;
     std::string coStrBaseName;
     std::string coStrFileName;
     std::string coStrExtName;
};

class clsFile
{
public:
     clsFile( const char* apsFileName, const char* apsMode ) : 
          cpFile(NULL),
          cbIsOpen(false),
          cbIsFail(false)
     {
          Open( apsFileName, apsMode );
     }
     clsFile( ) : 
          cpFile(NULL),
          cbIsOpen(false),
          cbIsFail(false)
     {
     }
     ~clsFile( )
     {
          Close();
     }
     bool Open( const char* apsFileName, const char* apsMode ) 
     {
          cpFile = fopen( apsFileName, apsMode );
          if ( cpFile == NULL )
          {
               cbIsFail = true;
               coStrResultMsg = strerror( errno );
               return false;
          }
          coStrFileName = apsFileName;

		  return true;
     }
     void Close( )
     {
          if ( cpFile != NULL ) fclose( cpFile );
     }
     void SetFileName( const char* apsBaseName, const char* apsExt )
     {
          coStrBaseName = apsBaseName;
          coStrExtName = apsExt;
     }
     const char* GetLine( )
     {
          if ( fgets( csBuff, MAX_BUFF_SIZE, cpFile ) == NULL )
          {
               return NULL;
          }
          return csBuff;
     }
     clsFile& operator << ( const char* apsBuff )
     {
          cbIsFail = false;
          if ( fputs( apsBuff, cpFile ) != strlen(apsBuff) )
          {
                cbIsFail = true;
          }
          return *this;
     }
     bool IsOpen( )   { return cbIsOpen; }
     const char* GetMsg( )
     {
          return coStrResultMsg.c_str();
     }
     const char* GetFileBaseName( )
     {
          if ( IsOpen() ) return coStrBaseName.c_str();
          return NULL;
     }
     const char* GetFileExtName( )
     {
          if ( IsOpen() ) return coStrExtName.c_str();
          return NULL;
     }
     bool Fail( )  { return cbIsFail; }
     const char* GetFileName( )
     {
          if ( IsOpen() ) return coStrFileName.c_str();
          return NULL;
     }
protected:
     FILE* cpFile;
     bool cbIsOpen;
     bool cbIsFail;
     char csBuff[ MAX_BUFF_SIZE ];
     std::string coStrFileName;
     std::string coStrBaseName;
     std::string coStrExtName;
     std::string coStrResultMsg;
};

#endif // AI_FILE_2008