static char Sccsid[]="@(#)xunlink.c	3.1";
/*
	Interface to unlink(II) which handles all error conditions.
	Returns 0 on success,
	fatal() on failure.
*/

xunlink(f)
{
	if (unlink(f))
		return(xmsg(f,"xunlink"));
	return(0);
}
