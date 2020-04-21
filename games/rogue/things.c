static	char	*Things_c	= "@(#)things.c	1.14";
/*
 * Contains functions for dealing with things like
 * potions and scrolls
 *
 * @(#)things.c	3.37 (Berkeley) 6/15/81
 */

#include <ctype.h>
#include "rogue.h"

/*
 * inv_name:
 *	return the name of something as it would appear in an
 *	inventory.
 */
char	*
inv_name(obj, drop)
reg1 rega1	struct	object	*obj;
reg1 regd1	bool	drop;
{
    reg1 rega2	char	*pb;

    switch (obj->o_type) {
	case SCROLL:
	    if (obj->o_count == 1)
		strcpy(prbuf, "A scroll ");
	    else
		sprintf(prbuf, "%d scrolls ", obj->o_count);
	    pb = &prbuf[strlen(prbuf)];
	    if (on(player, ISBLIND) && 0)
		*--pb = '.';
	    else {
		if (s_know[obj->o_which])
		    sprintf(pb, "of %s", s_magic[obj->o_which].mi_name);
	        else if (s_guess[obj->o_which])
		    sprintf(pb, "called %s", s_guess[obj->o_which]);
	        else
		    sprintf(pb, "titled '%s'", s_names[obj->o_which]);
	    }
        when POTION:
	    if (obj->o_count == 1)
		strcpy(prbuf, "A potion ");
	    else
		sprintf(prbuf, "%d potions ", obj->o_count);
	    pb = &prbuf[strlen(prbuf)];
	    if (on(player, ISBLIND) && obj->o_which != P_BLIND && 0)
		*--pb = '\0';
	    else {
	        if (p_know[obj->o_which])
		    sprintf(pb, "of %s(%s)", p_magic[obj->o_which].mi_name,
		      p_colors[obj->o_which]);
	        else if (p_guess[obj->o_which])
		    sprintf(pb, "called %s(%s)", p_guess[obj->o_which],
		      p_colors[obj->o_which]);
	        else if (obj->o_count == 1)
		    sprintf(prbuf, "A%s %s potion",
		      vowelstr(p_colors[obj->o_which]),
		      p_colors[obj->o_which]);
	        else
		    sprintf(prbuf, "%d %s potions", obj->o_count,
		      p_colors[obj->o_which]);
	    }
	when FOOD:
	    if (obj->o_which == 1)
		if (obj->o_count == 1)
		    sprintf(prbuf, "A%s %s", vowelstr(fruit), fruit);
		else
		    sprintf(prbuf, "%d %ss", obj->o_count, fruit);
	    else
		if (obj->o_count == 1)
		    strcpy(prbuf, "Some food");
		else
		    sprintf(prbuf, "%d rations of food", obj->o_count);
	when WEAPON:
	    if (obj->o_count > 1)
		sprintf(prbuf, "%d ", obj->o_count);
	    else
		strcpy(prbuf, "A ");
	    pb = &prbuf[strlen(prbuf)];
	    if (obj->o_flags & ISKNOW)
		sprintf(pb, "%s %s", num(obj->o_hplus, obj->o_dplus),
		    w_names[obj->o_which]);
	    else
		sprintf(pb, "%s", w_names[obj->o_which]);
	    if (obj->o_count > 1)
		strcat(prbuf, "s");
	when ARMOR:
	    if (obj->o_flags & ISKNOW)
		sprintf(prbuf, "%s %s Arm:%d",
		    num(a_class[obj->o_which] - obj->o_ac, 0),
		    a_names[obj->o_which],
		    obj->o_ac);
	    else
		sprintf(prbuf, "%s", a_names[obj->o_which]);
	when AMULET:
	    strcpy(prbuf, "The Amulet of Yendor");
	when WAND:
		sprintf(prbuf, "A %s ", W_type[obj->o_which]);
		if (!on(player, ISBLIND)) {
		    pb = &prbuf[strlen(prbuf)];
		    if (W_know[obj->o_which])
			sprintf(pb, "of %s%s(%s)",W_magic[obj->o_which].mi_name,
			    charge_str(obj), W_made[obj->o_which]);
		    else if (W_guess[obj->o_which])
			sprintf(pb, "called %s(%s)", W_guess[obj->o_which],
			    W_made[obj->o_which]);
		    else
			sprintf(&prbuf[2], "%s %s", W_made[obj->o_which],
			    W_type[obj->o_which]);
		}
        when RING:
	    if (on(player, ISBLIND) && 0)
		strcpy(prbuf, "A ring");
	    else {
		if (r_know[obj->o_which])
		    sprintf(prbuf, "A%s ring of %s(%s)", ring_num(obj),
			r_magic[obj->o_which].mi_name, r_rings[obj->o_which]);
		else if (r_guess[obj->o_which])
		    sprintf(prbuf, "A ring called %s(%s)",
		      r_guess[obj->o_which], r_rings[obj->o_which]);
		else
		    sprintf(prbuf,"A%s %s ring",vowelstr(r_rings[obj->o_which]),
		      r_rings[obj->o_which]);
	    }
	otherwise:
	    debug("Picked up something funny");
	    sprintf(prbuf, "Something bizarre %s", unctrl(obj->o_type));
    }
    if (obj == cur_armor)
	strcat(prbuf, " (being worn)");
    if (obj == cur_weapon)
	strcat(prbuf, " (weapon in hand)");
    if (obj == cur_ring[LEFT])
	strcat(prbuf, " (on left hand)");
    else if (obj == cur_ring[RIGHT])
	strcat(prbuf, " (on right hand)");
    if (drop && isupper(prbuf[0]))
	prbuf[0] = tolower(prbuf[0]);
    else if (!drop && islower(*prbuf))
	*prbuf = toupper(*prbuf);
    if (!drop)
	strcat(prbuf, ".");
    return prbuf;
}

