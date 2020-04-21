static	char	*Sccsid = "@(#)$Header: /d2/3.7/src/usr.bin/RCS/gclear.c,v 1.1 89/03/27 17:40:11 root Exp $";
/*
 * $Log:	gclear.c,v $
 * Revision 1.1  89/03/27  17:40:11  root
 * Initial check-in for 3.7
 * 
 * Revision 1.4  85/09/10  12:27:20  root
 * Larry fixed gclear to work nicely under mex.
 * 
 * Revision 1.3  85/03/11  13:25:25  bob
 * Fixed to work reliably on both GL1 & GL2 with many bitplanes.
 * 
 */

#include <gl.h>

main()
{
#ifdef gl2
	noport();		/* no port is needed, hint it so */
#endif gl2

	ginit();

#ifdef gl2
	if (ismex()) {		/* gclear confuses mex no end	*/
		gexit();	/* 	so don't let it happen	*/
		exit(0);
	}
#endif gl2

	doublebuffer();
	gconfig();
	frontbuffer(TRUE);
	color(BLACK);
#ifdef gl2
	textinit();
	tpon();
#endif gl2
	clear();
	gexit();
	exit(0);
}
