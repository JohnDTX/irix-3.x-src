/*
 *	Kurt Akeley
 *	31 may 1985
 *
 *	Intercept all putchar() calls and store characters in a
 *	  circular buffer.  Allow the contents of the buffer to
 *	  be printed.
 *
 *	Procedures:
 *	Functions:
 *	Static procedures:
 *	Static functions:
 */

#define CIRCBUF		5000

static char firstpoint[CIRCBUF];	/* first char locin buffer space */
static char *lastpoint;			/* last char loc in buffer space */
static char *startpoint;		/* first to be printed */
static char *endpoint;			/* one past last to be printed */

putchar (c)
int c;
{
    addchar (c);
    putcraw (c, 1);
    if (c == '\n')
	putcraw ('\r', 1);
    }

initcircbuf () {
    lastpoint = firstpoint + (CIRCBUF - 1);
    startpoint = endpoint = firstpoint;
    }    

static char *nextpoint (p)
char *p;
{
    if (p == lastpoint)
	p = firstpoint;
    else
	p = p+1;
    return p;
    }

static char *prevpoint (p)
char *p;
{
    if (p == firstpoint)
	p = lastpoint;
    else
	p = p-1;
    return p;
    }

static addchar (c)
char c;
{
    if (c == '\r')
	;
    else if (c == '\b')
	endpoint = prevpoint (endpoint);
    else {
	*endpoint = c;
	endpoint = nextpoint (endpoint);
	if (endpoint == startpoint) {
	    startpoint = nextpoint (startpoint);
	    while (*startpoint != '\n') {
		startpoint = nextpoint (startpoint);
		if (startpoint == endpoint)
		    break;
		}
	    startpoint = nextpoint (startpoint);
	    }
	}
    }

seecircbuf () {
    /* print the contents of the circular buffer */
    char *p;
    int count;
    int first;

    p = startpoint;
    count = 0;
    first = 1;
    while (p != endpoint) {
	if (first) {
	    putcraw ('|', 1);
	    putcraw (' ', 1);
	    first = 0;
	    }
	putcraw (*p, 1);
	if (*p == '\n') {
	    putcraw ('\r', 1);
	    count += 1;
	    first = 1;
	    if (count > 18) {
		char c;
		count = 0;
		putstring ("quit? [yn] ");
		c = negetch ();
		backup (11);
		if (c == 'y' || c == 'q')
		    break;
		}
	    }
	p = nextpoint (p);
	}
    }

static backup (count)
int count;
{
    int i;
    for (i=0; i < count; i++) {
	putcraw ('\b', 1);
	putcraw (' ', 1);
	putcraw ('\b', 1);
	}
    }

static putstring (s)
char *s;
{
    while (*s)
	putcraw (*s++, 1);
    }