/*
 * hand:
 *	return a string indicating which hand a ring is on
 */
char	*
hand(obj)
reg1 rega1	struct	object	*obj;
{
    if (obj == cur_ring[LEFT])
	return " (on left hand)";
    return " (on right hand)";
}

/*
 * money:
 *	Add to characters purse
 */
money()
{
    reg1 rega1	struct	room	*rp;

#ifdef	COLOR
    Dcolor(cw,GOLD);
#endif
    for (rp = rooms; rp < &rooms[MAXROOMS]; rp++)
	if (ce(hero, rp->r_gold)) {
	    if (notify) {
		msg("%s%d gold pieces.", terse?"":"You found ", rp->r_goldval);
	    }
	    purse += rp->r_goldval;
	    rp->r_goldval = 0;
	    Cmov(rp->r_gold);
	    addch(floor);
	    return;
	}
    msg("That gold must have been counterfeit");
}

/*
 * drop:
 *	put something down
 */
drop()
{
    reg1 regd1	char	ch;
    reg1 rega1	struct	linked_list *obj;
    reg1 rega2	struct	linked_list *nobj;
    reg1 rega3	struct	object	*op;

    ch = mvwinch(stdscr, hero.y, hero.x);
    if (ch != floor && ch != passage) {
	msg("There is something there already");
	return;
    }
    if ((obj = get_item("drop", 0)) == NULL)
	return;
    op = (struct object *) ldata(obj);
    if (!dropcheck(op))
	return;
    /*
     * Take it out of the pack
     */
    if (op->o_count >= 2 && op->o_type != WEAPON) {
	nobj = new_item(sizeof *op);
	op->o_count--;
	op = (struct object *) ldata(nobj);
	*op = *((struct object *) ldata(obj));
	op->o_count = 1;
	obj = nobj;
	if (op->o_group != 0)
		inpack++;
    } else
	detach(pack, obj);
    inpack--;
    /*
     * Link it into the level object list
     */
    attach(lvl_obj, obj);
    mvaddch(hero.y, hero.x, op->o_type);
    op->o_pos = hero;
    msg("Dropped %s", inv_name(op, TRUE));
}

