static	char	*Command_c	= "@(#)command.c	1.19";
/*
 * Read and execute the user commands
 *
 * @(#)command.c	3.45 (Berkeley) 6/15/81
 */

#include <ctype.h>
#include <signal.h>
#include "rogue.h"

extern char	*himem;
#ifdef	iris
char	notengch[] = "!\"#$%&{|}~*/;=+_`\\@^<>";/* characters not in Old English */
#endif

/*
 * command:
 *	Process the user commands
 */

command()
{
    reg1 regd1	char	ch;
    reg2 regd2	int	ntimes = 1;		/* Number of player moves */
    static	char	countch, direction, newcount = FALSE;
		char	*unctrl();

    if (on(player, ISHASTE))
	ntimes++;
    /*
     * Let the daemons start up
     */
    do_daemons(BEFORE);
    do_fuses(BEFORE);
    while (ntimes--) {
	look(TRUE);
	if (!running)
	    door_stop = FALSE;
	lastscore = purse;
	wmove(cw, hero.y, hero.x);
	if (!((running || count) && jump)) {
	    status();
	    Draw(cw);			/* Draw screen */
	}
	take = 0;
	after = TRUE;
	/*
	 * Read command or continue run
	 */
	if (wizard)
	    waswizard = TRUE;
	if (!no_command) {
	    if (running)
		ch = runch;
	    else if (count > 0)
		ch = countch;
	    else {
		ch = Readchar();
		if (mpos != 0 && !running)	/* Erase message if its there */
		    msg("");
	    }
	} else
	    ch = REST;
	if (no_command) {
	    if (--no_command <= 0)
		msg("You can move again");
	} else {
	    /*
	     * check for prefixes
	     */
	    if (isdigit(ch)) {
		count = 0;
		newcount = TRUE;
		while (isdigit(ch)) {
		    count = count * 10 + (ch - '0');
		    ch = Readchar();
		}
		countch = ch;
		/*
		 * turn off count for commands which don't make sense
		 * to repeat
		 */
		switch (ch) {
		    case 'h': case 'j': case 'k': case 'l':
		    case 'y': case 'u': case 'b': case 'n':
		    case 'H': case 'J': case 'K': case 'L':
		    case 'Y': case 'U': case 'B': case 'N':
		    case 'q': case 'r': case 's': case 'f':
		    case 't': case 'C': case 'I': case REST:
		    case 'z': case 'p':
			break;
		    default:
			count = 0;
		}
	    }
	    switch (ch) {
		case CTRL(H) :
		case CTRL(J) :
		case CTRL(K) :
		case CTRL(L) :
		case CTRL(Y) :
		case CTRL(U) :
		case CTRL(B) :
		case CTRL(N) :
		    door_stop = TRUE;
		    firstmove = TRUE;
		    ch += '@';
		    direction = ch;
		when 'f':
		    if (!on(player, ISBLIND)) {
			door_stop = TRUE;
			firstmove = TRUE;
		    }
		    if (count && !newcount)
			ch = direction;
		    else
			ch = Readchar();
		    switch (ch) {
			case 'h': case 'j': case 'k': case 'l':
			case 'y': case 'u': case 'b': case 'n':
			    ch = toupper(ch);
		    }
		    direction = ch;
	    }
	    newcount = FALSE;
	    /*
	     * execute a command
	     */
	    if (count > 0 && !running)
		count--;
	    switch (ch) {
		case '!' : shell();
		when 'h' : do_move(0, -1);
		when 'j' : do_move(1, 0);
		when 'k' : do_move(-1, 0);
		when 'l' : do_move(0, 1);
		when 'y' : do_move(-1, -1);
		when 'u' : do_move(-1, 1);
		when 'b' : do_move(1, -1);
		when 'n' : do_move(1, 1);
		when 'H' : do_run('h');
		when 'J' : do_run('j');
		when 'K' : do_run('k');
		when 'L' : do_run('l');
		when 'Y' : do_run('y');
		when 'U' : do_run('u');
		when 'B' : do_run('b');
		when 'N' : do_run('n');
		when 't':
		    if (!get_dir())
			after = FALSE;
		    else
			missile(delta.y, delta.x);
		when CTRL(C) :
		case 'Q' : after = FALSE; quit();
		when 'i' : after = FALSE; inventory(pack, 0);
		when 'I' : after = FALSE; picky_inven();
		when 'd' : drop();
		when 'q' : quaff();
		when 'r' : read_scroll();
		when 'e' : eat();
		when 'w' : wield();
		when 'W' : wear();
		when 'T' : take_off();
		when 'P' : ring_on();
		when 'R' : ring_off();
		when 'o' : option();
		when 'c' : call();
		when '>' : after = FALSE; d_level();
		when '<' : after = FALSE; u_level();
		when '?' : after = FALSE; help();
		when '/' : after = FALSE; identify();
		when 's' : search();
		when 'z' : do_zap(FALSE);
		when 'p':
		    if (get_dir())
			do_zap(TRUE);
		    else
			after = FALSE;
		when 'v' : msg("Rogue version %s. (physhy was here)", release);
							/* maybe Draw(curscr) */
		when CTRL(R) : after = FALSE; clearok(curscr,TRUE); Draw(cw);
		when CTRL(P) :
		    after = FALSE;
#ifdef	COLOR
		    cw->_attrib = Rcolor;
#endif
		    msg(huh);
		when 'S' : 
		    after = FALSE;
		    if (save_game(0)) {		/* != 0 if signalled */
			wmove(cw, LINES-1, 0); 
			wclrtoeol(cw);
			Draw(cw);
			endwin();
			exit(0);
		    }
		when REST : ;			/* Rest command */
		when ' ' : after = FALSE;	/* no-op */
#ifndef	SMALL
		when '+' :
		    after = FALSE;
		    if (wizard) {
			wizard = FALSE;
		        acolor(cw, Dbackground, Dforeground);
			msg("Not a wizard anymore");
		    } else {
			if (wizard = passwd()) {
			    acolor(cw, C_magenta, C_gold);
			    msg("You now feel a power far beyond mere mortals!");
			    waswizard = TRUE;
			} else {
			    acolor(cw, C_red, Dbackground);
			    msg("Sorry, Charlie");
			}
		    }
#endif	SMALL
		when ESCAPE :	/* Escape */
		    door_stop = FALSE;
		    count = 0;
		    after = FALSE;
#ifndef	SMALL
#ifdef WIZD
		when 'D' : dae_list(); /* of fuses & daemons */
#endif
#endif	SMALL
		otherwise :
		    after = FALSE;
#ifndef	SMALL
		    if (wizard) switch (ch) {
			case '@' : msg("@ %d,%d", hero.y, hero.x);
			when 'C' : createm();
			when CTRL(I) : inventory(lvl_obj, 0);
			when CTRL(W) : whatis();
			when CTRL(D) : level++; new_level();
			when CTRL(U) : level--; new_level();
			when CTRL(F) : show_win(stdscr, "More (level map)");
			when CTRL(X) : show_win(mw, "More (monsters)");
			case CTRL(M) : mem();
			when CTRL(T) : teleport();
			when CTRL(E) : msg("food left: %d", food_left);
			when CTRL(A) : msg("%d things in your pack", inpack);
			when CTRL(G) : add_pass();
			when CTRL(N) :
			{
			    struct	linked_list *item;

			    if ((item = get_item("charge", WAND)) != NULL)
				((struct object *) ldata(item))->o_charges = 10000;
			}
			when CTRL(H) :
			{
			    int	i;
			    struct	linked_list *item;
			    struct	object	*obj;

			    for (i = 0; i < 9; i++)
				raise_level();
			    /*
			     * Give the rogue a sword (+1,+1)
			     */
			    item = new_item(sizeof *obj);
			    obj = (struct object *) ldata(item);
			    obj->o_type = WEAPON;
			    obj->o_which = TWOSWORD;
			    init_weapon(obj, SWORD);
			    obj->o_hplus = 100;
			    obj->o_dplus = 100;
			    add_pack(item, TRUE);
			    cur_weapon = obj;
			    /*
			     * And his suit of armor
			     */
			    item = new_item(sizeof *obj);
			    obj = (struct object *) ldata(item);
			    obj->o_type = ARMOR;
			    obj->o_which = PLATE_MAIL;
			    obj->o_ac = -100;
			    obj->o_flags |= ISKNOW;
			    cur_armor = obj;
			    add_pack(item, TRUE);
			}
#ifdef	WIZD
			when 'D' : dae_list();	/*of fuses && daemons */
#endif
			otherwise :
#ifdef	iris
			  if (!eng(ch))
			      msg("Illegal Command.");
			  else
#endif
			      msg("Illegal Command '%s'.", unctrl(ch));
				count = 0;
		    } else
#endif	SMALL
		    {
#ifdef	iris
			if (!eng(ch))
			    msg("Illegal Command");
			else
#endif
			    msg("Illegal Command '%s'!", unctrl(ch));
			count = 0;
		    }
	    }
	    /*
	     * turn off flags if no longer needed
	     */
	    if (!running)
		door_stop = FALSE;
	}
	/*
	 * If he ran into something to take, let him pick it up.
	 */
	if (take != 0)
	    pick_up(take);
	if (!running)
	    door_stop = FALSE;
    }
    /*
     * Kick off the rest if the daemons and fuses
     */
    if (after) {
	look(FALSE);
	do_daemons(AFTER);
	do_fuses(AFTER);
	if (ISRING(LEFT, R_SEARCH))
	    search();
	else if (ISRING(LEFT, R_TELEPORT) && rnd(100) < 2)
	    teleport();
	if (ISRING(RIGHT, R_SEARCH))
	    search();
	else if (ISRING(RIGHT, R_TELEPORT) && rnd(100) < 2)
	    teleport();
    }
}


