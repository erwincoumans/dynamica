/* Copyright (c) 2008 NVIDIA CORPORATION

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

//For feedback and latest version see http://dynamica.googlecode.com

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
