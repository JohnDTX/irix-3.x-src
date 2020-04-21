char _Origin_[] = "System V";
/*	@(#)killall.c	1.3	*/
static	char	*Sccsid = "@(#)$Header: /d2/3.7/src/etc/RCS/killall.c,v 1.1 89/03/27 15:38:05 root Exp $";
/*
 * $Log:	killall.c,v $
 * Revision 1.1  89/03/27  15:38:05  root
 * Initial check-in for 3.7
 * 
 * Revision 1.4  85/06/24  16:04:55  bob
 * Changed to compile since sys/var.h went away.
 * 
 * Revision 1.3  85/03/22  14:45:33  bob
 * Fixed to deal with VM (_nproc, /dev/kmem, thingking).
 * 
 */

/*
 *  Go through the process table and kill all valid processes that
 *  are not Process 0 (sched) or Process 1 (init) or that do not
 *  have the current process group leader id.
 */

#include	<stdio.h>
#include	<nlist.h>
#include	<fcntl.h>
#include	<sys/types.h>
#ifdef m68000
#include	<sys/param.h>
#else m68000
*********** Needs work for other processors ***********
#endif m68000
#ifdef u3b
#include	<sys/seg.h>
#include	<sys/param.h>
#endif u3b
#include	<sys/proc.h>
#ifdef	BRAINLESS
#include	<sys/config.h>
#endif	BRAINLESS
#ifndef	sgi
#include	<sys/var.h>
#endif	sgi

#define	ERR	(-1)
#define	PROC	nl[0]
#define	NPROC	nl[1]
#define	PROCSZ	nproc
#ifdef u370
				/* no sched in u370 */
#define	FIRSTPROC	1
#else
#define	FIRSTPROC	3	/* 0=sched 1=init 2=thingking */
#endif

/*
 * u3b warning:  nl[0] will be "proc" and nl[1] will be "_v"
 * regardless of what variables appear below.
 */
#if pdp11 || vax || m68000
#define PROC_STR	"_proc"
#define NPROC_STR	"_nproc"
#endif
#if u3b || u370
#define PROC_STR	"proc"
#define NPROC_STR	"nproc"
#endif

struct nlist nl[] = {
	{ PROC_STR },
	{ NPROC_STR },
	{ 0 },
};

main(argc, argv)
int  argc;
char *argv[];
{
	register int	i = 0;
	register int	sig, fd, pgrp;
	unsigned int	Proc, Nproc, nproc;
	struct proc	*p;

	switch(argc) {
		case 1:
			sig = 9;
			break;
		case 2:
			sig = atoi(argv[1]);
			break;
		default:
			fprintf(stderr, "Usage: %s [signal]\n", argv[0]);
			exit(1);
	}

	if((fd = open("/dev/kmem", O_RDONLY)) == ERR) {
		perror("killall:read on kmem");
		exit(1);
	}

	nl[0].n_value = 0;
	nl[1].n_value = 0;

#ifdef u3b
	nl[0].n_value = sys3b(4,1); /* proc */
	nl[1].n_value = sys3b(4,4); /* nproc */
#else /* u3b */
	nlist("/vmunix", nl);

	if(PROC.n_value == 0) {
		fprintf(stderr, "%s: can't find process table\n", argv[0]);
		exit(1);
	}

	if(NPROC.n_value == 0) {
		fprintf(stderr, "%s: can't find process table size\n", argv[0]);
		exit(1);
	}
#endif /* u3b */

	Proc = (PROC.n_value);
	Nproc = (NPROC.n_value);

	if((pgrp = getpgrp()) == ERR) {
		perror("getpgrp");
		exit(1);
	}

#ifdef	DEBUG
printf("effective pgrp = %d\n", pgrp);
#endif

/*
 *  Read in the variable table so that the current process
 *  table size can be used.
 */

	if(lseek(fd, (long)Nproc, 0) == ERR) {
		perror("lseek on variable table");
		exit(1);
	}

	if(read(fd, &nproc, sizeof nproc) == ERR) {
		perror("read err on nproc");
		exit(1);
	}

/*
 *  Seek to process table and on past slots 0 and 1
 *  which are the scheduler and init.
 */
	/* SGI VM UNIX stores a pointer to the process table */
	if(lseek(fd, Proc, 0) == ERR) {
		perror("lseek on process table address");
		exit(1);
	}
	if(read(fd,&Proc,sizeof(Proc)) == ERR) {
		perror("read on process table address ");
		exit(1);
	}
	if(lseek(fd, (long)Proc, 0) == ERR) {
		perror("lseek on process table");
		exit(1);
	}
#ifdef	DEBUG
	printf("procsz=%d\n",PROCSZ);
#endif

	/* allocate a process table */
	if ((p = (struct proc *)malloc(sizeof(struct proc) * PROCSZ)) == 0) {
		perror("malloc on process table copy");
		exit(1);
	}
	if(read(fd, p, sizeof(struct proc) * PROCSZ) == ERR) {
		perror("read err on proc table");
		exit(1);
	}
	for(i=FIRSTPROC; i < PROCSZ; ++i) {
		if(p[i].p_stat == 0 || p[i].p_stat == SZOMB
		  || p[i].p_pgrp == pgrp || p[i].p_pid == 0)
			continue;
		else {
#ifdef	DEBUG
			printf("slot=%4d: kill(%6d,%2d)\tuid=%5u\tp_pgrp=%5d\n",
				i,p[i].p_pid,sig,p[i].p_uid,p[i].p_pgrp);
#else
			/* kill process group leaders */
			if(kill(p[i].p_pid, sig) == ERR) {
				fprintf(stderr, "kill: pid= %d: ", p[i].p_pid);
				perror("");
			}
#endif
		}
	}
	exit(0);
}