/*
 * quit:
 *	Have player make certain, then exit.
 */

quit()
{
    /*
     * Reset the signal in case we got here via an interrupt
     */
    char c;

    if (signal(SIGINT, quit) != quit)
	mpos = 0;
    msg("Really quit? (y-n)");
    Draw(cw);
    c = readchar();
    if (c == 'y' || c == 'Y' || c == CTRL(C) || c == 0177) {
	Clear();
	Move(LINES-1, 0);
	Draw(stdscr);
#ifndef	SCORE
#ifdef	SMALL
	death('\0');
#else
	score(purse, QUITIT);
#endif	SMALL
#endif
	endwin();
	resetty();
#ifdef	Curses
#ifdef	BSD
				/* damned if I know */
	if (CN)
		write(1,CN,strlen(CN));
#else
	if (CN)
		printf("%s",CN);
#endif	BSD
#endif	Curses
	exit(0);
    } else {
	signal(SIGINT, quit);
	wmove(cw, 0, 0);
	wclrtoeol(cw);
	status();
	Draw(cw);
	mpos = 0;
	count = 0;	/* allow the poor rogue to revoke '10000.'	*/
    }
}

/*
 * search:
 *	Player gropes about him to find hidden things.
 */

search()
{
    reg1 regd1	int	x;
    reg2 regd2	int	y;
    reg3 regd3	int	i;
    reg4 regd4	char	ch;

    /*
     * Look all around the hero, if there is something hidden there,
     * give him a chance to find it.  If its found, display it.
     */
    if (on(player, ISBLIND))
	return;
    for (x = hero.x - 1; x <= hero.x + 1; x++)
	for (y = hero.y - 1; y <= hero.y + 1; y++) {
	    ch = winat(y, x);
	    switch (ch) {
	      case SECRETDOOR:
		if (rnd(100) < 20) {
		    mvaddch(y, x, door);
		    count = 0;
		}
		break;
	      case TRAP:
		{
		    struct	trap *tp;

		    if (mvwinch(cw, y, x) == TRAP)
			break;
		    if (rnd(100) > 50)
			break;
		    tp = trap_at(y, x);
		    tp->tr_flags |= ISFOUND;
		    mvwaddch(cw, y, x, TRAP);
		    count = 0;
		    running = FALSE;
		    msg(tr_name(tp->tr_type));
		}
	    }
	}
}

