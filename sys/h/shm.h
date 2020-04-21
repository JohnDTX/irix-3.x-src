/*
**	IPC Shared Memory Facility.
**
** $Source: /d2/3.7/src/sys/h/RCS/shm.h,v $
** $Revision: 1.1 $
** $Date: 89/03/27 17:29:59 $
*/

/*
**	Implementation Constants.
*/

#define	SHMLBA	ctob(1)	/* segment low boundary address multiple */
			/* (SHMLBA must be a power of 2) */

/*
**	Permission Definitions.
*/

#define	SHM_R	0400	/* read permission */
#define	SHM_W	0200	/* write permission */

/*
**	ipc_perm Mode Definitions.
*/

#define	SHM_CLEAR	01000	/* clear segment on next attach */
#define	SHM_DEST	02000	/* destroy segment when # attached = 0 */

/*
**	Message Operation Flags.
*/

#define	SHM_RDONLY	010000	/* attach read-only (else read-write) */
#define	SHM_RND		020000	/* round attach address to SHMLBA */

/*
**	Structure Definitions.
*/

/*
** There is a shared mem id data structure for each segment in the system.
*/
struct shmid_ds {
	struct	ipc_perm shm_perm;	/* operation permission struct */
	/* THE ABOVE STRUCTURE MUST BE THE FIRST ITEM IN THIS STRUCTURE */

	long 	shm_segsz;		/* segment size (in bytes) */
/*	short 	shm_scat;		/* scatter load memory pointers */
	ushort 	shm_lpid;		/* pid of last shmop */
	ushort	shm_cpid;		/* pid of creator */
	short	shm_nattch;		/* current # attached */
	short	shm_cnattch;		/* in memory # attached */
	time_t	shm_atime;		/* last shmat time */
	time_t	shm_dtime;		/* last shmdt time */
	time_t	shm_ctime;		/* last change time */
	struct	pte *shm_ptbl;		/* base of page table */
	/* XXX */
	/*
	 * When shared memory segments are page-able, then some more
	 * stuff will be needed
	 */
/*	long	shm_swapaddr;		/* disk address for paging */
};

struct	shmpt_ds {
	short	shm_segbeg;	/* virtual start of shared data (in clicks) */
	short	shm_sflg;	/* R/W permission on segment */
};

struct	shminfo {
	int	shmmax,	/* max shared memory segment size */
		shmmin,	/* min shared memory segment size */
		shmmni,	/* # of shared memory identifiers */
		shmseg,	/* max attached shared memory segments per process */
		shmbrk,	/* gap (in clicks) used between data and shared mem */
		shmall;	/* max total shared memory system wide (in clicks) */
};

#ifdef	KERNEL
extern	struct shmid_ds	shmem[];	/* shared memory headers */
extern	struct shmid_ds	*shm_shmem[];	/* ptrs to attached segments */
extern	struct shmpt_ds shm_ptbl[];	/* mapping info per segment per user */
extern	struct shminfo	shminfo;	/* shared memory info structure */
#endif
