/*
 * Strings methods.
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/usr.bin/edge/libwin/common/RCS/string.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:47:37 $
 */
#include <stdio.h>
#include "string.h"

extern	char	*sbrk(int);

long *
_concat_(register const char *s1, register const char *s2)
{
	register char *to;
	register int len;
	char *rto;

	len = strlen(s1) + strlen(s2) + 1;
	rto = to = new char[len];
	while (*s1)
		*to++ = *s1++;
	while (*s2)
		*to++ = *s2++;
	*to = 0;
	return (long *) rto;
}

string::string(long *c)
{
	data = (char *) c;
}

/* s1 = s2 */
void
string::operator=(string& a)
{
	delete data;
	data = new char[strlen(a.data) + 1];
	strcpy(data, a.data);
}

/* s1 = "..." */
void
string::operator=(const char *s)
{
	delete data;
	data = new char[strlen(s) + 1];
	strcpy(data, s);
}

/* s1[i] */
char&
string::operator[](int i)
{
	if (i >= strlen(data)) {
		fprintf(stderr, "string[]: index out of range\n");
		return data[0];
	} else
		return data[i];
}

/* s1 += s2 */
void
string::operator+=(string& a)
{
	char *newdata;

	newdata = (char *) _concat_(data, a.data);
	delete data;
	data = newdata;
}

/* s1 += "..." */
void
string::operator+=(const char *s)
{
	char *newdata;

	newdata = (char *) _concat_(data, s);
	delete data;
	data = newdata;
}