/*
 * tr_name:
 *	print the name of a trap
 */

char	*
tr_name(ch)
char	ch;
{
    reg1 rega1	char	*s;

    switch (ch) {
      case TRAPDOOR:
	s = terse ? "A trapdoor." : "You found a trapdoor.";
      when BEARTRAP:
	s = terse ? "A beartrap." : "You found a beartrap.";
      when SLEEPTRAP:
	s = terse ? "A sleeping gas trap." :
	  "You found a sleeping gas trap.";
      when ARROWTRAP:
	s = terse ? "An arrow trap." :
	  "You found an arrow trap.";
      when TELTRAP:
	s = terse ? "A teleport trap." : "You found a teleport trap.";
      when DARTTRAP:
	s = terse ? "A dart trap." : "You found a poison dart trap.";
    }
    return s;
}

/*
 * help:
 *	Give single character help, or the whole mess if he wants it
 */
help()
{
    char	helpch;

#ifdef	iris
    msg("Character you want help for:");
    prompt(msgw);
#else
    msg("Character you want help for (* for all): ");
#endif
    helpch = readchar();
    msg("");
    help2(helpch,helpstr,wizard);
    if (wizard)
	   help2(helpch,wizhelp,FALSE);
}

help2(helpch,strp,noclean)
reg1 regd1	char	helpch;
reg2 rega1	struct	h_list *strp;
{
    reg3 regd2	int	cnt;

    /*
     * If its not a *, print the right help string
     * or an error if he typed a funny character.
     */
    if (helpch != '*') {
/*	wmove(cw, 1, 0); */
	while (strp->h_ch) {
	    if (strp->h_ch == helpch) {
#ifdef	iris
		if (!eng(helpch))
		    msg("%s", strp->h_desc);
		else
#endif
		    msg("%s%s", unctrl(strp->h_ch), strp->h_desc);
		break;
	    }
	    strp++;
	}
	if (strp->h_ch != helpch)
#ifdef	iris
	    if (!eng(helpch))
		msg("Unknown character");
	    else
#endif
		msg("Unknown character '%s'", unctrl(helpch));
	return;
    }
    /*
     * Here we print help for everything.
     * Then wait before we return to command mode
     */
    wclear(hw);
    cnt = 0;
    while (strp->h_ch) {
#ifdef	iris
	if (_iris) {
	    mvwaddstr(hw,1+(cnt%(24-1)),cnt>=24-1?40:0,unctrl(strp->h_ch));
	} else
#endif
	    mvwaddstr(hw,cnt%(24-1),cnt>=24-1?40:0,unctrl(strp->h_ch));
	waddstr(hw, strp->h_desc);
	cnt++;
	strp++;
    }
#ifdef	iris
    if (_iris) {
	wmove(hw, 0, 0);
	wprintw(hw, "Press space to continue");
    } else
#endif
    {
	wmove(hw, LINES-1, 0);
	wprintw(hw, "--Press space to continue--");
    }
    Draw(hw);
    wait_prompt(hw, ' ');
    if (!noclean) {
	wclear(hw);
	Draw(hw);
	wmove(cw, 0, 0);
	wclrtoeol(cw);
	status();
	touchwin(cw);
    }
}

