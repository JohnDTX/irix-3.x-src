#define library
#include "idb.h"

idb_buffsize (f)
	FILE		*f;
{
	ungetc (getc (f), f);
#ifdef BSD
	return (f->_bufsiz);
#else
	return (_bufsiz (f));
#endif
}
