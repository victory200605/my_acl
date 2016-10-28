/** @file AIConfig.h
 */

#ifndef __AILIBEX__AICONFIG_H__
#define __AILIBEX__AICONFIG_H__

#include "AILib.h"
#include "AIDualLink.h"
#include "AISynch.h"

///start namespace
AIBC_NAMESPACE_START

typedef struct tagDualLinkItem
{
    char        *cpcKey;
    char        *cpcValue;
    LISTHEAD    coIniChild;
} CONFIGITEM;

/**
 * @brief This is class AIConfig.
 */
class AIConfig
{
private:
    LISTHEAD    coIniHead;
    AIMutexLock coMutexLock;

private:
    char        csName[128];
    LISTHEAD    coConfigChild;

public:
    AIConfig(void);
    ~AIConfig(void);

private:
    void    Clean(void);

public:     /* Functions for INI */

    /**
     * @brief Load all value in the init file.
     *
     * @param apcFileName    IN - init file name
     *
     * @return 0:ok; -1:open init file failed; -2:malloc failed
     */
    int     LoadINI(char const* apcFileName);

    /**
     * @brief Reload all value in the init file.
     *
     * @return 0:ok; -1:open init file failed; -2:malloc failed
     */ 
    int     ReloadINI();

    /**
     * @brief Get one int value in the init file
     *
     * @param apcField    IN - e.g. :FwdGateway
     * @param apcKeyVal   IN - e.g. :port
     * @param aiDefValue  IN - the default value of the key,e.g. :5999
     *
     * @return the int value
     */
    int     GetIniInt(char const* apcField, char const* apcKeyVal, int aiDefValue);

    /**
     * @brief Get one string value in the init file
     *
     * @param apcField    IN - e.g. :FwdGateway
     * @param apckeyVal   IN - e.g. :ip
     * @param apcRetValue OUT - the value of the key,e.g. :10.3.3.92
     * @param aiSize      IN - the buffer size of apcRetValue
     *
     * @return the length of apcRetValue
     */
    int     GetIniString(char const* apcField, char const* apcKeyVal, char* apcRetValue, size_t aiSize);

    /**
     * @brief Get the number of key in the specified field.
     *
     * @param apcField    IN - e.g. :FwdGateway
     *
     * @return the number of key
     */
    size_t  GetKeyCount(char const* apcField);

    /**
     * @brief Get string form init file by index.
     *
     * @param apcField    IN - e.g. :FwdGateway
     * @param aiIndex     IN - the specified index
     * @param apcKeyVal   OUT - the key 
     * @param aiKeyLen    IN - the specified key length
     * @param apcRetValue OUT - the value of key
     * @param aiValLen    IN - the specified value length
     *
     * return 0:ok; -1:haven't succeed
     */ 
    int     GetIniStringByIndex(char const* apcField, size_t aiIndex, char* apcKeyVal, size_t aiKeyLen, char* apcRetValue, size_t aiValLen);
    
    /**
     * @brief Get int from init file by index.
     *
     * @param apcField     IN - e.g. :FwdGateway
     * @param aiIndex      IN - the specified index
     * @param apcKeyVal    OUT - the key
     * @param aiKeyLen     IN - the specified key length
     * @param aiDefValue   IN - the default value
     *
     * @return the int value or default int value. 
     */ 
    int     GetIniIntByIndex(char const* apcField, size_t aiIndex, char* apcKeyVal, size_t aiKeyLen, int aiDefValue);

    /**
     * @brief Get the number of field according to the specified apcFieldPattern.
     *
     * @param apcFieldPattern    IN - the specified apcFieldPattern
     *
     * @return the number of field.
     */
    size_t  GetFieldCount(char const* apcFieldPattern);

    /**
     * @brief Get field from FieldPattern according to index.
     *
     * @param apcFieldPattern     IN - the specified apcFieldPattern
     * @param aiIndex             IN - the index
     * @param apcField            OUT - the field in index
     * @param aiFieldLen          IN - ???can't find
     *
     * @return 0:ok; -1:haven't succeed
     */
    int     GetFieldByIndex(char const* apcFieldPattern, size_t aiIndex, char* apcField, size_t aiFieldLen);

    /**
     * @brief ShowAllInitInfo in the init file, use for TEST
     *
     * return 0
     */
    int     ShowAllInitInfo();
    
public:     /* For string split-operate */
    /**
     * @brief Get subString from one string.
     *
     * @param apcInStr    IN - one line string
     * @param apcOutStr   IN - sub string
     * @param aiOutSize   IN - the buffer size of apcOutStr
     * @param aiHasMarks  IN - 1 or 0
     *
     * @return Return the pointer of next sub string.
     */
    static  char *GetSubString(char const* apcInStr, char* apcOutStr, size_t aiOutSize, int aiHasMarks);
    friend  AIConfig *AIGetIniHandler(char const *apcIniFileName);
    friend  void AICloseIniHandler(void);
    friend  int  AIReloadAllIniHandler(void);
};

/**
 * @brief Initialize the init handler
 */
extern  void AIInitIniHandler(void);

/**
 * @brief Reload all the init handler
 */
extern  int  AIReloadAllIniHandler(void);

/**
 * @brief Close init handler
 */
extern  void AICloseIniHandler(void);

/**
  * @brief Search the AIConfig handle in goGlobalConfigHead or new the handle and added it into goGlobalConfigHead.you should use AIInitIniHandler() and AIGetIniHandler() before you use AIConfig::LoadINI().
  *
  * @param apcIniFileName    IN - the name of init file
  * 
  * @return the handle of the pointer of AIConfig,then you can use the handle to call GetIniInt() and GetIniString().
  */
extern  AIConfig *AIGetIniHandler(char const* apcIniFileName);

///end namespace
AIBC_NAMESPACE_END

#endif   /* AICONFIG_H_2004, 2004.08.20 */
