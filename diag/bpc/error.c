/*
 *	Kurt Akeley
 *	11 Sept 1984
 *
 *	Simple error handling package for bpcd
 *
 *	Functions
 *		long geterrors ()
 *
 *	Procedures
 *		initerror (count)
 *		adderror (verbose)
 */

static long errortotal = 0;	/* total errors */
static long messagecount = 0;	/* message count in this cycle */
static long messagecycle = 10;	/* number of messages per cycle */

initerror (count)
long count;
{
    errortotal = 0;
    messagecount = 0;
    messagecycle = count;
    }

adderror (verbose)
long verbose;
{
    errortotal += 1;
    if (verbose) {
	messagecount += 1;
	if (messagecount >= messagecycle) {
	    printf ("%d errors, <return> to continue, <space> to quit ... ",
		errortotal);
	    while (messagecount) {
		switch (negetch ()) {
		    case 015:
		    case 012:
			messagecount = 0;
			newline ();
			break;
		    case ' ':
			dobreak ();
			break;
		    }
		}
	    }
	}
    }

long geterrors () {
    return errortotal;
    }
