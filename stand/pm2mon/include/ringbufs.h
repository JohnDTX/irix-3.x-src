/*
 *		 Ring Buffer Package
 * 		    David J. Brown
 *		    April 11, 1983
 *
 *	Separated for Quirk PROM kernel June 20, 1983
 */

#ifndef RINGBUFS
#define RINGBUFS

/*
 * exports of ringbufs.c 
 */

/* We just want a pointer -- make it a pointer to char */
typedef unsigned char Datum;
typedef Datum *Pointer;

struct ring
{
    short	datumSize;	/* size of the data being managed	*/
    Pointer	first,		/* first bucket in the ring buffer	*/
    		last,		/* last bucket in the ring buffer	*/
		in,		/* next place to get an item		*/
		out;		/* next place to put an item		*/
};

/*
 * Macros to help user with Rings
 */

#define RingReset(ring)		((ring)->in = (ring)->out = (ring)->first)

#define	RingInit(ring, b, n)		\
	((ring)->in = (ring)->out = (ring)->first = (Pointer)(b) , \
	(ring)->last = (Pointer)((b)+n) , (ring)->datumSize = sizeof*(b))

#define RingEmpty(ring)		((ring)->in==(ring)->out)

#endif RINGBUFS
