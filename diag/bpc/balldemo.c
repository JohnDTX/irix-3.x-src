/*
 *	Kurt Akeley			9/18/82
 *
 *	These routines are intended to be linked with the main routine
 *	"console.b".
 *
 *	Routines:
 *		iterate (x, y, dx, dy, d2, d2y, dt)
 *		ball (x, y, dx, dy, d2x, d2y)
 *
 *	Updates:
 *		9/18/82	 KBA	Copied from console.c
 *		2/21/83  KBA	UC3 mods
 */

#include "uctest.h"
#include "console.h"
#include "ucdev.h"

#define FONTADDR	0x100


iterate (x, y, dx, dy, d2x, d2y, dt)
long	*x, *y, *dx, *dy, d2x, d2y, dt;
{
    /*
     *	Accepts position, velocity, and acceleration values, and
     *	  returns updated position and velocity.
     *	All arithmetic is integer.
     */

    *x += *dx * dt;
    *y += *dy * dt;
    *dx += d2x * dt;
    *dy += d2y * dt;
    }

#define	RBOUND		((1024-0x2d)<<8)
#define LBOUND		(-1<<8)
#define TBOUND		((768-0xd)<<8)
#define BBOUND		(-1<<8)

ball (x, y, dx, dy, d2x, d2y)
long	x, y, dx, dy, d2x, d2y;
{
    /*
     *	Loads the ball character into the font memory;  Then drops the
     *	  ball from the specified position, with specified velocity
     *	  and acceleration.  Single buffer (a) is used.  The ball is
     *	  erased immediately before it is moved.
     */

    long	i;
    long	drawx, drawy;	/* shifted to restore to screen coords. */
    long	charwidth, charheight;
    Save	s;

    save (&s);
    x = x << 8;	/* gets 8 bits of play */
    y = y << 8;

#ifdef UC3
#include "ball/ball.bpc"
#endif UC3
#ifdef UC4
#include "ball/ball16.bpc"
#endif UC4

    while (1) {
	restore (&s);
	breakcheck ();
	drawx = x >> 8;
	drawy = y >> 8;

	setcodes (colorcode, wecode);
	LDXS (drawx)
	LDXE (drawx+charwidth-1)
	LDYS (drawy)
	LDYE (drawy+charheight-1)
	LDFMADDR (FONTADDR)
	REQUEST (DRAWCHAR, 0)

	for (i=0; i<2; i++) {
	    iterate (&x, &y, &dx, &dy, d2x, d2y, 1);
	    if ((x >= RBOUND) || (x <= LBOUND))
		dx = -dx + d2x;
	    if ((y >= TBOUND) || (y <= BBOUND))
		dy = -dy + d2y;
	    }
	waitvert (TRUE);

	setcodes (planecode (0, sigplanes, uc_cfb), wecode);
	LDXS (drawx)
	LDXE (drawx+charwidth-1)
	LDYS (drawy)
	LDYE (drawy+charheight-1)
	LDFMADDR (FONTADDR)
	REQUEST (DRAWCHAR, 0)
	}
    }



