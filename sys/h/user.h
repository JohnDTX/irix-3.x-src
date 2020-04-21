/*
 * The user structure.
 * One allocated per process.
 * Contains all per process data that doesn't need to be referenced
 * while the process is swapped.
 * The user block is USIZE*click bytes long.
 * It contains the system stack per user; is cross referenced
 * with the proc structure for the same process.
 *
 * $Source: /d2/3.7/src/sys/h/RCS/user.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:30:17 $
 */

#include "machine/pcb.h"
#ifdef	KERNEL
#include "../h/dmap.h"
#include "../h/errno.h"
#include "../h/dir.h"
#else
#include <sys/dmap.h>
#include <sys/errno.h>
#include <sys/dir.h>
#endif

#define	SHSIZE	32

/*
 * This structure defines all the state present in the user struct that is
 * needed to do i/o for the user (except for u_error)
 */
struct	userio {
	caddr_t	uu_base;
	u_int	uu_count;
	off_t	uu_offset;
	char	uu_segflg;
	short	uu_fmode;
	ushort	uu_pbsize;
	ushort	uu_pboff;
	dev_t	uu_pbdev;
};

#ifdef	KERNEL
/* these defines make old code work */
#define	u_base		u_io.uu_base
#define	u_count		u_io.uu_count
#define	u_offset	u_io.uu_offset
#define	u_segflg	u_io.uu_segflg
#define	u_fmode		u_io.uu_fmode
#define	u_pbsize	u_io.uu_pbsize
#define	u_pboff		u_io.uu_pboff
#define	u_pbdev		u_io.uu_pbdev
#endif	KERNEL

struct	user {
	struct	pcb u_pcb;
	struct	proc *u_procp;		/* pointer to proc structure */
	char	u_comm[DIRSIZ];
	label_t	u_rsave;		/* save info when exchanging stacks */
	long	u_loadaddr;		/* load address of user in vmem */

	/* system calls */
	label_t	u_qsave;		/* label variable for tty interrupts */
	int	*u_ar0;			/* address of users saved R0 */
	int	*u_ap;			/* pointer to arglist */
	int	u_arg[10];		/* arguments to current system call */
	short	u_errcnt;		/* syscall error count */
	char	u_error;		/* return error code */
	union {				/* syscall return values */
		struct	{
			int	r_val1;
			int	r_val2;
		}r_reg;
		off_t	r_off;
		time_t	r_time;
#define	u_rval1	u_r.r_reg.r_val1
#define	u_rval2	u_r.r_reg.r_val2
#define	u_roff	u_r.r_off
#define	u_rtime	u_r.r_time
	} u_r;
	struct	userio u_io;		/* all the stuff for user i/o */

	/* descriptor management */
	struct file *u_ofile[NOFILE];	/* file structures of open files */
	char	u_pofile[NOFILE];	/* per-process flags of open files */
#define	EXCLOSE	01
	struct inode *u_cdir;		/* inode of current directory */
	struct inode *u_rdir;		/* root directory of current process */
	short	u_cmask;		/* mask for file creation */
	short	*u_ttyp;		/* pointer to pgrp in "tty" struct */
	dev_t	u_ttyd;			/* controlling tty dev */
	struct inode *u_ttyip;		/* controlling tty (stream) inode */

	/* resource control */
	daddr_t	u_limit;		/* maximum write address */

	/* memory management */
	size_t	u_tsize;		/* text size (clicks) */
	size_t	u_dsize;		/* data size (clicks) */
	size_t	u_ssize;		/* stack size (clicks) */
	struct	dmap u_dmap;		/* disk map for data segment */
	struct	dmap u_smap;		/* disk map for stack segment */
	struct	dmap u_cdmap, u_csmap;	/* shadows of u_dmap, u_smap, for
					   use of parent during fork */
	label_t u_ssave;		/* label variable for swapping */
	size_t	u_odsize, u_ossize;	/* for (clumsy) expansion swaps */
	time_t	u_outime;		/* user time at last sample */

	/* processes and protection */
	ushort	u_uid;			/* effective user id */
	ushort	u_gid;			/* effective group id */
	ushort	u_ruid;			/* real user id */
	ushort	u_rgid;			/* real group id */

	/* signals */
	int	u_signal[NSIG];		/* disposition of signals */

	/* timing and statistics */
	time_t	u_utime;		/* this process user time */
	time_t	u_stime;		/* this process system time */
	time_t	u_cutime;		/* sum of childs' utimes */
	time_t	u_cstime;		/* sum of childs' stimes */
	long	u_mem;
	long	u_ior;
	long	u_iow;
	long	u_iosw;
	long	u_ioch;
	time_t	u_start;
	time_t	u_ticks;
	char	u_acflag;

	/* profiling */
	struct {			/* profile arguments */
		short	*pr_base;	/* buffer base */
		unsigned pr_size;	/* buffer size */
		unsigned pr_off;	/* pc offset */
		unsigned pr_scale;	/* pc scaling */
	} u_prof;

	caddr_t	u_dirp;			/* pathname pointer */
	struct direct u_dent;		/* current directory entry */
	struct inode *u_pdir;		/* inode of parent directory of dirp */
	dev_t	u_scandev;		/* dev of scan rotor */
	ino_t	u_scaninum;		/* ino of scan rotor */
	off_t	u_scanpos;		/* rotor for consecutive scans */
	union {
		struct exdata {			/* header of executable file */
			long	Ux_mag;		/* magic number */
			long	Ux_tsize;	/* text size */
			long	Ux_dsize;	/* data size */
			long	Ux_bsize;	/* bss size */
			long	Ux_ssize;	/* symbol table size */
			long	Ux_rtsize;
			long	Ux_rdsize;
			long	Ux_entloc;	/* entry location */
		} Ux_A;
#define	ux_mag		Ux_A.Ux_mag
#define	ux_tsize	Ux_A.Ux_tsize
#define	ux_dsize	Ux_A.Ux_dsize
#define	ux_bsize	Ux_A.Ux_bsize
#define	ux_entloc	Ux_A.Ux_entloc
		char	ux_shell[SHSIZE];
	} u_exdata;

	short	u_lock;			/* process/text locking flags */

	/* red limit for kernel stack */
	int	u_stack[1];
};

#ifdef KERNEL
extern	struct user u;
extern	struct user swaputl;
extern	struct user forkutl;
extern	struct user xswaputl;
extern	struct user xswap2utl;
extern	struct user pushutl;
extern	struct user vfutl;
#endif
