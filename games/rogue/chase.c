static	char	*Chase_c	= "@(#)chase.c	1.7";
/*
 * Code for one object to chase another
 *
 * @(#)chase.c	3.17 (Berkeley) 6/15/81
 */

#include "rogue.h"

coord	ch_ret;				/* Where chasing takes you */

/*
 * runners:
 *	Make all the running monsters move.
 */

runners()
{
    reg1 rega1	struct	linked_list *item;
    reg2 rega2	struct	thing	*tp;

    for (item = mlist; item != NULL; item = next(item)) {
	tp = (struct thing *) ldata(item);
	if (off(*tp, ISHELD) && on(*tp, ISRUN)) {
	    if (off(*tp, ISSLOW) || tp->t_turn)
		do_chase(tp);
	    if (on(*tp, ISHASTE))
		do_chase(tp);
	    tp->t_turn ^= TRUE;
	}
    }
}

/*
 * do_chase:
 *	Make one thing chase another.
 */

do_chase(th)
reg1 rega1	struct	thing	*th;
{
				/* room of chaser, room of chasee */
    reg2 rega1	struct	room	*rer;
    reg3 rega2	struct	room	*ree;
    reg4 regd1	int	mindist = 32767;
    reg5 regd2	int	i;
    reg6 regd3	int	dist;
    reg7 regd4	bool	stoprun = FALSE;/* TRUE means we are there */
    reg8 rega5	char	sch;
		coord	this;		/* Temporary destination for chaser */

    rer = roomin(&th->t_pos);	/* Find room of chaser */
    ree = roomin(th->t_dest);	/* Find room of chasee */
    /*
     * We don't count doors as inside rooms for this routine
     */
    if (mvwinch(stdscr, th->t_pos.y, th->t_pos.x) == door)
	rer = NULL;
#ifdef	m68000
	this.x = th->t_dest->x;
	this.y = th->t_dest->y;
#else
	this = *(th->t_dest);		/* won't work for buggy MIT 68k ! */
#endif
    /*
     * If the object of our desire is in a different room, 
     * than we are and we are not in a corridor, run to the
     * door nearest to our goal.
     */
    if (rer != NULL && rer != ree)
	for (i = 0; i < rer->r_nexits; i++)	/* loop through doors */ {
	    dist = DISTANCE(th->t_dest->y, th->t_dest->x,
	      rer->r_exit[i].y, rer->r_exit[i].x);
	    if (dist < mindist) {		/* minimize distance */
		this = rer->r_exit[i];
		mindist = dist;
	    }
	}
    /*
     * this now contains what we want to run to this time
     * so we run to it.  If we hit it we either want to fight it
     * or stop running
     */
    if (!chase(th, &this)) {
	if (ce(this, hero)) {
	    attack(th);
	    return;
	} else if (th->t_type != 'F')
	    stoprun = TRUE;
    } else if (th->t_type == 'F')
	return;
#ifdef	COLOR
    Dcolor(cw, th->t_oldch);
#endif
    mvwaddch(cw, th->t_pos.y, th->t_pos.x, th->t_oldch);
#ifdef	COLOR
    acolor(cw, Dforeground, Dbackground);
#endif
    sch = mvwinch(cw, ch_ret.y, ch_ret.x);
    if (rer != NULL && (rer->r_flags & ISDARK) && sch == floor
      && DISTANCE(ch_ret.y, ch_ret.x, th->t_pos.y, th->t_pos.x) < 3
      && off(player, ISBLIND))
	th->t_oldch = ' ';
    else
	th->t_oldch = sch;

    if (cansee(unc(ch_ret)) && !on(*th, ISINVIS)) {
#ifdef	COLOR
	Dcolor(cw, th->t_type);
#endif
        mvwaddch(cw, ch_ret.y, ch_ret.x, th->t_type);
#ifdef	COLOR
	acolor(cw, Dforeground, Dbackground);
#endif
    }
    mvwaddch(mw, th->t_pos.y, th->t_pos.x, ' ');
    mvwaddch(mw, ch_ret.y, ch_ret.x, th->t_type);
    th->t_pos = ch_ret;
    /*
     * And stop running if need be
     */
    if (stoprun && ce(th->t_pos, *(th->t_dest)))
	th->t_flags &= ~ISRUN;
}

/*
 * runto:
 *	Set a monster running after something
 *	or stop it from running (for when it dies)
 */

runto(runner, spot)
reg1 rega1	coord	*runner;
		coord	*spot;
{
    reg2 rega2	struct	linked_list *item;
    reg3 rega3	struct	thing	*tp;

    /*
     * If we couldn't find him, something is funny
     */
    if ((item = find_mons(runner->y, runner->x)) == NULL)
	msg("CHASER '%s'", unctrl(winat(runner->y, runner->x)));
    tp = (struct thing *) ldata(item);
    /*
     * Start the beastie running
     */
    tp->t_dest = spot;
    tp->t_flags |= ISRUN;
    tp->t_flags &= ~ISHELD;
}

