# include "ctype.h"

# define PROMSTATIC

/*
 * splitspec() --
 * crack a bootfile string (without hurting the source string).
 *
	[prefix[.ext]:]file
 *
 * for backwards compatibility, any number of
 * punct's may separate the prefix from the extension.
 */
PROMSTATIC	char prefbuf[20],extbuf[20];

splitspec(str,_pref,_ext,_rest)
    char *str;
    char (**_pref);
    char (**_ext);
    char (**_rest);
{
    register char *sp,*cp;
    char *rp;

    prefbuf[0] = 000; *_pref = prefbuf;
    extbuf[0] = 000; *_ext = extbuf;
    *_rest = str;

    /*
     * find the first colon.  it separates the prefix and
     * extension from the "rest" of the spec.  if there is
     * no colon, the prefix and extension are both absent.
     */
    for( sp = str; *sp != 000; sp++ )
	if( *sp == ':' || *sp == '/' )
	    break;
    if( *sp != ':' )
	return;
    rp = sp;

    *_rest = sp+1;

    /*
     * there is at least a prefix.  skip over it
     * (assuming it's alphabetic).  copy it to prefbuf.
     */
    cp = prefbuf;
    for( sp = str; isalpha(*sp); sp++ )
	if( cp < prefbuf + sizeof prefbuf - 1 )
	    *cp++ = *sp;
    *cp = 000;

    /*
     * have taken care of the prefix.  skip the (optional)
     * separator punctuation and copy the extension to extbuf.
     */
    while( sp < rp && ispunct(*sp) )
	sp++;

    for( cp = extbuf; sp < rp; sp++ )
	if( cp < extbuf + sizeof extbuf - 1 )
	    *cp++ = *sp;
    *cp++ = 000;
}