/*
 * identify:
 *	Tell the player what a certain thing is.
 */

identify()
{
    reg1 rega1	char	ch, *str;

    msg("What do you want identified?");
/*  Draw(cw); */
    ch = readchar();
    mpos = 0;
    if (ch == ESCAPE) {
	msg("");
	return;
    }
    if (isalpha(ch) && isupper(ch))
	str = monsters[ch-'A'].m_name;
    else
	switch (ch) {
	  case GOLD:	str = "gold";
	  when STAIRS:	str = "passage leading down";
	  when PLAYER:	str = "you";
	  when TRAP:	str = "trap";
	  when POTION:	str = "potion";
	  when SCROLL:	str = "scroll";
	  when FOOD:	str = "food";
	  when WEAPON:	str = "weapon";
	  when ' ':	str = "solid rock";
	  when ARMOR:	str = "armor";
	  when WAND:	str = "wand or staff";
	  when RING:	str = "ring";
	  when AMULET:	str = "The Amulet of Yendor";
	  otherwise:
			if (iswall(ch))
			    str = "wall of a room";
			else if (ch == door)
			    str = "door";
			else if (ch == floor)
			    str = "room floor";
			else if (ch == passage)
			    str = "passage";
			else
			    str = "unknown character";
    }
#ifdef	COLOR
    Dcolor(cw,ch);
#endif
#ifdef	iris
    if (!eng(ch))
	msg("%s", str);
    else
#endif	iris
	msg("'%s' : %s", unctrl(ch), str);
#ifdef	COLOR
    acolor(cw, Dforeground, Dbackground);
#endif
}

/*
 *
 * Eng:
 *	Return true if character is Old English
 *	Will always return true if not on IRIS
 */
eng(ch)
char	ch;
{
#ifdef	iris
    if (_iris && (ch < ' ' || ch > 126 || index(notengch,ch)))
	return FALSE;
#endif	iris
    return TRUE;
}

/*
 * Look:
 *	A quick glance all around the player
 */

