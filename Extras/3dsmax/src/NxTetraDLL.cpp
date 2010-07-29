#include "PxPlugin.h"

#ifdef PXPLUGIN_SOFTBODY
#define NOMINMAX
#include <windows.h>

#include <NxPhysics.h>
#include "NxTetra.h"

NxTetraInterface* getNxTetraInterface()
{
	static NxTetraInterface* ret = NULL;
	if (ret != NULL) return ret;

	const char *dmodule = "NxTetra.dll";
	HMODULE module = LoadLibrary( dmodule );
	if ( module )
	{
		if ( module )
		{
			void *proc = GetProcAddress(module,"getTetraInterface");
			if ( proc )
			{
				typedef NxTetraInterface * (__cdecl * NX_GetToolkit)();
				ret = ((NX_GetToolkit)proc)();
			}
		}
	}
	return ret;
}
#endif //PXPLUGIN_SOFTBODY
