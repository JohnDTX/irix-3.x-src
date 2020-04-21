/* rec = idb_read (file, set) -- read a record
 *
 * The next record of the idb open as "file" is read, allocated in the memset
 * "set", and a pointer to it is returned.  A null byte is treated as end
 * of file; this allows an idb to be imbedded in another file.  NULL is
 * returned to the caller on this or physical end of file.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#define library
#include "idb.h"

#define Ntmp	256

static char	*get_field ();

static char	_field [1024];
static int	field_type;
static int	inside = 0;

static FILE	*rfile;
static Memset	*rset;

Rec *
idb_read (file, set)
	FILE		*file;
	Memset		*set;
{
	register Rec	*rec;
	int		ftype, i, j, argx;
	Attr		*attr, tmpattr [Ntmp], *at;
	char		*argv [1024], *r;

	idb_freeset (set);
	rfile = file; rset = set; inside = 0;
	i = getc (rfile);
	ungetc (i, rfile);
	if (i == EOF || i == 0) return (NULL);
	rec = (Rec *) idb_getmem (sizeof (Rec), rset);
	rec->type = idb_typei (*get_field ());
	rec->mode = strtol (get_field (), NULL, 8);
	rec->user = idb_stash (get_field (), rset);
	rec->group = idb_stash (get_field (), rset);
	rec->dstpath = idb_stash (get_field (), rset);
	rec->srcpath = idb_stash (get_field (), rset);
	rec->nattr = 0;
	rec->attr = NULL;
	r = get_field ();
	while (field_type != Eor) {
		at = idb_addattr (rec, r, 0, NULL, rset);
		r = get_field ();
		if (field_type == Lpar) {
			r = get_field ();
			while (field_type != Rpar && field_type != Eor) {
				idb_addarg (at, idb_stash (r, rset), rset);
				r = get_field ();
			}
			if (field_type == Rpar) r = get_field ();
		}
	}
	return (rec);
}

static char *
get_field ()
{
	register int	c, quote;
	register char	*p;

	field_type = 0;
	while (c = getc (rfile)) {
		if (c == EOF || c == '\0') { field_type = Eor; return (""); }
		if (!inside && c == '\n') {
			c = peekch (rfile);
			if (!isspace (c)) {
				field_type = Eor;
				return ("");
			}
		}
		if (inside && c == ',') break;
		if (!isspace (c)) break;
	}
	if (c == '(') {
		inside = 1; field_type = Lpar; return ("(");
	}
	if (c == ')') {
		inside = 0; field_type = Rpar; return (")");
	}
	if (c == '"' || c == '\'') {
		quote = c; p = _field;
		while ((c = getc (rfile)) != EOF && c != '\0' && c != quote) {
			if (c == '\\') {
				if ((c = getc (rfile)) == EOF || c == '\0')
					break;
			}
			*p++ = c;
		}
		*p = '\0';
	} else {
		p = _field;
		while (c != EOF && c != '\0') {
			if (isspace (c) || c == '(' || c == ')' ||
			    inside && c == ',') {
				ungetc (c, rfile);
				break;
			}
			if (c == '\\') {
				c = getc (rfile);
				if (c == EOF || c == '\0') break;
			}
			*p++ = c;
			c = getc (rfile);
		}
		*p = '\0';
	}
	return (_field);
}

#ifdef BSD

#define DIGIT(x) (isdigit(x)? ((x)-'0'): (10+tolower(x)-'a'))
#define MBASE 36

long
strtol(str, ptr, base)
register char *str;
char **ptr;
register int base;
{
	register long val;
	register int xx, sign;

	val = 0L;
	sign = 1;
	if(base < 0 || base > MBASE)
		goto OUT;
	while(isspace(*str))
		++str;
	if(*str == '-') {
		++str;
		sign = -1;
	} else if(*str == '+')
		++str;
	if(base == 0) {
		if(*str == '0') {
			++str;
			if(*str == 'x' || *str == 'X') {
				++str;
				base = 16;
			} else
				base = 8;
		} else
			base = 10;
	} else if(base == 16)
		if(str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
			str += 2;
	/*
	 * for any base > 10, the digits incrementally following
	 *	9 are assumed to be "abc...z" or "ABC...Z"
	 */
	while(isalnum(*str) && (xx=DIGIT(*str)) < base) {
		/* accumulate neg avoids surprises near maxint */
		val = base*val - xx;
		++str;
	}
OUT:
	if(ptr != (char**)0)
		*ptr = str;
	return(sign*(-val));
}
#endif
