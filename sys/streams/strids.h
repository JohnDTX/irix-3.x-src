/* Stream module ID and M_CTL registration
 *
 *	AT&T does not seem to care what numbers anything uses.  This is an SGI
 *	hack to reduce chaos.
 *
 *	
 * $Source: /d2/3.7/src/sys/streams/RCS/strids.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:34:56 $
 */



/* SGI stream module IDs
 */
#define STRID_DUART	9001		/* stream duart driver */
#define STRID_STTY_LD	9002		/* stream line discipline */
#define STRID_WIN	9003		/* stream windows */
#define STRID_PTC	9004		/* stream control pty */
#define STRID_PTS	9005		/* stream slave pty */
#define	STRID_CD3608	9006		/* stream central data serial driver */
#define	STRID_ISI16	9007		/* stream ISI 16 port serial driver */
#define STRID_PROM	9008		/* stream PROM console driver */
#define	STRID_GM	9009		/* stream GM control port */
#define	STRID_WM_QUEUE	9010		/* stream window mgr queue module */
#define STRID_IF_CY	9011		/* cypress network interface */
#define	STRID_HACK	9012		/* iris console stream hack */

/* IDs for M_CTL messages
 *	One of these ints should be the first 4 bytes of an M_CTL message
 */
#define STRCTL__			/* none defined yet */
