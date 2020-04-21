static	char	*Daemon_c	= "@(#)daemon.c	1.9";
/*
 * Contains functions for dealing with things that happen in the
 * future.
 *
 * @(#)daemon.c	3.3 (Berkeley) 6/15/81
 */

#include "rogue.h"

#define EMPTY 0
#define DAEMON -1
#define MAXDAEMONS 20

#define _X_ { EMPTY }

struct	delayed_action {
    int		d_type;
    int		(*d_func)();
    int		d_arg;
    int		d_time;
} d_list[MAXDAEMONS] = {
    _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_,
    _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, 
};

/*
 * d_slot:
 *	Find an empty slot in the daemon/fuse list
 */
struct	delayed_action *
d_slot()
{
    reg1 regd1	int	i;
    reg2 rega1	struct	delayed_action *dev;

    for (i = 0, dev = d_list; i < MAXDAEMONS; i++, dev++)
	if (dev->d_type == EMPTY)
	    return dev;
    debug("Ran out of fuse slots");
    return NULL;
}

/*
 * find_slot:
 *	Find a particular slot in the table
 */

struct	delayed_action *
find_slot(func)
reg1 rega1	int	(*func)();
{
    reg2 regd1	int	i;
    reg3 rega2	struct	delayed_action *dev;

    for (i = 0, dev = d_list; i < MAXDAEMONS; i++, dev++)
	if (dev->d_type != EMPTY && func == dev->d_func)
	    return dev;
    return NULL;
}

/*
 * daemon:
 *	Start a daemon, takes a function.
 */

daemon(func, arg, type)
int	(*func)(), arg, type;
{
    reg1 rega1	struct	delayed_action *dev;

    dev = d_slot();
    dev->d_type = type;
    dev->d_func = func;
    dev->d_arg = arg;
    dev->d_time = DAEMON;
}

/*
 * kill_daemon:
 *	Remove a daemon from the list
 */

kill_daemon(func)
int	(*func)();
{
    reg1 rega1	struct	delayed_action *dev;

    if ((dev = find_slot(func)) == NULL)
	return;
    /*
     * Take it out of the list
     */
    dev->d_type = EMPTY;
}

/*
 * do_daemons:
 *	Run all the daemons that are active with the current flag,
 *	passing the argument to the function.
 */

do_daemons(flag)
reg1 regd1	int	flag;
{
    reg2 rega1	struct	delayed_action *dev;

    /*
     * Loop through the devil list
     */
    for (dev = d_list; dev < &d_list[MAXDAEMONS]; dev++)
	/*
	 * Executing each one, giving it the proper arguments
	 */
	if (dev->d_type == flag && dev->d_time == DAEMON)
	    (*dev->d_func)(dev->d_arg);
}

/*
 * fuse:
 *	Start a fuse to go off in a certain number of turns
 */

fuse(func, arg, time, type)
int	(*func)(), arg, time, type;
{
    reg1 rega1	struct	delayed_action *wire;

    wire = d_slot();
    wire->d_type = type;
    wire->d_func = func;
    wire->d_arg = arg;
    wire->d_time = time;
}

/*
 * lengthen:
 *	Increase the time until a fuse goes off
 */

lengthen(func, xtime)
int	(*func)();
int	xtime;
{
    reg1 rega1	struct	delayed_action *wire;

    if ((wire = find_slot(func)) == NULL)
	return;
    wire->d_time += xtime;
}

/*
 * extinguish:
 *	Put out a fuse
 */

extinguish(func)
int	(*func)();
{
    reg1 rega1	struct	delayed_action *wire;

    if ((wire = find_slot(func)) == NULL)
	return;
    wire->d_type = EMPTY;
}

/*
 * do_fuses:
 *	Decrement counters and start needed fuses
 */

do_fuses(flag)
reg1 regd1	int	flag;
{
    reg2 rega1	struct	delayed_action *wire;

    /*
     * Step though the list
     */
    for (wire = d_list; wire < &d_list[MAXDAEMONS]; wire++) {
	/*
	 * Decrementing counters and starting things we want.  We also need
	 * to remove the fuse from the list once it has gone off.
	 */
	if (flag == wire->d_type && wire->d_time > 0 && --wire->d_time == 0) {
	    wire->d_type = EMPTY;
	    (*wire->d_func)(wire->d_arg);
	}
    }
}

/*
 * dftype(func)
 *	return name of function if known, else its address
 */

#ifdef WIZD
char	*
dftype(func)
int	(*func)();
{
	extern	int	swander();
	extern	int	rollwand();
	extern	int	unconfuse();
	extern	int	unsee();
	extern	int	nohaste();
	extern	int	runners();

	static	struct	{
		int	(*fptr)();
		char	*fname;
	} funcs[] = {
		swander,"swander",
		rollwand,"rollwand",
		unconfuse,"unconfuse",
		unsee,"unsee",
		nohaste,"nohaste",
		runners,"runners",
		0,0
	};
	int	i;

	for (i=0; funcs[i].fptr; i++)
		if (funcs[i].fptr == func)
			return funcs[i].fname;
	sprintf(prbuf,"%u",func);
	return prbuf;
}
#endif

/*
 * dae_list:
 *	Display queue of fuses and daemons
 */

#ifndef	SMALL
#ifdef WIZD
dae_list()
{
    reg1 rega1	struct	linked_list *item;
    reg2 rega2	struct	delayed_action *dev;
		int	i;
		char	*type;

    wclear(hw);
    wmove(hw,1,0);
    wprintw(hw,"Daemons and Fuses:\n");
    for (dev = d_list; dev < &d_list[MAXDAEMONS]; dev++) {
	/*
	 * Decrementing counters and starting things we want.  We also need
	 * to remove the fuse from the list once it has gone off.
	 */
	switch (dev->d_type) {
	  case BEFORE:	type = "Before";
	  when AFTER:	type = "After";
	  when EMPTY:	type = "Empty";
	  otherwise:	type = "Unknown";
	}
	if (dev->d_time == DAEMON)
	    strcpy(prbuf,"Daemon");
	else
	    sprintf(prbuf,"%6d turns",dev->d_time);
	wprintw(hw,"%4x:  %-12s  %8s  arg:%5u  %s\n",
	  dev,
	  dftype(dev->d_func),
	  type,
	  dev->d_arg,
	  prbuf);
    }
    wprintw(hw,"\nDaemons:\n");
    wprintw(hw,"Re-Code for Rogue 3.6\n");
#ifdef	COLOR
    wprintw(hw,
      "\nForeground color:%d  Background color:%d Number of colors:%d\n",
      Dforeground,Dbackground,Ncolors);
    for (i=0; i<Ncolors; i++) {
	wprintw(hw, "Color ");
	if (i == Dbackground)
	    acolor(hw,Dforeground,i);
	else
	    acolor(hw,Dbackground,i);
	wprintw(hw, " %d is ", i);
	if (i == Dbackground)
	    acolor(hw,i,Dforeground);
	else
	    acolor(hw,i,Dbackground);
	wprintw(hw, "%s",Colors[i]);
	acolor(hw,Dforeground,Dbackground);
	wprintw(hw, ".\n");
    }
#endif
    wmove(hw, 0, 0);
    wprintw(hw,"Press space to continue");
    Draw(hw);
    wait_prompt(hw, ' ');
    wmove(cw, 0, 0);
    wclrtoeol(cw);
    status();
    touchwin(cw);
}
#endif
#endif	SMALL
