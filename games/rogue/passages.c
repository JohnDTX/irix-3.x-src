static	char	*Passages_c	= "@(#)passages.c	1.6";
/*
 * Draw the connecting passages
 *
 * @(#)passages.c	2.3 (Berkeley) 2/11/81
 * @(#)passages.c	1.6 8/2/84
 */

#include "rogue.h"

#define NCONN 15

/*
 * connect:
 *	Connection tables for the tic-tac-toe board room layout.
 *	Format is <room><direction to draw>.  Only legal directions
 *	are 'd' (down) and 'r' (right).
 */

static	char	*connect[NCONN] = {
    "0r1r1d2d3d3r4d5d6r7r",
    "0r0d2d3r4r4d5d6r",
    "0r1r2d3d3r4r5d7r",
    "0r1d1r2d3d4d6r7r",
    "0r1d1r2d3r5d6r7r",
    "0d1d2d3d3r4d4r5d",
    "0r1d2d3d3r4r6r7r",
    "0d0r1r2d3d4d4r5d",
    "0d0r1r2d3r4d4r5d6r",
    "0r1r2d3d4d5d6r7r",
    "0d0r1d1r2d3d3r4d4r5d6r7r",
    "0r1d1r2d3d3r4d4r7r",
    "0r1d2d3d4d4r5d6r",
    "0d0r1d1r2d4d5d6r",
    "0d0r1d1r4r5d6r7r"
};

/*
 * do_passages:
 *	Draw all the passages on a level.
 */

do_passages()
{
    reg1 rega1	char	*scheme;

    scheme = connect[rnd(NCONN)];		/* Pick a method */
    /*
     * Draw each passage in this connection scheme
     */
    while(*scheme)
    {
	conn(*scheme, *(scheme+1));
	scheme += 2;
    }
}

/*
 * conn:
 *	Draw a corridor from a room in a certain direction.
 */

conn(rm, direc)
char	rm, direc;
{
    reg1 rega1	struct	room	*rpf;
    reg2 rega2	struct	room	*rpt;
    reg3 regd1	char	rmt;
    reg4 regd2	int	distance;
    reg5 regd3	int	turn_spot;
    reg6 regd4	int	turn_distance;
		coord	delta, curr, turn_delta, spos, epos;

    rm -= '0';				/* No more ascii bias */
    rpf = &rooms[rm];
    /*
     * Set up the movement variables, in two cases:
     * first drawing one down.
     */
    if (direc == 'd')
    {
	rmt = rm + 3;				/* room # of dest */
	rpt = &rooms[rmt];			/* room pointer of dest */
	delta.x = 0;				/* direction of move */
	delta.y = 1;
	spos.x = rpf->r_pos.x;			/* start of move */
	spos.y = rpf->r_pos.y;
	epos.x = rpt->r_pos.x;			/* end of move */
	epos.y = rpt->r_pos.y;
	if (!(rpf->r_flags & ISGONE))		/* if not gone pick door pos */
	{
	    spos.x += rnd(rpf->r_max.x-2)+1;
	    spos.y += rpf->r_max.y-1;
	}
	if (!(rpt->r_flags & ISGONE))
	    epos.x += rnd(rpt->r_max.x-2)+1;
	distance = abs(spos.y - epos.y) - 1;	/* distance to move */
	turn_delta.y = 0;			/* direction to turn */
	turn_delta.x = (spos.x < epos.x ? 1 : -1);
	turn_distance = abs(spos.x - epos.x);	/* how far to turn */
	turn_spot = rnd(distance-1) + 1;		/* where turn starts */
    }
    else if (direc == 'r')			/* setup for moving right */
    {
	rmt = rm + 1;
	rpt = &rooms[rmt];
	delta.x = 1;
	delta.y = 0;
	spos.x = rpf->r_pos.x;
	spos.y = rpf->r_pos.y;
	epos.x = rpt->r_pos.x;
	epos.y = rpt->r_pos.y;
	if (!(rpf->r_flags & ISGONE))
	{
	    spos.x += rpf->r_max.x-1;
	    spos.y += rnd(rpf->r_max.y-2)+1;
	}
	if (!(rpt->r_flags & ISGONE))
	    epos.y += rnd(rpt->r_max.y-2)+1;
	distance = abs(spos.x - epos.x) - 1;
	turn_delta.y = (spos.y < epos.y ? 1 : -1);
	turn_delta.x = 0;
	turn_distance = abs(spos.y - epos.y);
	turn_spot = rnd(distance-1) + 1;
    }
    else
	debug("error in connection tables");
    /*
     * Draw in the doors on either side of the passage or just put #'s
     * if the rooms are gone.
     */
    if (!(rpf->r_flags & ISGONE)) adddoor(rpf, &spos);
    else
    {
	Cmov(spos);
	addch(passage);
    }
    if (!(rpt->r_flags & ISGONE)) adddoor(rpt, &epos);
    else
    {
	Cmov(epos);
	addch(passage);
    }
    /*
     * Get ready to move...
     */
    curr.x = spos.x;
    curr.y = spos.y;
    while(distance)
    {
	/*
	 * Move to new position
	 */
	curr.x += delta.x;
	curr.y += delta.y;
	/*
	 * Check if we are at the turn place, if so do the turn
	 */
	if (distance == turn_spot && turn_distance > 0)
	    while(turn_distance--)
	    {
		Cmov(curr);
		addch(passage);
		curr.x += turn_delta.x;
		curr.y += turn_delta.y;
	    }
	/*
	 * Continue digging along
	 */
	Cmov(curr);
	addch(passage);
	distance--;
    }
    curr.x += delta.x;
    curr.y += delta.y;
    if (!ce(curr, epos))
	msg("Warning, connectivity problem on this level.");
}

/*
 * Add a door or possibly a secret door
 * also enters the door in the exits array of the room.
 */

adddoor(rm, cp)
reg1 rega1	struct	room	*rm;
reg2 rega2	coord	*cp;
{
    Cmov(*cp);
    addch(rnd(10) < level - 1 && rnd(100) < 20 ? SECRETDOOR : door);
    rm->r_exit[rm->r_nexits++] = *cp;
}

/*
 * add_pass:
 *	add the passages to the current window (wizard command)
 */

add_pass()
{
    reg1 regd1	int	y;
    reg2 regd2	int	x;
    reg3 regd3	int	ch;

    for (y = 1; y < LINES - 2; y++)
	for (x = 0; x < COLS; x++)
	    if ((ch=mvinch(y, x)) == passage || ch == door || ch == SECRETDOOR)
		mvwaddch(cw, y, x, ch);
}
