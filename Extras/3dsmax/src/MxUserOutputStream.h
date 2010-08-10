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

#ifndef MX_USEROUTPUTSTREAM_H
#define MX_USEROUTPUTSTREAM_H

//#include "NxUserOutputStream.h"
#include "MxUtils.h"

#include <max.h>
#include "MAXScrpt\MAXScrpt.h"
extern CharStream *gCurrentstream;

class MxUserOutputStream : public NxUserOutputStream
{
public:
	MxUserOutputStream(CharStream* outputStream) 
	{
		m_outputStream = outputStream;
	}

	virtual ~MxUserOutputStream()
	{
		m_outputStream = NULL;
	}

	void reportError(NxErrorCode code, const char* message, const char* file, int line)
	{
		if (gCurrentstream == NULL) return;
		if (code < NXE_DB_INFO)
		{
			gCurrentstream->printf("PhysX SDK Error: \"%s\" file %s line %d\n", message, file, line);
		}
	}
	NxAssertResponse reportAssertViolation(const char* message, const char* file, int line)
	{
		if (gCurrentstream == NULL) return NX_AR_CONTINUE;
		gCurrentstream->printf("PhysX SDK Assertion: \"%s\" file %s line %d\n", message, file, line);
		return NX_AR_CONTINUE;
	}
	void print(const char *message)
	{
		if (gCurrentstream == NULL) return;
		gCurrentstream->printf("PhysX SDK Message: %s\n", message);
	}
protected:
	CharStream* m_outputStream;
};

#endif //MX_USEROUTPUTSTREAM_H