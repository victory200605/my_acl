/**
 * @file Shlib.h
 */
#ifndef ACL_SHLIB_H
#define ACL_SHLIB_H

#include "Utility.h"

ACL_NAMESPACE_START

/** 
 * class CShlib
 */
class CShlib 
{
public:
   
    /**
     * Constructor
     */
    CShlib(void);
    
    /**
     * load a share library
     *
     * @param [in] apcLibName    share libaray path
     * @param [in] auMode        describes how this function shall operate upon apcLibName with respect to the processing of relocations and the scope of visibility of the symbols provided within apcLibName. detail reference to APL dlfcn
     * @retval 0    If success
     * @retval -1   If failed
     */ 
    apl_int_t Load( char const* apcLibName, apl_uint_t auMode = APL_RTLD_NOW );

    /** 
     * Unload shared library. 
     *
     */
    void Unload();

    /**
     * Look up symbol.
     *
     * @param [in] apcSymbol    the symbol's name
     * @retval APL_NULL    Fail.
     * @retval otherwise   Successful, the value is the address of a symbol.
     */
    void* GetSymbol( char const* apcSymbol );
    
    /**
     * Look up symbol in shared library, cast it to the desired  poapl_int_ter type, void* by default.
     *
     * @param [in] apcSymbol    the symbol's name
     * @retval APL_NULL    Fail.
     * @retval otherwise   Successful, the value is the address of a symbol.
     */
    template < class T >
    T GetSymbol( char const* apcSymbol ) {
        // Double cast avoids warning about casting object to function poapl_int_ter
        return (T)( this->GetSymbol(apcSymbol) );
    }
    
    /**
     * shall return a null-terminated character string that describes the last error that occurred during dynamic linking processing
     *
     * @retval null-pointer    Fail.
     * @retval otherwise       Successful, the return pointer pointed to a null-terminated character string.
     */
    const char* GetDlError() const;

    /**
     * If sharelibrary is loaded
     * 
     * @retval true    the sharelibrary is loaded.
     * @retval false   the sharelibrary isn't loaded.
     */
    bool IsLoad() const;
	
private:
    void* mpvHandle;
};

ACL_NAMESPACE_END

#endif//ACL_SHLIB_H
