#ifdef _JAVA

#include "../../declarations.h"
#include "../../text.h"

char *qspToSysString(QSP_CHAR *s)
{
	int len = QSP_WCSTOMBSLEN(s) + 1;
	char *ret = (char *)malloc(len);
	QSP_WCSTOMBS(ret, s, len);
	return ret;
}

#else

static void qspDummyFunc()
{
}

#endif