look(wakeup)
bool	wakeup;
{
    reg1 regd1	int	x;
    reg2 regd2	int	y;
    reg3 regd3	char	ch;
    reg4 regd4	int	oldx;
    reg5 regd5	int	oldy;
    reg6 regd6	bool	inpass;
		int	passcount = 0;
    reg7 rega1	struct	room	*rp;
		int	ey, ex;

    getyx(cw, oldy, oldx);
    if (oldrp != NULL && (oldrp->r_flags & ISDARK) && off(player, ISBLIND)) {
	for (x = oldpos.x - 1; x <= oldpos.x + 1; x++)
	    for (y = oldpos.y - 1; y <= oldpos.y + 1; y++)
		if ((y != hero.y || x != hero.x) && show(y, x) == floor)
		    mvwaddch(cw, y, x, ' ');
    }
    inpass = ((rp = roomin(&hero)) == NULL);
    ey = hero.y + 1;
    ex = hero.x + 1;
    for (x = hero.x - 1; x <= ex; x++)
	if (x >= 0 && x < COLS) for (y = hero.y - 1; y <= ey; y++) {
	    if (y <= 0 || y >= LINES - 1)
		continue;
	    if (isupper(mvwinch(mw, y, x))) {
		struct	linked_list *it;
		struct	thing	*tp;

		if (wakeup)
		    it = wake_monster(y, x);
		else
		    it = find_mons(y, x);
		tp = (struct thing *) ldata(it);
		if ((tp->t_oldch = mvinch(y, x)) == TRAP)
		    tp->t_oldch =
			(trap_at(y,x)->tr_flags&ISFOUND) ? TRAP : floor;
		if (tp->t_oldch == floor && (rp->r_flags & ISDARK)
		    && off(player, ISBLIND))
			tp->t_oldch = ' ';
	    }
	    /*
	     * Secret doors show as walls
	     */
	    if ((ch = show(y, x)) == SECRETDOOR)
		ch = secretdoor(y, x);
	    /*
	     * Don't show room walls if he is in a passage
	     */
	    if (off(player, ISBLIND)) {
		if (y == hero.y && x == hero.x
		 || (inpass && iswall(ch)))
			continue;
	    }
	    else
		if (y != hero.y || x != hero.x)
		    continue;
	    wmove(cw, y, x);
#ifdef	COLOR
	    Dcolor(cw, ch);
#endif
	    waddch(cw, ch);
#ifdef	COLOR
	    acolor(cw, Dforeground, Dbackground);
#endif
	    if (door_stop && !firstmove && running) {
		switch (runch) {
		    case 'h':
			if (x == ex)
			    continue;
		    when 'j':
			if (y == hero.y - 1)
			    continue;
		    when 'k':
			if (y == ey)
			    continue;
		    when 'l':
			if (x == hero.x - 1)
			    continue;
		    when 'y':
			if ((x + y) - (hero.x + hero.y) >= 1)
			    continue;
		    when 'u':
			if ((y - x) - (hero.y - hero.x) >= 1)
			    continue;
		    when 'n':
			if ((x + y) - (hero.x + hero.y) <= -1)
			    continue;
		    when 'b':
			if ((y - x) - (hero.y - hero.x) <= -1)
			    continue;
		}
		if (ch == door) {
		    if (x == hero.x || y == hero.y)
			running = FALSE;
		} else if (ch == passage) {
		    if (x == hero.x || y == hero.y)
			passcount++;
		} else if (ch != floor && !iswall(ch) && ch != ' ')
			running = FALSE;
	    }
	}
    if (door_stop && !firstmove && passcount > 1)
	running = FALSE;
    acolor(cw, C_PLAYER, Dbackground);
    mvwaddch(cw, hero.y, hero.x, PLAYER);
    acolor(cw, Dforeground, Dbackground);
    wmove(cw, oldy, oldx);
    oldpos = hero;
    oldrp = rp;
}

#ifdef	notdef

