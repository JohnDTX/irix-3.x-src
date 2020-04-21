/*
 * 'Compatibility definitions, gathered from various 3B2 5.3 files
 */


/*
 * buffer configuration information from 3B2 var.h and master.d/kernel
 */
struct val {
	int	v_nblk4096;		/* # of 4096 bytes stream buffers.*/
	int	v_nblk2048;		/* # of 2048 bytes stream buffers.*/
	int	v_nblk1024;		/* # of 1024 bytes stream buffers.*/
	int	v_nblk512;		/* # of 512 bytes stream buffers.*/
	int	v_nblk256;		/* # of 256 bytes stream buffers.*/
	int	v_nblk128;		/* # of 128 bytes stream buffers.*/
	int	v_nblk64;		/* # of 64 bytes stream buffers.*/
	int	v_nblk16;		/* # of 16 bytes stream buffers.*/
	int	v_nblk4;		/* # of 4 bytes stream buffers.	*/
};
#ifndef STREAM_DEF
extern struct val v;
#endif


/*
 * constants from master.d
 *	all of these numbers are arbitrary, and surely wrong.
 */
#define NBLK4096	2
#define NBLK2048	2
#define NBLK1024	2
#define NBLK512		16
#define NBLK256		16
#define NBLK128		32
#define NBLK64		64
#define NBLK16		64
#define NBLK4		256
#define NMUXLINK	4
#define NSTREVENT	20
#define MAXSEPGCNT	1

/* these values came from the 3B2 master.d/kernel, so they may have be
 *	adjusted */
#define NSTRPUSH	9
#define STRLOFRAC	80
#define STRMEDFRAC	90
#define STRMSGSZ	4096
#define STRCTLSZ	1024



#ifndef STREAM_DEF
extern int fmodcnt;
extern struct fmodsw fmodsw[];
#endif

#define min(a,b) MIN(a,b)
