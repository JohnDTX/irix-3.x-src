static	char	*Sticks	= "@(#)wands.c	1.4 10/9/84";
/*
 * Functions to implement the various sticks one might find
 * while wandering around the dungeon.
 *
 * @(#)sticks.c	3.14 (Berkeley) 6/15/81
 * @(#)wands.c	1.4 10/9/84 23:46:06
 */

#include <ctype.h>
#include "rogue.h"

fix_stick(cur)
reg1 rega1	struct	object	*cur;
{
    if (strcmp(W_type[cur->o_which], "wand") == 0)
	cur->o_damage = "2d3";
    else
	cur->o_damage = "1d1";
    cur->o_hurldmg = "1d1";

    cur->o_charges = 3 + rnd(5);
    switch (cur->o_which) {
	case WS_HIT:
	    cur->o_hplus = 3;
	    cur->o_dplus = 3;
	    cur->o_damage = "1d8";
	when WS_LIGHT:
	    cur->o_charges = 10 + rnd(10);
    }
}

do_zap(gotdir)
bool	gotdir;
{
    reg1 rega1	struct	linked_list *item;
    reg2 rega2	struct	object	*obj;
    reg3 rega3	struct	room	*rp;
    reg4 rega4	struct	thing	*tp;
    reg5 regd1	int	y;
    reg6 regd2	int	x;

    if ((item = get_item("zap with", WAND)) == NULL)
	return;
    obj = (struct object *) ldata(item);
    if (obj->o_type != WAND) {
	msg("You can't zap with that!");
	after = FALSE;
	return;
    }
    if (obj->o_charges == 0) {
	msg("Nothing happens.");
	return;
    }
    if (!gotdir)
	do {
	    delta.y = rnd(3) - 1;
	    delta.x = rnd(3) - 1;
	} while (delta.y == 0 && delta.x == 0);
    switch (obj->o_which) {
	case WS_LIGHT:
	    /*
	     * Ready Kilowatt wand.  Light up the room
	     */
	    W_know[WS_LIGHT] = TRUE;
	    lights();
	when WS_DRAIN:
	    /*
	     * Take away 1/2 of hero's hit points, then take it away
	     * evenly from the monsters in the room (or next to hero
	     * if he is in a passage)
	     */
	    if (pstats.s_hpt < 2) {
		msg("You are too weak to use it.");
		return;
	    }
	    else if ((rp = roomin(&hero)) == NULL)
		drain(hero.y-1, hero.y+1, hero.x-1, hero.x+1);
	    else
		drain(rp->r_pos.y, rp->r_pos.y+rp->r_max.y,
		    rp->r_pos.x, rp->r_pos.x+rp->r_max.x);
	when WS_POLYMORPH:
	case WS_TELAWAY:
	case WS_TELTO:
	case WS_CANCEL:
	{
	    char	monster, oldch;
	    int	rm;

	    y = hero.y;
	    x = hero.x;
	    while (step_ok(winat(y, x))) {
		y += delta.y;
		x += delta.x;
	    }
	    if (isupper(monster = mvwinch(mw, y, x))) {
		char	omonst = monster;

		if (monster == 'F')
		    player.t_flags &= ~ISHELD;
		item = find_mons(y, x);
		tp = (struct thing *) ldata(item);
		if (obj->o_which == WS_POLYMORPH) {
		    detach(mlist, item);
		    oldch = tp->t_oldch;
		    delta.y = y;
		    delta.x = x;
		    new_monster(item, monster = rnd(26) + 'A', &delta);
		    if (!(tp->t_flags & ISRUN))
			runto(&delta, &hero);
		    if (isupper(mvwinch(cw, y, x)))
			mvwaddch(cw, y, x, monster);
		    tp->t_oldch = oldch;
		    W_know[WS_POLYMORPH] |= (monster != omonst);
		}
		else if (obj->o_which == WS_CANCEL) {
		    tp->t_flags |= ISCANC;
		    tp->t_flags &= ~ISINVIS;
		} else {
		    if (obj->o_which == WS_TELAWAY) {
			do {
			    rm = rnd_room();
			    rnd_pos(&rooms[rm], &tp->t_pos);
			} until(winat(tp->t_pos.y, tp->t_pos.x) == floor);
		    } else {
			tp->t_pos.y = hero.y + delta.y;
			tp->t_pos.x = hero.x + delta.x;
		    }
		    if (isupper(mvwinch(cw, y, x)))
			mvwaddch(cw, y, x, tp->t_oldch);
		    tp->t_dest = &hero;
		    tp->t_flags |= ISRUN;
		    mvwaddch(mw, y, x, ' ');
		    mvwaddch(mw, tp->t_pos.y, tp->t_pos.x, monster);
		    if (tp->t_pos.y != y || tp->t_pos.x != x)
			tp->t_oldch = mvwinch(cw, tp->t_pos.y, tp->t_pos.x);
		}
	    }
	}
	when WS_MISSILE:
	{
	    static	struct	object	bolt = {
		'*' , {0, 0}, "", 0, 0, "1d4" , 0, 0, 100, 1
	    };

	    do_motion(&bolt, delta.y, delta.x);
	    if (isupper(mvwinch(mw, bolt.o_pos.y, bolt.o_pos.x))
		&& !save_throw(VS_MAGIC, ldata(find_mons(unc(bolt.o_pos)))))
		    hit_monster(unc(bolt.o_pos), &bolt);
	    else if (terse)
		msg("Missle vanishes");
	    else
		msg("The missle vanishes with a puff of smoke");
	    W_know[WS_MISSILE] = TRUE;
	}
	when WS_HIT:
	{
	    char	ch;

	    delta.y += hero.y;
	    delta.x += hero.x;
	    ch = winat(delta.y, delta.x);
	    if (isupper(ch)) {
		if (rnd(20) == 0) {
		    obj->o_damage = "3d8";
		    obj->o_dplus = 9;
		} else {
		    obj->o_damage = "1d8";
		    obj->o_dplus = 3;
		}
		fight(&delta, ch, obj, FALSE);
	    }
	}
	when WS_HASTE_M:
	case WS_SLOW_M:
	    y = hero.y;
	    x = hero.x;
	    while (step_ok(winat(y, x))) {
		y += delta.y;
		x += delta.x;
	    }
	    if (isupper(mvwinch(mw, y, x))) {
		item = find_mons(y, x);
		tp = (struct thing *) ldata(item);
		if (obj->o_which == WS_HASTE_M) {
		    if (on(*tp, ISSLOW))
			tp->t_flags &= ~ISSLOW;
		    else
			tp->t_flags |= ISHASTE;
		} else {
		    if (on(*tp, ISHASTE))
			tp->t_flags &= ~ISHASTE;
		    else
			tp->t_flags |= ISSLOW;
		    tp->t_turn = TRUE;
		}
		delta.y = y;
		delta.x = x;
		runto(&delta, &hero);
	    }
	when WS_ELECT:
	case WS_FIRE:
	case WS_COLD:
	{
	    char	dirch, ch, *name;
	    bool	bounced, used;
	    coord	pos;
	    coord	spotpos[BOLT_LENGTH];
	    static	struct	object	bolt = {
		'*' , {0, 0}, "", 0, 0, "6d6" , 0, 0, 100, 0
	    };


	    switch (delta.y + delta.x) {
		case 0: dirch = '/';
		when 1: case -1: dirch = (delta.y == 0 ? '-' : '|');
		when 2: case -2: dirch = '\\';
	    }
	    pos = hero;
	    bounced = FALSE;
	    used = FALSE;
	    if (obj->o_which == WS_ELECT)
		name = "bolt";
	    else if (obj->o_which == WS_FIRE)
		name = "flame";
	    else
		name = "ice";
	    for (y = 0; y < BOLT_LENGTH && !used; y++) {
		ch = winat(pos.y, pos.x);
		spotpos[y] = pos;
		if (ch == door || ch == SECRETDOOR || ch == ' ' || iswall(ch)) {
		    bounced = TRUE;
		    delta.y = -delta.y;
		    delta.x = -delta.x;
		    y--;
		    msg("The bolt bounces");
		} else {
			if (!bounced && isupper(ch)) {
			    if (!save_throw(VS_MAGIC,
			      ldata(find_mons(unc(pos))))) {
				bolt.o_pos = pos;
				hit_monster(unc(pos), &bolt);
				used = TRUE;
			    } else if (ch != 'M' || show(pos.y, pos.x) == 'M') {
				if (terse)
				    msg("%s misses", name);
				else
				    msg("The %s whizzes past the %s", name, monsters[ch-'A'].m_name);
				runto(&pos, &hero);
			    }
			} else
			    if (bounced && pos.y == hero.y && pos.x == hero.x) {
			    	bounced = FALSE;
			    	if (!save(VS_MAGIC)) {
				    if (terse)
				    	msg("The %s hits", name);
				    else
				    	msg("You are hit by the %s", name);
				    if ((pstats.s_hpt -= roll(6, 6)) <= 0)
				    	death('b');
				    used = TRUE;
			    	} else
				    msg("The %s whizzes by you", name);
			    }
			mvwaddch(cw, pos.y, pos.x, dirch);
			Draw(cw);
		}
		pos.y += delta.y;
		pos.x += delta.x;
	    }
	    for (x = 0; x < y; x++)
		mvwaddch(cw, spotpos[x].y, spotpos[x].x, show(spotpos[x].y,
		  spotpos[x].x));
	    W_know[obj->o_which] = TRUE;
	}
	otherwise:
	    msg("What a bizarre schtick!");
    }
    obj->o_charges--;
}

