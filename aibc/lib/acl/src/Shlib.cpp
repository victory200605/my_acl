
#include "acl/Shlib.h"

ACL_NAMESPACE_START

CShlib::CShlib()
    : mpvHandle(NULL)
{
}

apl_int_t CShlib::Load( const char* apcLibName, apl_uint_t auMode ) 
{
	if ( this->IsLoad() )
	{
	    apl_set_errno(APL_EBUSY);
	    return -1;
	}
	
    this->mpvHandle = apl_dlopen(apcLibName, auMode);

    return this->mpvHandle == NULL ? -1 : 0;
}

void CShlib::Unload()
{
    if ( this->mpvHandle != NULL ) 
    {
        apl_dlclose(mpvHandle);
        this->mpvHandle = NULL;
    }
}

void* CShlib::GetSymbol( const char* apcSymbol ) 
{
    return apl_dlsym(this->mpvHandle, apcSymbol);
}

const char* CShlib::GetDlError() const
{
    const char* lpcDlError = apl_dlerror();
	return lpcDlError == NULL ? "" : lpcDlError;
}

bool CShlib::IsLoad() const
{
	return this->mpvHandle != NULL;
}

ACL_NAMESPACE_END
