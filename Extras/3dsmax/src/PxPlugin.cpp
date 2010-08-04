#include "PxPlugin.h"

//#include <NxPhysics.h>
#include "MxUtils.h"

#include <max.h>
#include "iFnPub.h"

#include "MAXScrpt\MAXScrpt.h"
#include "MAXScrpt\Listener.h"

#include "PxFunctions.h"

#ifdef PXPLUGIN_LEGACY
#include "PxLegacyFunctions.h"
#endif

HINSTANCE hInstance;
int controlsInit = FALSE;
CharStream* gCurrentstream = NULL;

// This function is called by Windows when the DLL is loaded.  This 
// function may also be called many times during time critical operations
// like rendering.  Therefore developers need to be careful what they
// do inside this function.  In the code below, note how after the DLL is
// loaded the first time only a few statements are executed.
BOOL WINAPI DllMain(HINSTANCE hinstDLL, ULONG fdwReason, LPVOID lpvReserved)
{
	hInstance = hinstDLL;				// Hang on to this DLL's instance handle.

	if (!controlsInit) {
		controlsInit = TRUE;
		InitCustomControls(hInstance);	// Initialize MAX's custom controls
		InitCommonControls();			// Initialize Win95 controls
	}

	return (TRUE);
}

__declspec( dllexport ) const TCHAR* LibDescription()
{
	return "PhysX SDK Plugin";
}

__declspec( dllexport ) int LibNumberClasses()
{
#ifdef PXPLUGIN_LEGACY
	return 2;
#else
	return 1;
#endif
}

extern ClassDesc2* GetPxPluginDesc();
#ifdef PXPLUGIN_LEGACY
extern ClassDesc2* GetPxPluginLegacyDesc();
#endif
__declspec( dllexport ) ClassDesc* LibClassDesc(int i)
{
	switch(i) {
		case 0: return GetPxPluginDesc(); //return &pxPluginDescriptor;
#ifdef PXPLUGIN_LEGACY
		case 1: return GetPxPluginLegacyDesc(); //&pxPluginLegacyDescriptor;
#endif
		default: return 0;
	}
}

__declspec( dllexport ) ULONG LibVersion()
{
	return VERSION_3DSMAX;
}

TCHAR* GetString(int id)
{
	static TCHAR buf[256];

	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}
