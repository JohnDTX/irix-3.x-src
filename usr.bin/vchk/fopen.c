/*
 * (C) 1983 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

#include	<stdio.h>
#include	<errno.h>

struct _iobuf *
fopen(file, mode)
char *file;
register char *mode;
{
	extern int errno;
	register f;
	register struct _iobuf *iop;
	extern struct _iobuf *_lastbuf;

	for (iop = _iob; iop->_flag&(_IOREAD|_IOWRT); iop++)
		if (iop >= _lastbuf)
			return(NULL);
	if (*mode=='w')
		f = creat(file, 0666);
	else if (*mode=='a') {
		if ((f = open(file, 1)) < 0) {
			if (errno == ENOENT)
				f = creat(file, 0666);
		}
		lseek(f, 0L, 2);
	} else
		f = open(file, 0);
	if (f < 0)
		return(NULL);
	iop->_cnt = 0;
if ((iop->_flag & _IOMYBUF) == 0) {
iop->_ptr = (char *)0;
iop->_base = (char *)0;
iop->_flag = _IOMYBUF;
}
	iop->_file = f;
	if (*mode != 'r')
		iop->_flag |= _IOWRT;
	else
		iop->_flag |= _IOREAD;
	return(iop);
}