look()
{
    reg1 regd1	int	x;
    reg2 regd2	int	y;
    reg3 regd3	char	ch;
    reg4 regd4	int	oldx;
    reg5 regd5	int	oldy;
    reg6 regd6	bool	inpass;
		int	passcount = 0;

    getyx(cw, oldy, oldx);
    inpass = (bool)(roomin(&hero) == NULL);
    for (x = hero.x - 1; x <= hero.x + 1; x++)
	if (x >= 0 && x < COLS) for (y = hero.y - 1; y <= hero.y + 1; y++) {
	    if (y <= 0 || y >= LINES - 1)
		continue;
	    ch = show(y, x);
	    /*
	     * Secret doors show as walls
	     */
	    if (ch == SECRETDOOR)
		ch = secretdoor(y, x);
	    /*
	     * Don't show room walls if he is in a passage
	     */
	    if (y == hero.y && x == hero.x ||
		(inpass && iswall(ch)))
		    continue;
	    wmove(cw, y, x);
#ifdef	COLOR
	    Dcolor(cw, ch);
#endif
	    waddch(cw, ch);
#ifdef	COLOR
	    acolor(cw, Dforeground, Dbackground);
#endif
	    if (door_stop && !firstmove && running) {
		if (ch != passage) switch (runch) {
		    case 'h':
			if (x == hero.x + 1)
			    continue;
		    when 'j':
			if (y == hero.y - 1)
			    continue;
		    when 'k':
			if (y == hero.y + 1)
			    continue;
		    when 'l':
			if (x == hero.x - 1)
			    continue;
		    when 'y':
			if ((x + y) - (hero.x + hero.y) >= 1)
			    continue;
		    when 'u':
			if ((y - x) - (hero.y - hero.x) >= 1)
			    continue;
		    when 'n':
			if ((x + y) - (hero.x + hero.y) <= -1)
			    continue;
		    when 'b':
			if ((y - x) - (hero.y - hero.x) <= -1)
			    continue;
		}
		switch (ch) {
		  case ' ':
		    break;
		  default:
		    if (ch == passage) {
		        if (x == hero.x || y == hero.y)
			    passcount++;
		        break;
		    }
		    if (ch == door && (x == hero.x || y == hero.y)) {
			running = FALSE;
			break;
		    }
		    if (ch == floor)
			break;
		    if (!iswall(ch))
			running = FALSE;
		    break;
		}
	    }
	}
    if (door_stop && !firstmove && passcount > 2)
	running = FALSE;
    wmove(cw, oldy, oldx);
}

#endif

/*
 * d_level:
 *	He wants to go down a level
 */

d_level()
{
    if (winat(hero.y, hero.x) != STAIRS)
	msg("I see no way down.");
    else {
	level++;
	new_level();
    }
}

/*
 * u_level:
 *	He wants to go up a level
 */

u_level()
{
    if (winat(hero.y, hero.x) == STAIRS) {
	if (amulet) {
	    level--;
	    if (level == 0)
		total_winner();
	    new_level();
	    msg("You feel a wrenching sensation in your gut");
	    return;
	}
    }
    msg("I see no way up");
}

/*
 * secret_door:
 *	Figure out what a secret door looks like.
 */

secretdoor(y, x)
reg1 regd1	int	y;
reg2 regd2	int	x;
{
    reg3 regd3	int	i;
    reg4 rega1	struct	room	*rp;
    reg5 rega2	coord	*cpp;
		coord	cp;

    cp.y = y;
    cp.x = x;
    cpp = &cp;
    for (rp = rooms, i = 0; i < MAXROOMS; rp++, i++) {
	if (inroom(rp, cpp)) {
	    if (y == rp->r_pos.y ||
		y == rp->r_pos.y + rp->r_max.y - 1) return(wallh);
	    else
		return(wallv);
	}
    }
    return('p');
}

/*
 * Let him escape for a while
 */

shell()
{
    reg1 rega1	int	pid;
		int	ret_status;
    reg2 rega2	char	*sh = getenv("SHELL");
		int	i;
#ifdef WIZDB
		int	wizquit();
#endif

    /*
     * Set the terminal back to original mode
     */
    wclear(hw);
    wmove(hw, LINES-1, 0);
    Draw(hw);
    endwin();
    in_shell = TRUE;
    fflush(stdout);
    /*
     * Fork and do a shell
     */
    i = 15;
    while ((pid = fork()) < 0 && i-- > 0)
	sleep(1);
    if (pid == 0) {
	/*
	 * Set back to original user, just in case
	 */
	setuid(getuid());
	setgid(getgid());
	execl(sh == NULL ? "/bin/sh" : sh, "shell", "-i", 0);
	perror("No shelly");
	exit(-1);
    } else {
	int	endit();

	if (pid > 0) {
	    signal(SIGINT, SIG_IGN);
	    signal(SIGQUIT, SIG_IGN);
	    while ((i = wait(&ret_status)) != pid && i >= 0)
	        ;
#ifdef WIZDB
	    if (wizard)
	        signal(SIGINT, wizquit);
	    else
#endif
	        signal(SIGINT, quit);
	    signal(SIGQUIT, quit);
	} else
	    perror("Can't fork()!");
	printf("\n[Press return to continue]");
	noecho();
	crmode();
	in_shell = FALSE;
	wait_for('\n');
	clearok(cw, TRUE);
	touchwin(cw);
    }
}

