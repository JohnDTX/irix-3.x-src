/*
 * Routines to deal with the pack
 *
 * @(#)pack.c	3.6 (Berkeley) 6/15/81
 * @(#)pack.c	1.4 10/9/84
 */

#include <ctype.h>
#include "rogue.h"

/*
 * add_pack:
 *	Pick up an object and add it to the pack.  If the argument is non-null
 * use it as the linked_list pointer instead of gettting it off the ground.
 */
add_pack(item, silent)
reg1 rega1	struct	linked_list *item;
bool	silent;
{
    reg2 rega2	struct	linked_list *ip;
    reg3 rega3	struct	linked_list *lp;
    reg4 rega4	struct	object	*obj;
    reg5 rega5	struct	object	*op;
    reg6 regd1	char	ch;
    reg7 regd2	bool	exact;
    reg8 regd3	bool	from_floor;

    if (item == NULL) {
	from_floor = TRUE;
	if ((item = find_obj(hero.y, hero.x)) == NULL)
	    return;
    } else
	from_floor = FALSE;
    obj = (struct object *) ldata(item);
    /*
     * Link it into the pack.  Search the pack for a object of similar type
     * if there isn't one, stuff it at the beginning, if there is, look for one
     * that is exactly the same and just increment the count if there is.
     * it  that.  Food is always put at the beginning for ease of access, but
     * is not ordered so that you can't tell good food from bad.  First check
     * to see if there is something in thr same group and if there is then
     * increment the count.
     */
    if (obj->o_group) {
	for (ip = pack; ip != NULL; ip = next(ip)) {
	    op = (struct object *) ldata(ip);
	    if (op->o_group == obj->o_group) {
		/*
		 * Put it in the pack and notify the user
		 */
		op->o_count++;
		if (from_floor) {
		    detach(lvl_obj, item);
		    mvaddch(hero.y, hero.x,
			(roomin(&hero) == NULL ? passage : floor));
		}
		discard(item);
		item = ip;
		goto picked_up;
	    }
	}
    }
    /*
     * Check if there is room
     */
    if (inpack == MAXPACK-1) {
	msg("You can't carry anything else.");
	return;
    }
    /*
     * Check for and deal with scare monster scrolls
     */
    if (obj->o_type == SCROLL && obj->o_which == S_SCARE)
	if (obj->o_flags & ISFOUND) {
#ifdef	COLOR
	    Dcolor(cw, SCROLL);
#endif
	    msg("The scroll turns to dust as you pick it up.");
	    detach(lvl_obj, item);
	    mvaddch(hero.y, hero.x, floor);
	    return;
	} else
	    obj->o_flags |= ISFOUND;

    inpack++;
    if (from_floor) {
	detach(lvl_obj, item);
	mvaddch(hero.y, hero.x, (roomin(&hero) == NULL ? passage : floor));
    }
    /*
     * Search for an object of the same type
     */
    exact = FALSE;
    for (ip = pack; ip != NULL; ip = next(ip)) {
	op = (struct object *) ldata(ip);
	if (obj->o_type == op->o_type)
	    break;
    }
    if (ip == NULL) {
	/*
	 * Put it at the end of the pack since it is a new type
	 */
	for (ip = pack; ip != NULL; ip = next(ip)) {
	    op = (struct object *) ldata(ip);
	    if (op->o_type != FOOD)
		break;
	    lp = ip;
	}
    } else {
	/*
	 * Search for an object which is exactly the same
	 */
	while (ip != NULL && op->o_type == obj->o_type) {
	    if (op->o_which == obj->o_which) {
		exact = TRUE;
		break;
	    }
	    lp = ip;
	    if ((ip = next(ip)) == NULL)
		break;
	    op = (struct object *) ldata(ip);
	}
    }
    if (ip == NULL) {
	/*
	 * Didn't find an exact match, just stick it here
	 */
	if (pack == NULL)
	    pack = item;
	else {
	    lp->l_next = item;
	    item->l_prev = lp;
	    item->l_next = NULL;
	}
    } else {
	/*
	 * If we found an exact match.  If it is a potion, food, or a 
	 * scroll, increase the count, otherwise put it with its clones.
	 */
	if (exact && ISMULT(obj->o_type)) {
	    op->o_count++;
	    discard(item);
	    item = ip;
	    goto picked_up;
	}
	if ((item->l_prev = prev(ip)) != NULL)
	    item->l_prev->l_next = item;
	else
	    pack = item;
	item->l_next = ip;
	ip->l_prev = item;
    }
picked_up:
    /*
     * Notify the user
     */
    obj = (struct object *) ldata(item);
    if (notify && !silent) {
#ifdef	COLOR
	Dcolor(cw, obj->o_type);
#endif
#ifdef	iris
	plusstr = "Plus ";
#endif
	msg("%s%s (%c)",
	  terse?"":"You now have ", inv_name(obj, !terse), pack_char(obj));
#ifdef	iris
	plusstr = "+";
#endif
    }
    if (obj->o_type == AMULET)
	amulet = TRUE;
}

