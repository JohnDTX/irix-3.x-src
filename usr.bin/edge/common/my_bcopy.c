/*
 * $Source: /d2/3.7/src/usr.bin/edge/common/RCS/my_bcopy.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:46:01 $
 */
    /*
     * version of bcopy which handles a destination overlapping the source
     */
my_bcopy(from, to, length)
    char *from, *to;
    long length;
{
    if ((from < to) && (to < from+length)) {
	    /* copy from tail, to prevent clobbering source */
	while (length--)
	    *(to+length) = *(from+length);
    }
    else bcopy(from, to, length);
}