/*
 * chase:
 *	Find the spot for the chaser(er) to move closer to the
 *	chasee(ee).  Returns TRUE if we want to keep on chasing later
 *	FALSE if we reach the goal.
 */

chase(tp, ee)
struct	thing	*tp;
coord	*ee;
{
    reg1 regd1	int	x;
    reg2 regd2	int	y;
    reg3 regd3	int	dist;
    reg4 regd4	int	thisdist;
    reg5 rega1	struct	linked_list *item;
    reg6 rega2	struct	object	*obj;
    reg7 rega3	coord	*er = &tp->t_pos;
    reg8 regd5	char	ch;
    reg9 regd6	int	ey;
         regd7	int	ex;

    /*
     * If the thing is confused, let it move randomly. Invisible
     * Stalkers are slightly confused all of the time, and bats are
     * quite confused all the time
     */
    if ((on(*tp, ISHUH) && rnd(10) < 8) || (tp->t_type == 'I' && rnd(100) < 20)
      || (tp->t_type == 'B' && rnd(100) < 50)) {
	/*
	 * get a valid random move
	 */
	ch_ret = *rndmove(tp);
	dist = DISTANCE(ch_ret.y, ch_ret.x, ee->y, ee->x);
	/*
	 * Small chance that it will become un-confused 
	 */
	if (rnd(1000) < 50)
	    tp->t_flags &= ~ISHUH;
    } else {
    /*
     * Otherwise, find the empty spot next to the chaser that is
     * closest to the chasee.
     */
	/*
	 * This will eventually hold where we move to get closer
	 * If we can't find an empty spot, we stay where we are.
	 */
	dist = DISTANCE(er->y, er->x, ee->y, ee->x);
	ch_ret = *er;

	ey = er->y + 1;
	ex = er->x + 1;
	for (x = er->x - 1; x <= ex; x++)
	    for (y = er->y - 1; y <= ey; y++) {
		coord	tryp;

		tryp.x = x;
		tryp.y = y;
		if (!diag_ok(er, &tryp))
		    continue;
		ch = winat(y, x);
		if (step_ok(ch)) {
		    /*
		     * If it is a scroll, it might be a scare monster scroll
		     * so we need to look it up to see what type it is.
		     */
		    if (ch == SCROLL) {
			for (item = lvl_obj; item != NULL; item = next(item)) {
			    obj = (struct object *) ldata(item);
			    if (y == obj->o_pos.y && x == obj->o_pos.x)
				break;
			}
			if (item != NULL && obj->o_which == S_SCARE)
			    continue;
		    }
		    /*
		     * If we didn't find any scrolls at this place or it
		     * wasn't a scare scroll, then this place counts
		     */
		    thisdist = DISTANCE(y, x, ee->y, ee->x);
		    if (thisdist < dist) {
			ch_ret = tryp;
			dist = thisdist;
		    }
		}
	    }
    }
    return (dist != 0);
}

/*
 * roomin:
 *	Find what room some coordinates are in. NULL means they aren't
 *	in any room.
 */

struct	room	*
roomin(cp)
reg1 rega1	coord	*cp;
{
    reg2 rega2	struct	room	*rp;

    for (rp = rooms; rp < &rooms[MAXROOMS]; rp++)
	if (inroom(rp, cp))
	    return rp;
    return NULL;
}

/*
 * find_mons:
 *	Find the monster from his corrdinates
 */

struct	linked_list *
find_mons(y, x)
reg1 regd1	int	y;
		int	x;
{
    reg2 rega1	struct	linked_list *item;
    reg3 rega2	struct	thing	*th;

    for (item = mlist; item != NULL; item = next(item)) {
	th = (struct thing *) ldata(item);
	if (th->t_pos.y == y && th->t_pos.x == x)
	    return item;
    }
    return NULL;
}

/*
 * diag_ok:
 *	Check to see if the move is legal if it is diagonal
 */

diag_ok(sp, ep)
reg1 rega1	coord	*sp;
reg2 rega2	coord	*ep;
{
    if (ep->x == sp->x || ep->y == sp->y)
	return TRUE;
    return (step_ok(mvinch(ep->y, sp->x)) && step_ok(mvinch(sp->y, ep->x)));
}

/*
 * cansee:
 *	returns true if the hero can see a certain coordinate.
 */

cansee(y, x)
reg1 regd1	int	y;
reg2 regd2	int	x;
{
    reg3 rega1	struct	room	*rer;
		coord	tp;

    if (on(player, ISBLIND))
	return FALSE;
    tp.y = y;
    tp.x = x;
    rer = roomin(&tp);
    /*
     * We can only see if the hero in the same room as
     * the coordinate and the room is lit or if it is close.
     */
    return (rer != NULL && rer == roomin(&hero) && !(rer->r_flags&ISDARK)) ||
      DISTANCE(y, x, hero.y, hero.x) < 3;
}
