/*
 *		 Ring Buffer Package
 * 		    David J. Brown
 *		    April 11, 1983
 *
 *	Separated for Quirk PROM kernel June 20, 1983
 */

/*
 *  Routines to manage ring-buffers
 *
 *     These routines are generic - they take a pointer to the ring	XXX
 *  in question, and return/enter an element from/into the ring.	XXX
 */

#include "ringbufs.h"


/*
 * RingGet - return an item from the event queue: 'ring'
 *	returns -1 if event queue is empty.
 */
int
Ring_Get(ring)
register struct ring	*ring;
{
    register Datum item;

    if (ring->in == ring->out)			/* ring is empty 	*/
	return -1;

    item = *ring->out;				/* get the item pointer	*/

    ring->out++;
    if (ring->out == ring->last)
        ring->out = ring->first;

    return item;
}

/*
 * RingPut - Enter the datum: item, into the event queue: ring.
 *	returns -1 if event queue is full.
 */
int
Ring_Put(ring, item)
register struct ring *ring;	/* a5 */
register Datum item;
{
    register Pointer		newin;		/* a3 */
    
    newin = ring->in + 1;
    if (newin == ring->last)
        newin = ring->first;

    if(newin == ring->out)			/* ring buffer is full	*/
	return -1;

    *ring->in = item;

    ring->in = newin;			/* update the input pointer	*/
    return 0;
}

/*
 * RingCount - return a count of items in the event queue
 */
int
Ring_Count(ring)
register struct ring	*ring;
{
    unsigned short	count;
    
    count = (ring->in - ring->out) /* / ring->datumSize*/ ;
    if (count<0)
        count += (ring->last - ring->first) /* / ring->datumSize */;
    return(count);
}


# ifndef RingEmpty
RingEmpty(ring)
register struct ring	*ring;
{
    return ring->in == ring->out;
}
# endif RingEmpty
