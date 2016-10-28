#ifndef __AILIBEX__AISHLIB__
#define __AILIBEX__AISHLIB__

#include <dlfcn.h>
#include "AILib.h"

///start namespace
AIBC_NAMESPACE_START

/** Encapsulates a shared library handle.
 *@see AutoShlib
 */
class AIShlib 
{
public:
    AIShlib();
    
    int Load( const char* apcLibName, int aiMode = RTLD_NOW );

    /** Unload shared library. */
    void UnLoad();

    /** Look up symbol. */
    void* GetSymbol( const char* apcSymbol );
    
    /** Look up symbol in shared library, cast it to the desired
     * pointer type, void* by default.
     */
    template < class T >
    T GetSymbol( const char* apcSymbol ) {
        // Double cast avoids warning about casting object to function pointer
        return (T)( this->GetSymbol(apcSymbol) );
    }
    
    /** Get dlerror **/
    const char* GetDlError() const;

	bool IsLoad() const;
	
private:
    void* cpHandle;
    
    char  csErrorMsg[1024];
};

///end namespace
AIBC_NAMESPACE_END

#endif  /*!QPID_SYS_SHLIB_H*/

