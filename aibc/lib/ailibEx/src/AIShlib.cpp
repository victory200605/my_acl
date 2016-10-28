
#include "AIShlib.h"
#include "AIString.h"
#include <stdio.h>
#include <dlfcn.h>

///start namespace
AIBC_NAMESPACE_START

AIShlib::AIShlib() : cpHandle(NULL)
{}

int AIShlib::Load( const char* apcLibName, int aiMode /* = RTLD_NOW */ ) 
{
	this->UnLoad();
	
	// Initialize error msg
	csErrorMsg[0] = '\0';
	
	//TODO : may be clean last error
	::dlerror();
	
    cpHandle = ::dlopen(apcLibName, aiMode);

    if ( cpHandle == NULL )
    {
        const char* lpcDlError = ::dlerror();
	    StringCopy( csErrorMsg, ( lpcDlError == NULL ? "" : lpcDlError ), sizeof(csErrorMsg) );
        return -1;
    }
    else
    {
        return 0;
    }
}

void AIShlib::UnLoad()
{
    if ( cpHandle != NULL ) 
    {
        ::dlclose(cpHandle);
        cpHandle = NULL;
    }
}

void* AIShlib::GetSymbol( const char* apcSymbol ) 
{
    // Initialize error msg
	csErrorMsg[0] = '\0';
	
	//TODO : may be clean last error
    ::dlerror();
    
    void* lpSym = ::dlsym(cpHandle, apcSymbol);
    if ( lpSym == NULL )
    {
        const char* lpcDlError = ::dlerror();
	    StringCopy( csErrorMsg, ( lpcDlError == NULL ? "" : lpcDlError ), sizeof(csErrorMsg) );
    }
    
    return lpSym;
}

const char* AIShlib::GetDlError() const
{
	return csErrorMsg;
}

bool AIShlib::IsLoad() const
{
	return cpHandle != NULL;
}

///end namespace
AIBC_NAMESPACE_END
