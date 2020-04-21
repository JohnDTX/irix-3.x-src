/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

	/* these are various subroutines called by other routines */
 
match(sp, fp, pp, lp)
	/* match the pattern pointed to by pp in the string (source) pointed to by
	 * sp. If found, copies the string preceedind the pattern in source
	 * to the array pointed to by fp and the string following the pattern 
	 * to the array pointed to by lp. It returns a 'true' if
	 * pattern is matched else it returns a 'false'. 		*/
	/* the strings pointed to by fp and lp are null in case of no match */
char *sp, *fp, *pp, *lp ;
{
	int found = 0;
	char *source, *first, *pattern, *last, *s1, *p1;
	source = sp;
	first = fp;
	last = lp;
	pattern = pp;
	*first = *last = '\0';
	if ( ! *source && ! *pattern ) return (1) ; /* source is null, pattern is null */
	if ( ! *source && *pattern ) return (0) ; /* source is null pattern is not null */
	if ( *source && ! *pattern ) { /* source not null, pattern is null */
		assign (1000, first, source); /* copy source to the array pointed to by fp */
		return(1);
	}
	/* both pattern and source are not null */
	while ( *source ) { 
		if (*source != *pattern ) {
			*first++ = *source++;
			continue;
		}
		s1 = source ; s1++;
		p1 = pattern ; p1++;
		while ( *p1 ) {
			if (*p1 == *s1) {
				++p1; ++s1;
				if ( ! *s1 ) { /* end of source */
					if ( ! *p1 ) { /* end of pattern */
						*last = *first =  '\0' ;
						return(1);
					}
					*fp = *lp = '\0'; /* match failed */
					return (0) ;
				}
			}
			else {
				*first++ = *source ;
				source++;
				break;
			}
		}
		if ( ! *p1 ) { /* end of pattern */
			source = s1;
			found = 1;
			break;
		}
	}
	if ( ! found ) {
		*fp = *lp = '\0';
		return (0);
	}
	while ( *source  ) *last++ = *source++;
	*first = *last = '\0';
	return (1);
}
assign (size, des, source)
	/* this routine copies the string pointed to by source to the string
	 * pointed to by des truncating it to the size of destination, if required */
char *des, *source;
int size;
{
	char *s, *d;
	int i = 0;
	s = source;
	d = des ;
	while (*source ) { /* not end of source */
		if ( i < size ) {
			*d++ = *source++;
			i++ ;
		}
		else break ;
	}
	*d = '\0' ;
}
concat (first, second, result)
char *first, *second, *result;
{
	char *t,  *f, *s, *r, temp[1000];
	f = first; t = temp ; s = second; r = result;
	while (*f ) /* not end o first string */
		*t++ = *f++ ;
	while (*s) /* not end of second string */
		*t++ = *s++ ;
	*t = '\0' ;
	assign (1000, r, temp) ;
}
initialize()
{
	int n = 0, m = 0, l ;
	extern char aline[], lcnt[], rpt[], pline[], rqname[];
	extern int lines;
	int ln[10];
	lines++;
	while (n <= 5) lcnt[n++] = ' ' ;
	lcnt[n--] = '\0' ;
	l = lines ;
	while ( l > 9 ) {
		ln [m++] = l % 10 ;
		l = l / 10 ;
	}
	ln [m] = l ;
	n = n - m ;
	while (m >= 0) lcnt [ n++ ] =  ln [ m-- ] + '0' ;
	rpt[0] = pline[0] = rqname[0] = '\0';
}
next13()
{
	int n = 0;
	extern char rpt[], aline[], line[];
	rpt[0] = '\0';
	while (aline[n]) line[n] = aline[n++];
	aline[0] = line[n] = '\0';
}
split(src, des, len)
	/* this routine cuts 'len' charecters off the end of the string 
	 * pointed to by src into the string pointed to by des. If len
	 * is greater than the length of src, it returns with des as
	 * a null string						*/
char *src, *des ;
int len;
{
	char *source, *dest;
	int l, n = 0 ;
	source = src;
	dest = des;
	l = len;
	while (*source) {
		source++;
		n++;
	}
	if (n < l) {
		*dest = '\0';
		return (0);
	}
	n = 0;
	while (n < l){
		*dest++ = *(source - l + n) ;
		n++ ;
	}
	*dest = '\0';
	*(source - l) = '\0';
}