/*
 * do special checks for dropping or unweilding|unwearing|unringing
 */
dropcheck(op)
reg1 rega1	struct	object	*op;
{
    str_t save_max;

    if (op == NULL)
	return TRUE;
    if (op != cur_armor && op != cur_weapon
	&& op != cur_ring[LEFT] && op != cur_ring[RIGHT])
	    return TRUE;
    if (op->o_flags & ISCURSED) {
	msg("You can't.  It appears to be cursed.");
	return FALSE;
    }
    if (op == cur_weapon)
	cur_weapon = NULL;
    else if (op == cur_armor) {
	waste_time();
	cur_armor = NULL;
    } else if (op == cur_ring[LEFT] || op == cur_ring[RIGHT]) {
	switch (op->o_which) {
	    case R_ADDSTR:
		save_max = max_stats.s_str;
		chg_str(-op->o_ac);
		max_stats.s_str = save_max;
		break;
	    case R_SEEINVIS:
		player.t_flags &= ~CANSEE;
		extinguish(unsee);
		light(&hero);
#ifdef	COLOR
		Dcolor(cw, PLAYER);
#endif
		mvwaddch(cw, hero.y, hero.x, PLAYER);
#ifdef	COLOR
		acolor(cw, Dforeground, Dbackground);
#endif
		break;
	}
	cur_ring[op == cur_ring[LEFT] ? LEFT : RIGHT] = NULL;
    }
    return TRUE;
}

/*
 * return a new thing
 */
struct	linked_list *
new_thing()
{
    reg1 rega1	struct	linked_list *item;
    reg1 rega2	struct	object	*cur;
    reg1 regd1	int	j;
    reg1 regd2	int	k;

    item = new_item(sizeof *cur);
    cur = (struct object *) ldata(item);
    cur->o_hplus = cur->o_dplus = 0;
    cur->o_damage = cur->o_hurldmg = "0d0";
    cur->o_ac = 11;
    cur->o_count = 1;
    cur->o_group = 0;
    cur->o_flags = 0;
    /*
     * Decide what kind of object it will be
     * If we haven't had food for a while, let it be food.
     */
    switch (no_food > 3 ? 2 : pick_one(things, NUMTHINGS)) {
	case 0:
	    cur->o_type = POTION;
	    cur->o_which = pick_one(p_magic, MAXPOTIONS);
	when 1:
	    cur->o_type = SCROLL;
	    cur->o_which = pick_one(s_magic, MAXSCROLLS);
	when 2:
	    no_food = 0;
	    cur->o_type = FOOD;
	    if (rnd(100) > 10)
		cur->o_which = 0;
	    else
		cur->o_which = 1;
	when 3:
	    cur->o_type = WEAPON;
	    cur->o_which = rnd(MAXWEAPONS);
	    init_weapon(cur, cur->o_which);
	    if ((k = rnd(100)) < 10) {
		cur->o_flags |= ISCURSED;
		cur->o_hplus -= rnd(3)+1;
	    } else if (k < 15)
		cur->o_hplus += rnd(3)+1;
	when 4:
	    cur->o_type = ARMOR;
	    for (j = 0, k = rnd(100); j < MAXARMORS; j++)
		if (k < a_chances[j])
		    break;
	    if (j == MAXARMORS) {
		debug("Picked a bad armor %d", k);
		j = 0;
	    }
	    cur->o_which = j;
	    cur->o_ac = a_class[j];
	    if ((k = rnd(100)) < 20) {
		cur->o_flags |= ISCURSED;
		cur->o_ac += rnd(3)+1;
	    }
	    else if (k < 28)
		cur->o_ac -= rnd(3)+1;
	when 5:
	    cur->o_type = RING;
	    cur->o_which = pick_one(r_magic, MAXRINGS);
	    switch (cur->o_which) {
		case R_ADDSTR:
		case R_PROTECT:
		case R_ADDHIT:
		case R_ADDDAM:
		    if ((cur->o_ac = rnd(3)) == 0) {
			cur->o_ac = -1;
			cur->o_flags |= ISCURSED;
		    }
		when R_AGGR:
		case R_TELEPORT:
		    cur->o_flags |= ISCURSED;
	    }
	when 6:
	    cur->o_type = WAND;
	    cur->o_which = pick_one(W_magic, MAXWANDS);
	    fix_stick(cur);
	otherwise:
	    debug("Picked a bad kind of object");
	    wait_for(' ');
    }
    return item;
}