/*
 * allow a user to call a potion, scroll, or ring something
 */
call()
{
    reg1 rega1	struct	object	*obj;
    reg2 rega2	struct	linked_list *item;
    reg3 rega3	char	**guess;
    reg4 rega4	char	*elsewise;
    reg5 rega5	bool	*know;
		char	*malloc();

    item = get_item("call", CALLABLE);
    /*
     * Make certain that it is somethings that we want to wear
     */
    if (item == NULL)
	return;
    obj = (struct object *) ldata(item);
    switch (obj->o_type) {
	case RING:
	    guess = r_guess;
	    know = r_know;
	    elsewise = (r_guess[obj->o_which] != NULL ?
	      r_guess[obj->o_which] : r_rings[obj->o_which]);
	when POTION:
	    guess = p_guess;
	    know = p_know;
	    elsewise = (p_guess[obj->o_which] != NULL ?
	      p_guess[obj->o_which] : p_colors[obj->o_which]);
	when SCROLL:
	    guess = s_guess;
	    know = s_know;
	    elsewise = (s_guess[obj->o_which] != NULL ?
	      s_guess[obj->o_which] : s_names[obj->o_which]);
	when WAND:
	    guess = W_guess;
	    know = W_know;
	    elsewise = (W_guess[obj->o_which] != NULL ?
	      W_guess[obj->o_which] : W_made[obj->o_which]);
	otherwise:
	    msg("You can't call that anything");
	    return;
    }
    if (know[obj->o_which]) {
	msg("That has already been identified");
	return;
    }
    msg("%salled \"%s\"", terse ? "C" : "Was c", elsewise);
    if (terse)
	msg("Call it: ");
    else
	msg("What do you want to call it? ");
    if (guess[obj->o_which] != NULL)
	cfree(guess[obj->o_which]);
    strcpy(prbuf, elsewise);
    if (get_str(prbuf, cw) == NORM) {
	guess[obj->o_which] = malloc((unsigned int) strlen(prbuf) + 1);
	strcpy(guess[obj->o_which], prbuf);
    }
}

#ifndef	SMALL
/*
 * see if user knows password
 */
passwd()
{
    reg1 rega1	char	*sp;
    reg2 regd1	char	c;
		char	buf[80];

    msg("Wizard's Password:");
    mpos = 0;
    sp = buf;
    while ((c = readchar()) != '\n' && c != '\r' && c != '\033')
#ifdef V7
	if (c == _tty.sg_kill)
#else
	if (c == _tty.c_cc[VKILL])
#endif
	    sp = buf;
#ifdef V7
	else if (c == _tty.sg_erase && sp > buf)
#else
	else if (c == _tty.c_cc[VERASE] && sp > buf)
#endif
	    sp--;
	else
	    *sp++ = c;
    if (sp == buf)
	return FALSE;
    *sp = '\0';
    return (strcmp(PASSWD, crypt(buf, "mT")) == 0);
}

/*
 * wizquit:
 *	Handle quit signal for wizards
 */

#ifdef WIZDB
wizquit()
{
	wizdebug();
}

