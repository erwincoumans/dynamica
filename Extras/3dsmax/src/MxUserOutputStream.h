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