/*
 * drain:
 *	Do drain hit points from player's shtick
 */

drain(ymin, ymax, xmin, xmax)
int	ymin, ymax, xmin, xmax;
{
    reg1 regd1	int	i;
    reg2 regd2	int	j;
    reg3 regd3	int	count;
    reg4 rega1	struct	thing	*ick;
    reg5 rega2	struct	linked_list *item;

    /*
     * First count how many things we need to spread the hit points among
     */
    count = 0;
    for (i = ymin; i <= ymax; i++)
	for (j = xmin; j <= xmax; j++)
	    if (isupper(mvwinch(mw, i, j)))
		count++;
    if (count == 0) {
	msg("You have a tingling feeling");
	return;
    }
    count = pstats.s_hpt / count;
    pstats.s_hpt /= 2;
    /*
     * Now zot all of the monsters
     */
    for (i = ymin; i <= ymax; i++)
	for (j = xmin; j <= xmax; j++)
	    if (isupper(mvwinch(mw, i, j)) &&
	      ((item = find_mons(i, j)) != NULL)) {
		ick = (struct thing *) ldata(item);
		if ((ick->t_stats.s_hpt -= count) < 1)
		    killed(item, cansee(i, j) && !on(*ick, ISINVIS));
	    }
}

/*
 * charge a wand for wizards.
 */
char	*
charge_str(obj)
reg1 rega1	struct	object	*obj;
{
    static	char	buf[20];

    if (!(obj->o_flags & ISKNOW))
	buf[0] = '\0';
    else if (terse)
	sprintf(buf, " [%d]", obj->o_charges);
    else
	sprintf(buf, " [%d charge%s]", obj->o_charges,
	  obj->o_charges == 1 ? "" : "s");
    return buf;
}
