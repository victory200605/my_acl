//=============================================================================
/**
* \file    AIDir.h
* \brief dir operation interface
*
* Copyright (c) 2007 Asiainfo Technologies(China),Inc.
* RCS: $Id: AIDir.h,v 1.4 2009/03/02 13:39:50 daizh Exp $
*
* History
* 2008.03.07 first release by DZH
*/
//=============================================================================
#ifndef __AILIBEX__AIDIR_H__
#define __AILIBEX__AIDIR_H__

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>
#include <sys/stat.h>

#include "AILib.h"

///start namespace
AIBC_NAMESPACE_START

class AIDir 
{
public:
    enum 
    {
        AI_NO_ERROR = 0,
        AI_WARN_END_OF_DIR   =  1,
        AI_ERROR_OPEN_DIR    = -1,
        AI_ERROR_READ_DIR    = -2,
        AI_ERROR_MAKE_DIR    = -3,
        AI_ERROR_REMOVE_DIR  = -3,
        AI_ERROR_REMOVE_FILE = -3,
        AI_ERROR_READ_FILE   = -4
    };
public:
	AIDir();

	~AIDir();

	//open directory, if success, return eNoError
	int Open( const char* apcDirName );

	static int Make( const char* apcDirName, int aiMode = 0700 );
	static int Remove( const char* apcDirName );

	//rewind directory to start position
	void Rewind();

	//close directory
	void Close();

	//read next directory node
	int ReadNext();

	//get file or directory name
	const char* GetName() const;

	//get file or directory full name
	const char* GetFullName() const;

	//get dir path
	const char* GetDirPath() const;

	bool IsDir() const;

	bool IsFile() const;

	bool IsDots() const;

protected:
	//directory pointer
	DIR* cpoDir;

	//dirent
	struct dirent* cpoResultItem;
	struct dirent* cpoItem;

	//directory stat
	struct stat coStat;

	//current directory path
	char csDirPath[PATH_MAX];

	//full name
	char csFullName[PATH_MAX];
	char csBaseName[PATH_MAX];
};

///end namespace
AIBC_NAMESPACE_END

#endif //AI_DIR_H_2008
