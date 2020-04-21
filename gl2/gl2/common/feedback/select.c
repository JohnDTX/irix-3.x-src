/*
 select causes a nameing conflict with IP/TCP code so
 we put it into its own dot.oh, and make the real thing be
 called gselect.
			Peter Broadwell as per some scr.
			Tue Aug 12 18:29:11 PDT 1986
*/

void select(buff,buflen)
short *buff;
long buflen;		/* length in names */
{
    gselect(buff,buflen);
}
