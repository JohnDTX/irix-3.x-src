/*
 * Kernel version types
 */

/*
 * Return the current running version of the kernel.
 * Kernel version strings are arbitrary in nature, and thus
 * have no specific format.  Typically though, they will be
 * of the format "X.Y" where X is the major release version and
 * Y is the minor release version.  Example: "3.0"
 */
#define	KVERS_KERNEL	0		/* kernel version */

/*
 * Return the graphics hardware that this system supports.  Currently
 * supported return values are "GL1" or "GL2".
 */
#define	KVERS_GLTYPE	1		/* gl type this kernel supports */

/*
 * Return the current graphics library version that this kernel supports.
 * This will be a string of the form "GLX.Y" where X is the type of
 * graphics, and Y is the version.  Example: "GL2.26"
 */
#define	KVERS_GL	2		/* gl version */

/*
 * Return the cpu type of the running machine.  This will return a
 * string with the following possible answers: "pmII" or "ipII".
 */
#define	KVERS_CPUTYPE	3		/* processor type */

/*
 * Return a flag indicating whether or not the running machine has a
 * floating point accelerator.  Possible answers are "yes" or "no".
 */
#define	KVERS_HAVEFPA	4		/* have we a fpa? */

#ifdef	KERNEL
#define	KVERS_LAST	4		/* last kvers type supported */
#endif
