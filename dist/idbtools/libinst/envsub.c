#include <ctype.h>
#include "inst.h"

#define NULL		0
#define left_curl	'{'
#define right_curl	'}'

extern char	*getenv ();

envsub (line)
	char		*line;
{
	char		buff [Strsize], name [Strsize], *mark;
	register char	*p, *s, *t;

	s = line;
	t = buff;
	while (*s) {
		if (*s == '\\') {
			*t++ = *++s;
			++s;
			continue;
		}
		if (*s != '$') {
			*t++ = *s++;
			continue;
		}
		mark = ++s;
		p = name;
		if (*s == left_curl) {
			while (*++s != '\0' && *s != right_curl) {
				*p++ = *s;
			}
			if (*s == right_curl) ++s;
		} else {
			while (*s == '_' || isalnum (*s)) {
				*p++ = *s++;
			}
		}
		*p = '\0';
		if ((p = getenv (name)) != NULL) {
			while (*t = *p++) ++t;
		} else {
			*t++ = '$';
			s = mark;
		}
	}
	*t = '\0';
	strcpy (line, buff);
}