/*
 * pick an item out of a list of nitems possible magic items
 */
pick_one(magic, nitems)
reg1 rega1	struct	magic_item *magic;
int	nitems;
{
    reg1 rega2	struct	magic_item *end;
    reg1 regd1	int	i;
    reg1 rega3	struct	magic_item *start;

    start = magic;
    for (end = &magic[nitems], i = rnd(100); magic < end; magic++)
	if (i < magic->mi_prob)
	    break;
    if (magic == end) {
	if (wizard) {
	    msg("bad pick_one: %d from %d items", i, nitems);
	    for (magic = start; magic < end; magic++)
		msg("%s: %d%%", magic->mi_name, magic->mi_prob);
	}
	magic = start;
    }
    return magic - start;
}

/*
 * do_amulet:
 *	He found The Amulet of Yendor!
 */

do_amulet()
{
#ifdef	COLOR
	Dcolor(cw,AMULET);
#else
	wstandout(cw);
#endif
	msg("Congratulations, You have found");
#ifdef	COLOR
	Dcolor(cw,AMULET);
#else
	wstandout(cw);
#endif
	msg("The Amulet of Yendor!!!");
	wstandend(cw);
}

/*
 * point wand in direction
 */

#ifdef	ROGUE2.6

point()
{
    uchar	ch;
    reg1 rega1	struct	object	*obj;
    reg1 rega2	struct	linked_list *item;
    coord	dir;
    coord	path;
    coord	was;
    struct	object	wand;
    uchar	wasch;	/* char	that was there before zapping */

    if ((item = get_item("point", WAND)) == NULL)
        return;
    obj = (struct object *) ldata(item);
    if (obj->o_type != WAND) {
        msg("You can't point that.");
        return;
    }
    msg("what direction?");
    mpos = 0;
    switch (readchar()) {
      case 'h' : dir.y =  0; dir.x = -1;
      when 'j' : dir.y =  1; dir.x =  0;
      when 'k' : dir.y = -1; dir.x =  0;
      when 'l' : dir.y =  0; dir.x =  1;
      when 'y' : dir.y = -1; dir.x = -1;
      when 'u' : dir.y = -1; dir.x =  1;
      when 'b' : dir.y =  1; dir.x = -1;
      when 'n' : dir.y =  1; dir.x =  1;
      otherwise: msg("bad direction"); return;
    }
    if (obj->o_ac > 0 || wizard) {	/* o_ac used for count of charges */
	path = hero;
	was.x = hero.x + dir.x;
	was.y = hero.y + dir.y;
	wasch = winat(was.y,was.x);
	debug("hero=%d,%d",hero.x,hero.y);
	while (1) {
	    path.x += dir.x;
	    path.y += dir.y;
	    debug("path=%d,%d",path.x,path.y);
	    if (path.x < 0 || path.x >= COLS || path.y < 0 || path.y >= LINES)
		break;
				/* put back what bolt was over */
	    wmove(cw,was.y,was.x);
	    debug("was=%d,%d  bolt over:'%c' wasch:'%c'",
	      was.x,was.y,winat(was.y,was.x),wasch);
/*	    waddch(cw,winat(was.y,was.x)); */
#ifdef	COLOR
	    Dcolor(cw, wasch);
#endif
	    waddch(cw,wasch);
	    was.x = path.x;
	    was.y = path.y;
				/* see what is where we are going */
	    ch = winat(path.y,path.x);
	    wasch = ch;
	    debug("hitting '%c' with our '%c'",ch,ZAP);
	    if (!iswall(ch) && ch != SECRETDOOR && (ch < 'A' || ch > 'Z')) {
		wmove(cw,path.y,path.x);
#ifdef	COLOR
		Dcolor(cw, ZAP);
#endif
		waddch(cw,ZAP);
#ifdef	COLOR
		acolor(cw, Dforeground,Dbackground);
#endif
		Draw(cw);
		tick(8);
		continue;
	    }
	    if (ch < 'A' || ch > 'Z')
		break;
	    switch (obj->o_which) {
	      case W_POLYMORPH:
		debug("Zapping a '%c' with our Polymorph",ch);
		w_know[obj->o_which] = TRUE;
		if (ch >= 'A' && ch <= 'Z') {
		    struct	linked_list *item;
		    struct	thing	*tp;

				/*
				 * Find the monster we want to fight
				 */
		    if ((item = find_mons(path.y, path.x)) == NULL)
			debug("Point at what @ %d,%d", path.y, path.x);
		    tp = (struct thing *) ldata(item);
				/*
				 * Get rid of the monster.
				 */
/*
 *		    for (pitem = tp->t_pack; pitem != NULL;
 *		      pitem = next(pitem)) {
 *			struct	object	*obj = (struct object *) ldata(pitem);
 *
 *			obj->o_pos = tp->t_pos;
 *			detach(tp->t_pack, pitem);
 *			fall(pitem, FALSE);
 *		    }
 */
		    remove(&path, item);
/*		    life = FALSE; */

				/*
				 * Create new monster.
				 */
		    item = new_item(sizeof *tp);
		    new_monster(item, randmonster(TRUE), &path);
		    tp = (struct thing *) ldata(item);
		    tp->t_flags |= ISRUN;
		    tp->t_pos = path;
		    tp->t_dest = &hero;
		}
		break;
	      case W_LIGHTNING:
		debug("Zapping a '%c' with our Lightning",ch);
		w_know[obj->o_which] = TRUE;
		wand.o_damage = "3d6";
		wand.o_hplus = 10000;
		wand.o_dplus = 0;
		wand.o_flags = 0;
		fight(&path,ch,&wand,FALSE);
		break;
	      default:
		debug("Zapping a '%c' with our default",ch);
		msg("it backfires");
		break;
	    }
	    break;
	}
	obj->o_ac--;
    } else
	msg("no charges left");
    waste_time();
}

