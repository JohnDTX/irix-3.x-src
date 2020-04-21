char _Origin_[] = "System V";

/* @(#)ipcs.c	1.4 */

/*
**	ipcs - IPC status
**	Examine and print certain things about message queues, semaphores,
**		and shared memory.
*/

#include	"sys/types.h"
#include	"sys/ipc.h"
#include	"sys/msg.h"
#include	"sys/sem.h"
#ifndef pdp11
#include	"sys/shm.h"
#endif
#include	"nlist.h"
#include	"fcntl.h"
#include	"time.h"
#include	"grp.h"
#include	"pwd.h"
#include	"stdio.h"

#define	TIME	0
#define	MSG	1
#define	SEM	2
#define	SHM	3
#define	MSGINFO	4
#define	SEMINFO	5
#define	SHMINFO	6

struct nlist nl[] = {		/* name list entries for IPC facilities */
	{"_time",},
	{"_msgque",},
	{"_sema",},
	{"_shmem",},
	{"_msginfo",},
	{"_seminfo",},
	{"_shminfo",},
	{NULL}
};
char	chdr[] = "T     ID     KEY        MODE       OWNER    GROUP",
				/* common header format */
	chdr2[] = "  CREATOR   CGROUP",
				/* c option header format */
	*name = "/vmunix",	/* name list file */
	*mem = "/dev/kmem",	/* memory file */
	opts[] = "abcmopqstC:N:";/* allowable options for getopt */
extern char	*optarg;	/* arg pointer for getopt */
int		bflg,		/* biggest size:
					segsz on m; qbytes on q; nsems on s */
		cflg,		/* creator's login and group names */
		mflg,		/* shared memory status */
		oflg,		/* outstanding data:
					nattch on m; cbytes, qnum on q */
		pflg,		/* process id's: lrpid, lspid on q;
					cpid, lpid on m */
		qflg,		/* message queue status */
		sflg,		/* semaphore status */
		tflg,		/* times: atime, ctime, dtime on m;
					ctime, rtime, stime on q;
					ctime, otime on s */
		err;		/* option error count */
extern int	optind;		/* option index for getopt */

extern char		*ctime();
extern struct group	*getgrgid();
extern struct passwd	*getpwuid();
extern long		lseek();

main(argc, argv)
int	argc;	/* arg count */
char	**argv;	/* arg vector */
{
	register int	i,	/* loop control */
			md,	/* memory file file descriptor */
			o;	/* option flag */
	time_t		time;	/* date in memory file */
#ifndef pdp11
	struct shmid_ds	mds;	/* shared memory data structure */
	struct shminfo shminfo;	/* shared memory information structure */
#endif
	struct msqid_ds	qds;	/* message queue data structure */
	struct msginfo msginfo;	/* message information structure */
	struct semid_ds	sds;	/* semaphore data structure */
	struct seminfo seminfo;	/* semaphore information structure */

	/* Go through the options and set flags. */
	while((o = getopt(argc, argv, opts)) != EOF)
		switch(o) {
		case 'a':
			bflg = cflg = oflg = pflg = tflg = 1;
			break;
		case 'b':
			bflg = 1;
			break;
		case 'c':
			cflg = 1;
			break;
		case 'C':
			mem = optarg;
			break;
		case 'm':
#ifdef pdp11
			printf("Shared Memory facility not in system.\n");
#endif
			mflg = 1;
			break;
		case 'N':
			name = optarg;
			break;
		case 'o':
			oflg = 1;
			break;
		case 'p':
			pflg = 1;
			break;
		case 'q':
			qflg = 1;
			break;
		case 's':
			sflg = 1;
			break;
		case 't':
			tflg = 1;
			break;
		case '?':
			err++;
			break;
		}
	if(err || (optind < argc)) {
		fprintf(stderr,
			"usage:  ipcs [-abcmopqst] [-C corefile] [-N namelist]\n");
		exit(1);
	}
	if((mflg + qflg + sflg) == 0)
		mflg = qflg = sflg = 1;

	/* Check out namelist and memory files. */
	nlist(name, nl);
	if(!nl[TIME].n_value) {
		fprintf(stderr, "ipcs:  no namelist\n");
		exit(1);
	}
	if((md = open(mem, O_RDONLY)) < 0) {
		fprintf(stderr, "ipcs:  no memory file\n");
		exit(1);
	}
	lseeke(md, (long)nl[TIME].n_value, 0);
	reade(md, &time, sizeof(time));
	printf("IPC status from %s as of %s", mem, ctime(&time));

	/* Print Message Queue status report. */
	if(qflg) {
		if(nl[MSG].n_value) {
			i = 0;
			lseeke(md, (long)nl[MSGINFO].n_value, 0);
			reade(md, &msginfo, sizeof(msginfo));
			lseeke(md, (long)nl[MSG].n_value, 0);
			printf("%s%s%s%s%s%s\nMessage Queues:\n", chdr,
				cflg ? chdr2 : "",
				oflg ? " CBYTES  QNUM" : "",
				bflg ? " QBYTES" : "",
				pflg ? " LSPID LRPID" : "",
				tflg ? "   STIME    RTIME    CTIME " : "");
		} else {
			i = msginfo.msgmni;
			printf("Message Queue facility not in system.\n");
		}
		while(i < msginfo.msgmni) {
			reade(md, &qds, sizeof(qds));
			if(!(qds.msg_perm.mode & IPC_ALLOC)) {
				i++;
				continue;
			}
			hp('q', "SRrw-rw-rw-", &qds.msg_perm, i++, msginfo.msgmni);
			if(oflg)
				printf("%7u%6u", qds.msg_cbytes, qds.msg_qnum);
			if(bflg)
				printf("%7u", qds.msg_qbytes);
			if(pflg)
				printf("%6u%6u", qds.msg_lspid, qds.msg_lrpid);
			if(tflg) {
				tp(qds.msg_stime);
				tp(qds.msg_rtime);
				tp(qds.msg_ctime);
			}
			printf("\n");
		}
	}

	/* Print Shared Memory status report. */
#ifndef pdp11
	if(mflg) {
		if(nl[SHM].n_value) {
			i = 0;
			lseeke(md, (long)nl[SHMINFO].n_value, 0);
			reade(md, &shminfo, sizeof(shminfo));
			lseeke(md, (long)nl[SHM].n_value, 0);
			if(oflg || bflg || tflg || !qflg || !nl[MSG].n_value)
				printf("%s%s%s%s%s%s\n", chdr,
					cflg ? chdr2 : "",
					oflg ? " NATTCH" : "",
					bflg ? "  SEGSZ" : "",
					pflg ? "  CPID  LPID" : "",
					tflg ? "   ATIME    DTIME    CTIME " : "");
			printf("Shared Memory:\n");
		} else {
			i = shminfo.shmmni;
			printf("Shared Memory facility not in system.\n");
		}
		while(i < shminfo.shmmni) {
			reade(md, &mds, sizeof(mds));
			if(!(mds.shm_perm.mode & IPC_ALLOC)) {
				i++;
				continue;
			}
			hp('m', "DCrw-rw-rw-", &mds.shm_perm, i++, shminfo.shmmni);
			if(oflg)
				printf("%7u", mds.shm_nattch);
			if(bflg)
				printf("%7d", mds.shm_segsz);
			if(pflg)
				printf("%6u%6u", mds.shm_cpid, mds.shm_lpid);
			if(tflg) {
				tp(mds.shm_atime);
				tp(mds.shm_dtime);
				tp(mds.shm_ctime);
			}
			printf("\n");
		}
	}
#endif

	/* Print Semaphore facility status. */
	if(sflg) {
		if(nl[SEM].n_value) {
			i = 0;
			lseeke(md, (long)nl[SEMINFO].n_value, 0);
			reade(md, &seminfo, sizeof(seminfo));
			lseeke(md, (long)nl[SEM].n_value, 0);
			if(bflg || tflg || (!qflg || !nl[MSG].n_value) &&
				(!mflg || !nl[SHM].n_value))
				printf("%s%s%s%s\n", chdr,
					cflg ? chdr2 : "",
					bflg ? " NSEMS" : "",
					tflg ? "   OTIME    CTIME " : "");
			printf("Semaphores:\n");
		} else {
			i = seminfo.semmni;
			printf("Semaphore facility not in system.\n");
		}
		while(i < seminfo.semmni) {
			reade(md, &sds, sizeof(sds));
			if(!(sds.sem_perm.mode & IPC_ALLOC)) {
				i++;
				continue;
			}
			hp('s', "--ra-ra-ra-", &sds.sem_perm, i++, seminfo.semmni);
			if(bflg)
				printf("%6u", sds.sem_nsems);
			if(tflg) {
				tp(sds.sem_otime);
				tp(sds.sem_ctime);
			}
			printf("\n");
		}
	}
	exit(0);
}