/*
 * inventory:
 *	list what is in the pack
 */
inventory(list, type)
struct	linked_list *list;
int	type;
{
    reg1 rega1	struct	object	*obj;
    reg2 regd1	char	ch;
    reg3 regd2	int	n_objs;
		char	inv_temp[80];
		struct	object	*oldobj;

    n_objs = 0;
    for (ch = 'a'; list != NULL; ch++, list = next(list)) {
	obj = (struct object *) ldata(list);
	if (type && type != obj->o_type && !(type == CALLABLE &&
	  (obj->o_type == SCROLL || obj->o_type == POTION ||
	  obj->o_type == RING || obj->o_type == WAND)))
	    continue;
	switch (n_objs++) {
	    /*
	     * For the first thing in the inventory, just save the string
	     * in case there is only one.
	     */
	    case 0:
		sprintf(inv_temp, "%c) %s", ch, inv_name(obj, FALSE));
		oldobj = obj;
		break;
	    /*
	     * If there is more than one, clear the screen, print the
	     * saved message and fall through to ...
	     */
	    case 1:
		if (slow_invent) {
#ifdef	COLOR
		    Dcolor(cw, obj->o_type);
#else
		    if (obj->o_type == AMULET)
			wstandout(cw);
#endif
		    msg(inv_temp);
		} else {
		    wclear(hw);
#ifdef	COLOR
		    Dcolor(hw, oldobj->o_type);
#else
		    if (oldobj->o_type == AMULET)
			wstandout(hw);
#endif
		    wprintw(hw, "\n%s\n", inv_temp);
		}
#ifndef	COLOR
		if (oldobj->o_type == AMULET)
		    wstandend(cw);
#endif
	    /*
	     * Print the line for this object
	     */
	    default:
		if (slow_invent) {
#ifdef	COLOR
		    Dcolor(cw, obj->o_type);
#else
		    if (obj->o_type == AMULET)
			wstandout(cw);
#endif
		    msg("%c) %s", ch, inv_name(obj, FALSE));
		} else {
#ifdef	COLOR
		    Dcolor(hw, obj->o_type);
#else
		    if (obj->o_type == AMULET)
			wstandout(hw);
#endif
		    wprintw(hw, "%c) %s\n", ch, inv_name(obj, FALSE));
		}
	}
#ifndef	COLOR
	if (obj->o_type == AMULET)
	    wstandend(cw);
#endif
    }
    if (n_objs == 0) {
#ifdef	COLOR
	acolor(cw, C_cyan, Dbackground);
#endif
	if (terse)
	    msg(type == 0 ? "Empty handed." :
	      "Nothing appropriate");
	else
#ifdef	iris
	    msg(type == 0 ? "You are empty handed." :
	      "Nothing appropriate");
#else
	    msg(type == 0 ? "You are empty handed." :
	      "You don't have anything appropriate");
#endif
	return FALSE;
    }
    if (n_objs == 1) {
#ifdef	COLOR
	Dcolor(cw, obj->o_type);
#else
	if (obj->o_type == AMULET)
	    wstandout(cw);
#endif
	msg(inv_temp);
#ifndef	COLOR
	if (obj->o_type == AMULET)
	    wstandend(cw);
#endif
	return TRUE;
    }
    if (!slow_invent) {
#ifdef	COLOR
	acolor(hw, Dforeground, Dbackground);
#endif
#ifdef	iris
	mvwaddstr(hw, 0, 0, "Press space to continue");
#else
	mvwaddstr(hw, LINES-1, 0, "--Press space to continue--");
#endif
	Draw(hw);
	wait_for(' ');
	clearok(cw, TRUE);
	touchwin(cw);
    }
    return TRUE;
}

