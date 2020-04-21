/*	@(#)filbuf.c	2.1	*/
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
#include <stdio.h>

extern _findbuf();
extern int read();
extern int fflush();
extern FILE *_lastbuf;

int
_filbuf(iop)
register FILE *iop;
{
	register FILE *diop;
	register char *iop_flag;

	iop_flag = &iop->_flag;

	if (iop->_base == NULL)  /* get buffer if we don't have one */
		_findbuf(iop);

	if ( !(*iop_flag & _IOREAD) )
		if (*iop_flag & _IORW)
			*iop_flag |= _IOREAD;
		else
			return(EOF);

	/* if this device is a terminal (line-buffered) or unbuffered, then */
	/* flush buffers of all line-buffered devices currently writing */

	if (*iop_flag & (_IOLBF | _IONBF))
		for (diop = _iob; diop < _lastbuf; diop++ )
			if (diop->_flag & _IOLBF)
				(void) fflush(diop);

	iop->_ptr = iop->_base;
	iop->_cnt = read(fileno(iop), (char *)iop->_base,
	    (unsigned)((*iop_flag & _IONBF) ? 1 : _bufsiz(iop) ));
	if (--iop->_cnt >= 0)		/* success */
		return (*iop->_ptr++);
	if (iop->_cnt != -1)		/* error */
		*iop_flag |= _IOERR;
	else {				/* end-of-file */
		*iop_flag |= _IOEOF;
		if (*iop_flag & _IORW)
			*iop_flag &= ~_IOREAD;
	}
	iop->_cnt = 0;
	return (EOF);
}
