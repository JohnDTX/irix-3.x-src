/* new_level.c @+++@ */
static	char	*Sccsid = "@(#)new_level.c	1.10 8/29/84";
#include "rogue.h"

/*
 * new_level:
 *	Dig and draw a new level
 *
 * @(#)new_level.c	3.7 (Berkeley) 6/2/81
 */

new_level()
{
    reg1 regd1	int	rm;
    reg2 regd2	int	i;
    reg3 regd3	char	ch;
		coord	stairs;

    if (level > max_level)
	max_level = level;
    wclear(cw);
    wclear(mw);
    Clear();
    status();
    /*
     * Free up the monsters on the last level
     */
    free_list(mlist);
    do_rooms();				/* Draw rooms */
    do_passages();			/* Draw passages */
    no_food++;
    put_things();			/* Place objects (if any) */
    /*
     * Place the staircase down.
     */
    do {
        rm = rnd_room();
	rnd_pos(&rooms[rm], &stairs);
    } until (winat(stairs.y, stairs.x) == floor);
    addch(STAIRS);
    /*
     * Place the traps
     */
    if (rnd(10) < level)
    {
	ntraps = rnd(level/4)+1;
	if (ntraps > MAXTRAPS)
	    ntraps = MAXTRAPS;
	i = ntraps;
	while (i--)
	{
	    do
	    {
		rm = rnd_room();
		rnd_pos(&rooms[rm], &stairs);
	    } until (winat(stairs.y, stairs.x) == floor);
	    switch(rnd(6))
	    {
		when 0: ch = TRAPDOOR;
		when 1: ch = BEARTRAP;
		when 2: ch = SLEEPTRAP;
		when 3: ch = ARROWTRAP;
		when 4: ch = TELTRAP;
		when 5: ch = DARTTRAP;
	    }
	    addch(TRAP);
	    traps[i].tr_type = ch;
	    traps[i].tr_flags = 0;
	    traps[i].tr_pos = stairs;
	}
    }
    do
    {
	rm = rnd_room();
	rnd_pos(&rooms[rm], &hero);
    }
    until(winat(hero.y, hero.x) == floor);
    light(&hero);
    wmove(cw, hero.y, hero.x);
#ifdef	COLOR
    Dcolor(cw, PLAYER);
#endif
    waddch(cw, PLAYER);
#ifdef	COLOR
    acolor(cw, Dforeground, Dbackground);
#endif
}

/*
 * Pick a room that is really there
 */

rnd_room()
{
    reg1 regd1	int	rm;

    do {
	rm = rnd(MAXROOMS);
    } while (rooms[rm].r_flags & ISGONE);
    return rm;
}

/*
 * put_things:
 *	put potions and scrolls on this level (rogue 3.6)
 */

put_things()
{
    reg1 regd1	int	i;
    reg2 rega1	struct	linked_list *item;
    reg3 rega2	struct	object	*cur;
    reg4 regd2	int	rm;
		coord	tp;

    /*
     * Throw away stuff left on the previous level (if anything)
     */
    free_list(lvl_obj);
    /*
     * Once you have found the amulet, the only way to get new stuff is
     * go down into the dungeon.
     */
    if (amulet && level < max_level)
	return;
    /*
     * Do MAXOBJ attempts to put things on a level
     */
    for (i = 0; i < MAXOBJ; i++)
	if (rnd(100) < 35)
	{
	    /*
	     * Pick a new object and link it in the list
	     */
	    item = new_thing();
	    attach(lvl_obj, item);
	    cur = (struct object *) ldata(item);
	    /*
	     * Put it somewhere
	     */
	    rm = rnd_room();
	    do {
		rnd_pos(&rooms[rm], &tp);
	    } until (winat(tp.y, tp.x) == floor);
	    mvaddch(tp.y, tp.x, cur->o_type);
	    cur->o_pos = tp;
	}
    /*
     * If he is really deep in the dungeon and he hasn't found the
     * amulet yet, put it somewhere on the ground
     */
    if (level > 25 && !amulet)
    {
	item = new_item(sizeof *cur);
	attach(lvl_obj, item);
	cur = (struct object *) ldata(item);
	cur->o_hplus = cur->o_dplus = 0;
	cur->o_damage = cur->o_hurldmg = "0d0";
	cur->o_ac = 11;
	cur->o_type = AMULET;
	/*
	 * Put it somewhere
	 */
	rm = rnd_room();
	do {
	    rnd_pos(&rooms[rm], &tp);
	} until (winat(tp.y, tp.x) == floor);
	mvaddch(tp.y, tp.x, cur->o_type);
	cur->o_pos = tp;
    }
}