/*
 * pick_up:
 *	Add something to characters pack.
 */
pick_up(ch)
char	ch;
{
    switch (ch) {
	case GOLD:
	    money();
	    break;
	default:
	    debug("Where did you pick that up???");
	case ARMOR:
	case POTION:
	case FOOD:
	case WEAPON:
	case SCROLL:	
	case RING:
	case WAND:
	    add_pack(NULL, FALSE);
	    break;
	case AMULET:
	    do_amulet();
	    add_pack(NULL, FALSE);
    }
}

/*
 * picky_inven:
 *	Allow player to inventory a single item
 */
picky_inven()
{
    reg1 rega1	struct	linked_list *item;
    reg2 regd1	char	ch;
    reg3 regd2	char	mch;

    if (pack == NULL)
	msg("You aren't carrying anything");
    else if (next(pack) == NULL)
	msg("a) %s", inv_name((struct object *) ldata(pack), FALSE));
    else {
	msg(terse ? "Item: " : "Which item do you wish to inventory: ");
	mpos = 0;
	if ((mch = readchar()) == ESCAPE) {
	    msg("");
	    return;
	}
	for (ch = 'a', item = pack; item != NULL; item = next(item), ch++)
	    if (ch == mch) {
		msg("%c) %s",ch,inv_name((struct object *) ldata(item), FALSE));
		return;
	    }
	if (!terse)
	    msg("'%s' not in pack", unctrl(mch));
	msg("Range is 'a' to '%c'", --ch);
    }
}

/*
 * get_item:
 *	pick something out of a pack for a purpose
 */
struct	linked_list *
get_item(purpose, type)
char	*purpose;
int	type;
{
    reg1 rega1	struct	linked_list *obj;
    reg2 regd1	char	ch;
    reg3 regd2	char	och;

    if (pack == NULL)
	msg("You aren't carrying anything.");
    else {
	for (;;) {
	    if (terse)
		msg("%s what?",purpose);
	    else
#ifdef	iris
		msg("Which object do you want to %s?",purpose);
#else
		msg("Which object do you want to %s? (* for list): ",purpose);
#endif
	    ch = readchar();
	    mpos = 0;
	    /*
	     * Give the poor player a chance to abort the command
	     */
	    if (ch == ESCAPE || ch == CTRL(G)) {
		after = FALSE;
		msg("");
		return NULL;
	    }
	    if (ch == '*') {
		mpos = 0;
		if (inventory(pack, type) == 0) {
		    after = FALSE;
		    return NULL;
		}
		continue;
	    }
	    for (obj = pack, och = 'a'; obj != NULL; obj = next(obj), och++)
		if (ch == och)
		    break;
	    if (obj == NULL) {
		msg("Please specify a letter between 'a' and '%c'", och-1);
		continue;
	    }
	    else 
		return obj;
	}
    }
    return NULL;
}

pack_char(obj)
reg1 rega1	struct	object	*obj;
{
    reg2 rega2	struct	linked_list *item;
    reg3 regd1	char	c;

    c = 'a';
    for (item = pack; item != NULL; item = next(item))
	if ((struct object *) ldata(item) == obj)
	    return c;
	else
	    c++;
    return 'z';
}
