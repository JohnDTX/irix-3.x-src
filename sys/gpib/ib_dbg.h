# ifndef _DEBUG_
# define _DEBUG_

# ifdef DEBUG
#	define dprintf(x)	(DEBUG?_dprintf x:0)
#	define ifdebug(x)	(DEBUG?x:0)
#	define difdef(x)	x
#	define dassert(c)	if(!(c))_assert("c",__FILE__,__LINE__)
extern int DEBUG;
# else  DEBUG
#	define dprintf(x)
#	define ifdebug(x)
#	define difdef(x)
#	define dassert(x)
# endif DEBUG


# ifdef DEBUG
#	define SLEEP(e,p)\
	(dprintf((" slp-e")),\
	sleep((char*)e,p),dprintf((" unslp-e")))
#	define WAKEUP(e)\
	(dprintf((" wak-e")),\
	wakeup((char*)e))
# else  DEBUG
#	define SLEEP(e,p)	sleep((char*)e,p)
#	define WAKEUP(e)	wakeup((char*)e)
# endif DEBUG

# ifdef DEBUG
#	define INREG(f)		tlc_inreg(rp,f)
#	define OUTREG(f,x)	tlc_outreg(rp,f,x)
# else  DEBUG
#	define INREG(f)		_INREG(f)
#	define OUTREG(f,x)	_OUTREG(f,x)
# endif DEBUG

# ifdef DEBUG
extern int DEBUG;
extern int ib_debug;
extern int ib_q_debug;
extern int ib_tlc_debug;
extern int ib_subr_debug;
extern int ib_probe_debug;
extern int ib_machdep_debug;
extern int ib_dbg_debug;
extern int ib_board_debug;
extern int ib_trc_debug;
extern int ib_print_debug;
extern int ib_hoff_debug;
# endif DEBUG

# endif _DEBUG_
