/**
 * @file Dir.h
 */
#ifndef ACL_DIR_H
#define ACL_DIR_H

#include "acl/Utility.h"

ACL_NAMESPACE_START

/**
 * class CDir
 */
class CDir 
{
public:
 
    /**
     * Constructor
     */
    CDir(void);
    
    /**
     * Destructor
     */
    ~CDir(void);
    
    /**
     * Open diretory.
     *
     * @param [in] apcDirPath    the path of dir
     * @retval 0 If success
     * @retval 1 If failed
     */
    apl_int_t Open( char const* apcDirPath );

    /**
     * Rewind directory to start position
     * @retval 0 If success
     * @retval 1 If failed
     */
    apl_int_t Rewind(void);
	
    /**
     * Obtain the current location associated with the directory stream.
     * @retval 0 If success
     * @retval 1 If failed
     */
    apl_int_t Tell(void);
	
    /**
     * Set the position of the next readdir() operation on the directory stream specified by aiPos.
     *
     * @param [in] aiPos    the specified position
     * @retval 0 If success
     * @retval 1 If failed
     */ 
    apl_int_t Seek( apl_size_t aiPos );

    /**
     * Close directory.
     *
     */
    void Close(void);

    /**
     * Read next directory node.
     * 
     * @retval 0 If success
     * @retval 1 If failed
     */
    apl_int_t Read(void);

    /**
     * Get file or directory name.
     * 
     * @return the file or directory name
     */
    char const* GetName() const;

    /**
     * Get file or directory full name.
     * 
     * @return the file or directory full name
     */
    char const* GetFullName() const;

    /**
     * Get dir path.
     * 
     * @return the dir path
     */
    char const* GetPath() const;

    /**
     * If the current node is a directory.
     *
     * @retval true current node is directory 
     * @retval false current node isn't directory 
     */
    bool IsDir() const;

    /**
     * If the current node is a file.
     *
     * @retval true current node is file 
     * @retval false current node isn't file
     */
    bool IsFile() const;

    /**
     * If the current node are dots.
     *
     * @retval true current node are dots
     * @retval false current node aren't dots
     */
    bool IsDots() const;

public:

    /**
     * Create the the specified directory by specified mode.
     *
     * @param [in] apcDirPath    the specified dir path
     * @param [in] aiMode        the specified mode
     * @retval 0 If success
     * @retval 1 If failed
     */ 
    static apl_int_t Make( char const* apcDirPath, apl_int_t aiMode = 0700 );

    /**
     * Remove the specified directory.
     *
     * @param [in] apcDirPath    the specified dir path
     * @retval 0 If success
     * @retval 1 If failed
     */
    static apl_int_t Remove( char const* apcDirPath );
	
protected:
    bool      mbIsOpened;
    
    apl_dir_t moDir;
    
    apl_dirent_t* mpoDirent;
    
    /* Directory stat */
	apl_stat_t moStat;

	/* Current directory path */
	char macDirPath[APL_PATH_MAX];

	/* Full name */
	char macFullName[APL_PATH_MAX];
};

ACL_NAMESPACE_END

#endif//ACL_FILE_H
