/*
 * Draw the nine rooms on the screen
 *
 * @(#)rooms.c	3.8 (Berkeley) 6/15/81
 * @(#)rooms.c	1.8 8/2/84
 */

#include "rogue.h"

do_rooms()
{
    reg1 regd1	int	i;
    reg2 rega1	struct	room	*rp;
    reg3 rega2	struct	linked_list *item;
    reg4 rega3	struct	thing	*tp;
    reg5 regd2	int	left_out;
		coord	top;
		coord	bsze;
		coord	mp;

    /*
     * bsze is the maximum room size
     */
    bsze.x = COLS/3;
    bsze.y = LINES/3;
    /*
     * Clear things for a new level
     */
    for (rp = rooms; rp < &rooms[MAXROOMS]; rp++)
	rp->r_goldval = rp->r_nexits = rp->r_flags = 0;
    /*
     * Put the gone rooms, if any, on the level
     */
    left_out = rnd(4);
    for (i = 0; i < left_out; i++)
	rooms[rnd_room()].r_flags |= ISGONE;
    /*
     * dig and populate all the rooms on the level
     */
    for (i = 0, rp = rooms; i < MAXROOMS; rp++, i++) {
	/*
	 * Find upper left corner of box that this room goes in
	 */
	top.x = (i%3)*bsze.x + 1;
	top.y = i/3*bsze.y;
	if (rp->r_flags & ISGONE) {
	    /*
	     * Place a gone room.  Make certain that there is a blank line
	     * for passage drawing.
	     */
	    do {
		rp->r_pos.x = top.x + rnd(bsze.x-2) + 1;
		rp->r_pos.y = top.y + rnd(bsze.y-2) + 1;
		rp->r_max.x = -COLS;
		rp->r_max.x = -LINES;
	    } until(rp->r_pos.y > 0 && rp->r_pos.y < LINES-1);
	    continue;
	}
	if (rnd(10) < level-1)
	    rp->r_flags |= ISDARK;
	/*
	 * Find a place and size for a random room
	 */
	do {
	    rp->r_max.x = rnd(bsze.x - 4) + 4;
	    rp->r_max.y = rnd(bsze.y - 4) + 4;
	    rp->r_pos.x = top.x + rnd(bsze.x - rp->r_max.x);
	    rp->r_pos.y = top.y + rnd(bsze.y - rp->r_max.y);
	} until (rp->r_pos.y != 0);
	/*
	 * Put the gold in
	 */
	if (rnd(100) < 50 && (!amulet || level >= max_level)) {
	    rp->r_goldval = GOLDCALC;
	    rnd_pos(rp, &rp->r_gold);
	    if (roomin(&rp->r_gold) != rp)
		endwin(), abort();
	}
	draw_room(rp);
	/*
	 * Put the monster in
	 */
	if (rnd(100) < (rp->r_goldval > 0 ? 80 : 25)) {
	    item = new_item(sizeof *tp);
	    tp = (struct thing *) ldata(item);
	    do {
		rnd_pos(rp, &mp);
	    } until(mvwinch(stdscr, mp.y, mp.x) == floor);
	    new_monster(item, randmonster(FALSE), &mp);
	    /*
	     * See if we want to give it a treasure to carry around.
	     */
	    if (rnd(100) < monsters[tp->t_type-'A'].m_carry)
		attach(tp->t_pack, new_thing());
	}
    }
}

/*
 * Draw a box around a room
 */

draw_room(rp)
reg1 rega1	struct	room	*rp;
{
    reg2 regd1	int	j;
    reg3 regd2	int	k;

#ifdef	COLOR
    acolor(stdscr,C_cyan,Dbackground);
#endif
    Move(rp->r_pos.y, rp->r_pos.x+1);
    vert(rp->r_max.y-2);				/* Draw left side */
    Move(rp->r_pos.y+rp->r_max.y-1, rp->r_pos.x);
    addch(wallll);					/* Lower Left corner */
    horiz(rp->r_max.x-2);				/* Draw bottom */
    addch(walllr);					/* Lower Right corner */
    Move(rp->r_pos.y, rp->r_pos.x);
    addch(wallul);					/* Upper Left corner */
    horiz(rp->r_max.x-2);				/* Draw top */
    addch(wallur);					/* Upper Right corner */
    vert(rp->r_max.y-2);				/* Draw right side */
#ifdef	COLOR
    acolor(stdscr,C_blue,Dbackground);
#endif
    /*
     * Put the floor down
     */
    for (j = 1; j < rp->r_max.y-1; j++) {
	Move(rp->r_pos.y + j, rp->r_pos.x+1);
	for (k = 1; k < rp->r_max.x-1; k++)
	    addch(floor);
    }
    /*
     * Put the gold there
     */
#ifdef	COLOR
    acolor(stdscr,C_gold,Dbackground);
#endif
    if (rp->r_goldval)
	mvaddch(rp->r_gold.y, rp->r_gold.x, GOLD);
#ifdef	COLOR
    acolor(stdscr,Dforeground,Dbackground);
#endif
}

/*
 * horiz:
 *	draw a horizontal line
 */

horiz(count)
reg1 regd1	int	count;
{
    while (count--)
	addch(wallh);
}

/*
 * vert:
 *	draw a vertical line
 */

vert(count)
reg1 regd1	int	count;
{
    reg2 regd2	int	x;
    reg3 regd3	int	y;

    while (count--) {
	getyx(stdscr,y,x);
	Move(y+1,x-1);
	addch(wallv);
    }
}

/*
 * rnd_pos:
 *	pick a random spot in a room
 */

rnd_pos(rp, cp)
reg1 rega1	struct	room	*rp;
reg2 rega2	coord	*cp;
{
    cp->x = rp->r_pos.x + rnd(rp->r_max.x-2) + 1;
    cp->y = rp->r_pos.y + rnd(rp->r_max.y-2) + 1;
}
