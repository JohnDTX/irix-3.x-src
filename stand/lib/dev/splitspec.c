/*
 * $Source: /d2/3.7/src/stand/lib/dev/RCS/splitspec.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:14:50 $
 */

#include	"ctype.h"
#include	"stand.h"

/*
** splitspec
**   crack a bootfile string (without hurting the source string).
**
**	[prefix[.ext]:]file
**
**   for backwards compatibility, any number of
**   punct's may separate the prefix from the extension.
*/
char	prfxbuf[ 20 ],	/* holds the prefix string	*/
	extbuf[ 20 ];	/* holds the extension string	*/

splitspec( str, prfx, ext, fname )
char	*str;
char	**prfx;
char	**ext;
char	**fname;
{
    register char	*sp,
			*cp;
    char		*rp;
    register struct devsw *dp;

    extbuf[ 0 ] = prfxbuf[ 0 ] = 0;
    *prfx = prfxbuf;
    *ext = extbuf;
    *fname = str;

    /*
    ** find the first colon.  it separates the prefix and
    ** extension from the "rest" of the spec.  if there is
    ** no colon, the prefix and extension are both absent.
    */
    for ( sp = str; *sp; sp++ )
	if ( *sp == ':' || *sp == '/' )
	    break;

    if ( *sp != ':' )	/* no colon, so no prefix and extension	*/
	return;
    rp = sp;

    sp++;
    *fname = sp;

    /*
    ** there is something before the colon.  separate it
    ** into prefix (media type) and extension (device spec).
    **
    ** this is a little tricky, since the addition of TCP
    ** netbooting.  TCP hostnames can contain numeric digits.
    ** the way this code used to work, it would parse "hd0:foo"
    ** as "hd.0:foo", so people have gotten used to referring
    ** to devices in this more compact (but syntactically
    ** incorrect) way.  if we want to continue to support this
    ** omission of the punctuation mark between MEDIA and
    ** DEVSPEC, we must actually use the device names from the
    ** devsw to attempt to distinguish DEVSPEC strings containing
    ** numeric digits from MEDIA+DEVSPEC with the punctuation
    ** omitted.
    **
    ** note that this will create some surprises.  for example:
    **
    ** "tcphost:foo" will be parsed as "tcp.host:foo"
    **
    ** if you actually want to refer to a host called "tcphost",
    ** you must say "tcp.tcphost:foo".
    */
    cp = prfxbuf;
    for ( sp = str; isalpha( *sp ); sp++ )
	if ( cp < prfxbuf + sizeof prfxbuf - 1 )
	    *cp++ = *sp;
    *cp = 0;

    /* try to match a device */
    for ( dp = devsw; dp->dv_name; dp++ ) {
	if ( !strcmp( prfxbuf, dp->dv_name ) )
	    goto gotmatch;
    }

    /*
    ** no match, so the whole string before the colon must be
    ** the extension or device spec.
    */
    prfxbuf[0] = '\0';
    sp = str;

gotmatch:
    /*
    ** have taken care of the prefix.  skip the (optional)
    ** separator punctuation and copy the extension to extbuf.
    */
    while ( (sp < rp) && (*sp != '*') && (ispunct( *sp )) )
	sp++;

    for ( cp = extbuf; sp < rp; sp++ )
	if ( cp < extbuf + sizeof extbuf - 1 )
	    *cp++ = *sp;
    *cp++ = 0;
}