#endif	ROGUE2.6

/* tick() sleeps for specified number of clock ticks */
tick(ticks)
unsigned ticks;
{
	UNSIGN u;

	u = 0;
	profil((char	*)&u,sizeof u,0,2);
	while (u < ticks)	/* system will increment u each clock tick */
		continue;
	profil(&u,sizeof u,0,0);/* turn off profiling */
}

/*
 * put_things:
 *	put potions and scrolls and wands and rings on this level
 */

#ifdef	ROGUE26

put_things()
{
    reg1 regd1	int	i;
    reg1 regd2	int	j;
    reg1 regd3	int	k;
    reg1 rega1	struct	linked_list *item;
    reg1 rega2	struct	object	*cur;
    reg1 regd4	int	rm;
    coord	tp;

    /*
     * Throw away stuff left on the previous level (if anything)
     */
    free_list(lvl_obj);
    /*
     * Do MAXOBJ attempts to put things on a level
     */
    m_amulet = FALSE;		/* not made on this level */
    for (i = 0; i < MAXOBJ; i++) {
	if (!m_amulet && (wizard || rnd(31*31) < level*level && level > 24)) {
	    m_amulet = TRUE;
	    item = new_item(sizeof *cur);
	    attach(lvl_obj, item);
	    cur = (struct object *) ldata(item);
	    cur->o_hplus = cur->o_dplus = 0;
	    cur->o_damage = cur->o_hurldmg = "0d0";
	    cur->o_ac = 11;
	    cur->o_type = AMULET;
	    cur->o_which = 0;
	    /*
	     * Put it somewhere
	     */
	    rm = rnd_room();
	    do
		    rnd_pos(&rooms[rm], &tp);
	    until(winat(tp.y, tp.x) == floor);
	    mvaddch(tp.y, tp.x, cur->o_type);
	    cur->o_pos = tp;
	} else {
		if (rnd(100) < 35) {
		    /*
		     * Pick a new object and link it in the list
		     */
		    item = new_item(sizeof *cur);
		    attach(lvl_obj, item);
		    cur = (struct object *) ldata(item);
		    cur->o_hplus = cur->o_dplus = 0;
		    cur->o_damage = cur->o_hurldmg = "0d0";
		    cur->o_ac = 11;
		    /*
		     * Decide what kind of object it will be
		     * If we haven't had food for a while, let it be food.
		     */
		    switch(no_food > 3 ? 4 : rnd(10)) {
			case 0:
			case 5:
			    cur->o_type = POTION;
			    for (j = 0, k = rnd(100); j < MAXPOTIONS; j++)
				if (k < p_magic[j].mi_prob)
				    break;
			    cur->o_which = j;
			    break;
			case 2:
			case 3:
			    cur->o_type = SCROLL;
			    for (j = 0, k = rnd(100); j < MAXSCROLLS; j++)
				if (k < s_magic[j].mi_prob)
				    break;
			    cur->o_which = j;
			    break;
			case 1:
			    cur->o_type = WAND;
			    for (j = 0, k = rnd(100); j < MAXWANDS; j++)
				if (k < w_magic[j].mi_prob)
				    break;
			    cur->o_which = j;
			    cur->o_ac = rnd(6) + 1;	/* number of charges */
			    break;
			case 6:
			    cur->o_type = RING;
			    for (j = 0, k = rnd(100); j < MAXRINGS; j++)
				if (k < r_magic[j].mi_prob)
				    break;
			    cur->o_which = j;
			    if ((k = rnd(100)) < 10) {
				cur->o_flags |= ISCURSED;
				cur->o_which = -1;
			    } else
				cur->o_which = 1;
			    break;
			case 4:
			case 7:
			    no_food = 0;
			    cur->o_type = FOOD;
			    if (rnd(100) > 10)
				cur->o_which = 0;
			    else if (rnd(100) > 50)
				cur->o_which = 1;
			    else
				cur->o_which = 2;
			    break;
			case 8:
			    cur->o_type = WEAPON;
			    cur->o_which = rnd(MAXWEAPONS);
			    init_weapon(cur, cur->o_which);
			    if ((k = rnd(100)) < 10) {
				cur->o_flags |= ISCURSED;
				cur->o_hplus -= rnd(3)+1;
			    } else
				if (k < 15)
				    cur->o_hplus += rnd(3)+1;
			    break;
			case 9:
			    cur->o_type = ARMOR;
			    for (j = 0, k = rnd(100); j < MAXARMORS; j++)
				if (k < a_chances[j])
				    break;
			    if (j == MAXARMORS) {
				debug("Picked a bad armor %d", k);
				j = 0;
			    }
			    cur->o_which = j;
			    cur->o_ac = a_class[j];
			    if ((k = rnd(100)) < 20) {
				cur->o_flags |= ISCURSED;
				cur->o_ac += rnd(3)+1;
			    } else
				if (k < 28)
				    cur->o_ac -= rnd(3)+1;
			otherwise:
			    debug("Picked a bad kind of object");
			    wait_for(' ');
		    }
		    /*
		     * Put it somewhere
		     */
		    rm = rnd_room();
		    do
			rnd_pos(&rooms[rm], &tp);
		    until(winat(tp.y, tp.x) == floor);
		    mvaddch(tp.y, tp.x, cur->o_type);
		    cur->o_pos = tp;
		}
	}
    }
}

