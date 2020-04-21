/*
 * Definitions for advice user can give to the kernel.
 *
 * $Source: /d2/3.7/src/sys/h/RCS/advice.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:29:09 $
 */

/* fadvise() stuff */

/*
 * Inform the system that sequential i/o is going to be done on the
 * given file descriptor, and that the i/o is not likely to be
 * re-refrenced.  Good for programs scanning large linear files.
 */
#define	ADVISE_SEQUENTIAL	0x0001