/*
**	lseeke - lseek with error exit
*/

lseeke(f, o, w)
int	f,	/* fd */
	w;	/* whence */
long	o;	/* offset */
{
#ifdef vax
	o &= 0x3fffffff;
#endif
	if(lseek(f, o, w) < 0) {
		perror("ipcs:  seek error");
		exit(1);
	}
}

/*
**	reade - read with error exit
*/

reade(f, b, s)
int	f,	/* fd */
	s;	/* size */
char	*b;	/* buffer address */
{
	if(read(f, b, s) != s) {
		perror("ipcs:  read error");
		exit(1);
	}
}

/*
**	hp - common header print
*/

hp(type, modesp, permp, slot, slots)
char				type,	/* facility type */
				*modesp;/* ptr to mode replacement characters */
register struct ipc_perm	*permp;	/* ptr to permission structure */
int				slot,	/* facility slot number */
				slots;	/* # of facility slots */
{
	register int		i,	/* loop control */
				j;	/* loop control */
	register struct group	*g;	/* ptr to group group entry */
	register struct passwd	*u;	/* ptr to user passwd entry */

	printf("%c%7d%s%#8.8x ", type, slot + slots * permp->seq,
		permp->key ? " " : " 0x", permp->key);
	for(i = 02000;i;modesp++, i >>= 1)
		printf("%c", (permp->mode & i) ? *modesp : '-');
	if((u = getpwuid(permp->uid)) == NULL)
		printf("%9d", permp->uid);
	else
		printf("%9.8s", u->pw_name);
	if((g = getgrgid(permp->gid)) == NULL)
		printf("%9d", permp->gid);
	else
		printf("%9.8s", g->gr_name);
	if(cflg) {
		if((u = getpwuid(permp->cuid)) == NULL)
			printf("%9d", permp->cuid);
		else
			printf("%9.8s", u->pw_name);
		if((g = getgrgid(permp->cgid)) == NULL)
			printf("%9d", permp->cgid);
		else
			printf("%9.8s", g->gr_name);
	}
}

/*
**	tp - time entry printer
*/

tp(time)
time_t	time;	/* time to be displayed */
{
	register struct tm	*t;	/* ptr to converted time */

	if(time) {
		t = localtime(&time);
		printf(" %2d:%2.2d:%2.2d", t->tm_hour, t->tm_min, t->tm_sec);
	} else
		printf(" no-entry");
}
