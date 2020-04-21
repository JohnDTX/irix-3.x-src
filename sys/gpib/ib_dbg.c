# undef  DEBUG
#include "ib.h"
/*
 * ib_dbg.c --
 * debugging / diagnostic routines for ib driver.
 */

#include "../h/param.h"
#include "../h/types.h"
#include "../h/buf.h"
#include "../h/tty.h"


# ifdef DEBUG
# undef DEBUG
# define DEBUG ib_dbg_debug
# endif DEBUG
#include "../h/ib_ioctl.h"
#include "../gpib/ib_ieee.h"
#include "../gpib/ib_reg.h"
#include "../gpib/ib.defs"
#include "../gpib/ib_dbg.h"


# ifdef DEBUG

int ASSERTFAIL = 2;
_assert(s,f,l)
    char *s;
    char *f;
    int l;
{
    ib_print_debug++;
    _dprintf(" !dassert %s \"%s\"/%d\n",s,f,l);
    if( --ASSERTFAIL <= 0 )
	panic("assert");
    ib_print_debug--;
}

# include "ib_trc.h"
int ntrace;
struct trace _trcbuf[NTCELLS];
struct trace *_trc = _trcbuf+0;
typedef struct { int x[4]; } A;

_trace(a)
    A a;
{
    register struct trace *tp;
    tp = _trc;

if(!(_trcbuf<=tp&&tp<_trcbuf+NTCELLS))panic("TRACE BOTCH");
    tp->when = ++ntrace;
    *((A *)tp->args) = a;
    tp++;
    if( tp == _trcbuf+NTCELLS )
	tp = _trcbuf;
    _trc = tp;
}

_dprintf(a)
    A a;
{
    if( ib_print_debug )
	printf(a);
    if( ib_trc_debug )
	_trace(a);
}

prdata(s,n)
    register u_char *s;
    int n;
{

    _dprintf(" <%02x %02x %02x",s[0],s[1],s[2]);
    _dprintf(" %02x %02x %02x",s[3],s[4],s[5]);
    _dprintf(" %02x %02x>",s[6],s[7]);
}
# endif DEBUG
