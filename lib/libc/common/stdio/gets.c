/*	@(#)gets.c	3.3	*/
/*LINTLIBRARY*/
/*
 * This version reads directly from the buffer rather than looping on getc.
 * Ptr args aren't checked for NULL because the program would be a
 * catastrophic mess anyway.  Better to abort than just to return NULL.
 */
#include <stdio.h>
#include "stdiom.h"

extern int _filbuf();
extern _bufsync();
extern char *memccpy();

char *
gets(ptr)
char *ptr;
{
	char *p, *ptr0 = ptr;
	register int n;
	register FILE *fi = stdin;

	for ( ; ; ) {
		if (fi->_cnt <= 0) { /* empty buffer */
			if (_filbuf(fi) == EOF) {
				if (ptr0 == ptr)
					return (NULL);
				break; /* no more data */
			}
			fi->_ptr--;
			fi->_cnt++;
		}
		n = fi->_cnt;
		if ((p = memccpy(ptr, (char *) fi->_ptr, '\n', n)) != NULL)
			n = p - ptr;
		ptr += n;
		fi->_cnt -= n;
		fi->_ptr += n;
		_BUFSYNC(fi);
		if (p != NULL) { /* found '\n' in buffer */
			ptr--; /* step back over '\n' */
			break;
		}
	}
	*ptr = '\0';
	return (ptr0);
}