#endif	ROGUE26

/*
 * inventory:
 *	list what is in the pack
 */

#ifdef	notdef

inventory(list, type)
struct	linked_list *list;
int	type;
{
    reg1 rega1	struct	object	*obj;
    reg1 regd1	char	ch;
    reg1 rega2	struct	linked_list *l;
    reg1 regd2	int	n_objs;
    struct	object	*t_obj;
    char	t_ch;

    n_objs = 0;
    for (ch = 'a', l=list; ch <= 'a'+MAXPACK-1 && l != NULL;
      l = next(l), ch++) {
	obj = (struct object *) ldata(l);
	if (type && type != obj->o_type)
	    continue;
	n_objs++;
	t_obj = obj;
	t_ch  = ch;
	if (slow_invent) {
#ifdef	COLOR
	    Dcolor(cw, obj->o_type);
#else
	    if (obj->o_type == AMULET)
		wstandout(cw);
#endif
	    msg("%c) %s\n", ch, inv_name(obj, FALSE));
	    if (obj->o_type == AMULET)
		wstandend(cw);
	    return 1;
	}
    }
    if (n_objs == 0) {
#ifdef	COLOR
	acolor(cw, C_cyan, Dbackground);
#endif
#ifdef	iris
	msg("You are empty handed.");
#else
	msg(terse == 0 ? "You are empty handed." :
			"You don't have anything appropriate");
#endif
	return 0;
    }
    if (slow_invent)
	return 1;
    if (n_objs == 1) {
#ifdef	COLOR
	Dcolor(cw, t_obj->o_type);
#else
	if (t_obj->o_type == AMULET)
	    wstandout(cw);
#endif
	msg("%c) %s\n", t_ch, inv_name(t_obj, FALSE));
	if (t_obj->o_type == AMULET)
	    wstandend(cw);
	return 1;
    }
    wclear(hw);
#ifdef	iris
    wmove(hw,1,0);
#endif
    for (ch = 'a'; ch <= 'a'+MAXPACK-1 && list != NULL;
      list = next(list), ch++) {
	obj = (struct object *) ldata(list);
	if (type && type != obj->o_type)
	    continue;
	wprintw(hw, "%c) ", ch);
#ifdef	COLOR
	Dcolor(hw, obj->o_type);
#else
	if (obj->o_type == AMULET)
	    wstandout(hw);
#endif
	wprintw(hw, "%s", inv_name(obj, FALSE));
#ifdef	COLOR
	acolor(hw, Dforeground, Dbackground);
#else
	if (obj->o_type == AMULET)
	    wstandend(hw);
#endif
	wprintw(hw, "\n");
    }
#ifdef	COLOR
    acolor(hw, Dforeground, Dbackground);
#endif
#ifdef	iris
    mvwaddstr(hw, 0, 0, "Press space to continue");
#else
    mvwaddstr(hw, LINES-1, 0, "--Press space to continue--");
#endif
    Draw(hw);
    wait_prompt(hw, ' ');
    clearok(cw, TRUE);
    touchwin(cw);
    return 1;
}

#endif notdef


/*
 * find_obj:
 *	find the unclaimed object at y, x
 */

struct	linked_list *
find_obj(y, x)
reg1 regd1	int	y;
int	x;
{
    reg1 rega1	struct	linked_list *obj;
    reg1 rega2	struct	object	*op;

    for(obj = lvl_obj; obj != NULL; obj = next(obj)) {
	op = (struct object *) ldata(obj);
	if (op->o_pos.y == y && op->o_pos.x == x)
		return obj;
    }
    sprintf(prbuf, "Non-object %d,%d", y, x);
    debug(prbuf);
    return NULL;
}