wizdebug()
{
    WINDOW	*Win;
    int		i,j,k;
    char	c;
    int		hoff;	/* Horizontal OFFset */

    mpos = 0;
    printf("\r");
    for (i=0; i<25; i++)
	printf("\n");
    printf("Quit signal -- wizard debugging!\r\n");
    printf("Select window (s=screen c=standard screen m=monster h=help:");
    fflush(stdout);
    c = readchar() & 0177;
    printf("%c - ",c);
    switch (c) {
      case 's':
	Win = stdscr;
	printf("stdscr\r\n");
	break;
      case 'c':
	Win = cw;
	printf("cw\r\n");
	break;
      case 'm':
	Win = mw;
	printf("mw\r\n");
	break;
      case 'h':
	Win = hw;
	printf("hw\r\n");
	break;
      default:
	Win = stdscr;
	printf("Unknown ... using stdscr\r\n");
	break;
    }
    printf("Enter debugging level:");
    fflush(stdout);
    _Debug = 0;
    while ((c=putchar(readchar())) >= '0' && c <= '9') {
	_Debug = (_Debug * 10) + c - '0';
	fflush(stdout);
    }
    printf("\r\n");
    fflush(stdout);
    printf("Enter horizontal offset:");
    fflush(stdout);
    hoff = 0;
    while ((c=putchar(readchar())) >= '0' && c <= '9') {
	hoff = (hoff * 10) + c - '0';
	fflush(stdout);
    }
    printf("\r\n");
    fflush(stdout);
    printf("cw:cur(%d,%d) max(%d,%d) beg(%d,%d)\r\n",
      cw->_cury, cw->_curx,
      cw->_maxy, cw->_maxx,
      cw->_begy, cw->_begx);
    printf("stdscr:cur(%d,%d) max(%d,%d) beg(%d,%d)\r\n",
      stdscr->_cury, stdscr->_curx,
      stdscr->_maxy, stdscr->_maxx,
      stdscr->_begy, stdscr->_begx);
    printf("hero(y:%d,x:%d)\n",hero.y,hero.x);
    if (Win==cw)
	printf("curscr is cw\n");
    else if (Win==stdscr)
	printf("curscr is stdscr\n");
    else
	printf("curscr is Unknown\n");
    printf("cur(%d,%d) max(%d,%d) beg(%d,%d)\r\n",
      Win->_cury, Win->_curx,
      Win->_maxy, Win->_maxx,
      Win->_begy, Win->_begx);
    printf("flags=0x%x sub=%c end=%c full=%c scroll=%c flush=%c stand=%c\r\n",
      Win->_flags,
      (Win->_flags&_SUBWIN)?'Y':'N',
      (Win->_flags&_ENDLINE)?'Y':'N',
      (Win->_flags&_FULLWIN)?'Y':'N',
      (Win->_flags&_SCROLLWIN)?'Y':'N',
      (Win->_flags&_FLUSH)?'Y':'N',
      (Win->_flags&_STANDOUT)?'Y':'N');
#ifdef	COLOR
    printf("attributes=0x%x foreground=%d background=%d\n",
      Win->_attrib,
      dfcolor(Win->_attrib),
      dbcolor(Win->_attrib));
#endif
    printf("clear=%c leave=%c scroll=%c\r\n",
      Win->_clear?'Y':'N',
      Win->_leave?'Y':'N',
      Win->_scroll?'Y':'N');
    printf("Hit RETURN:");
    fflush(stdout);
    readchar();
    printf("\r\n");
    fflush(stdout);
    for (i=0; i<24; i++) {
	printf("line=%2d F=%2d L=%2d <",
	  i,Win->_firstch[i],Win->_lastch);
	for (j=hoff; j<40+hoff && j < COLS; j++) {
	    if (Win->_y[i][j] & 0x80) {
		putpad(SO);
		printf("%s",unctrl(Win->_y[i][j]&0x7f));
		putpad(SE);
	    } else
		printf("%s",unctrl(Win->_y[i][j]&0x7f));
	}
	printf(">");
	if (i<23)
		printf("\r\n");
    }
    printf("RETURN:");
    fflush(stdout);
    readchar();
    printf("\r\n");
    fflush(stdout);
    printf("Printing each Pixel as '%%02o'\n");
    for (i=0; i<24; i++) {
	printf("%2d ",
	  i,Win->_firstch[i],Win->_lastch);
	for (j=hoff; j<36+hoff && j < COLS; j++) {
	    printf("%02o",Win->_Y[i][j]&077);
	}
	if (i<23)
		printf("\r\n");
    }
    printf("RETURN:");
    fflush(stdout);
    readchar();
    printf("\r\n");
    fflush(stdout);
/*  clearok(curscr,TRUE); */ /* 03-22-83 so color will show */
/*  Draw(curscr); */
    clearok(cw,TRUE);
    Draw(cw);
    fflush(stdout);
    /*
     * Reset the signal in case we got here via an interrupt
     */
    signal(SIGINT, wizquit);
}
#endif

mem()
{
    int		stack;

    msg("data: 0 to %u.  stack: %u. to %u.",sbrk(0),&stack,himem);
}
#endif	SMALL
