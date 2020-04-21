/*
 * $Source: /d2/3.7/src/sys/sys/RCS/sysent.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:35:39 $
 */
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/sysent.h"

#include "nfs.h"
#include "tcp.h"
#include "ustrm.h"
#include "sgigsc.h"

/*
 * This table is the switch used to transfer
 * to the appropriate routine for processing a system call.
 */

int	alarm();
int	chdir();
int	chmod();
int	chown();
int	chroot();
int	close();
int	creat();
int	dup();
int	exec();
int	exece();
int	fcntl();
int	fork();
int	oldfstat();
int	fstat();
int	getgid();
int	getpid();
int	getuid();
int	gtime();
int	ioctl();
int	kill();
int	link();
int	lock();
int	mkdir();
int	mknod();
int	msgsys();
int	nice();
int	nosys();
int	nullsys();
int	open();
int	pause();
int	pipe();
int	profil();
int	ptrace();
int	read();
int	rexit();
int	rmdir();
int	saccess();
int	sbreak();
int	seek();
int	semsys();
int	setgid();
int	setpgrp();
int	setuid();
int	shmsys();
int	smount();
int	ssig();
int	oldstat();
int	stat();
int	stime();
int	sumount();
int	sync();
int	sysacct();
int	times();
int	ulimit();
int	umask();
int	unlink();
int	utime();
int	utssys();
int	wait();
int	write();

/* sgi system calls */
int	getversion();
int	phys();
int	tracesys();
int	sginap();
int	oldlstat();
int	lstat();
int	ftruncate();
int	gethostname();
int	readlink();
int	reboot();
int	sethostname();
int	symlink();
int	truncate();
int	vadvise();

#ifdef	IP2
extern	int	sgi_ip2();
#else
#define	sgi_ip2		nosys
#endif

#ifndef	KOPT_NOGL
int	grsys();
#else
#define	grsys	nosys
#endif

#ifdef	PROF
int	kprof(), kprofgather();
#else
#define	kprof		nosys
#define	kprofgather	nosys
#endif

/* nfs */
#if NNFS > 0
#include "../nfs/nfs_export.h"
#else
#define	nfs_mount	nosys
#define	nfs_svc		nosys
#define	nfs_getfh	nosys
#define	async_daemon	nosys
#define	exportfs	nosys
#endif

/* streams */
#if NUSTRM > 0
int	getstrmsg();
int	putstrmsg();
int	poll();
#else
#define getstrmsg nosys
#define putstrmsg nosys
#define poll nosys
#endif

/* 5.3 filesystem switch calls */
int	getdents();
int	fstatfs();
int	statfs();
int	sysfs();
int	rename();

/* 4.2 system TCP calls */
#if NTCP > 0
int	accept(), bind(), connect(), getdtablesize(), gethostid(),
	getpagesize(), getpeername(), getsockname(), getsockopt(),
	listen(), recv(), recvfrom(), recvmsg(), select(),
	send(), sendto(), sendmsg(), setsockopt(), sethosid(),
	shutdown(), socket(), setsockopt(), sethostid(), shutdown(),
	socket(), socketpair();
#else
int	notcp();
#define accept notcp
#define bind notcp
#define connect notcp
#define getdtablesize notcp
#define gethostid notcp
#define getpagesize notcp
#define getpeername notcp
#define getsockname notcp
#define getsockopt notcp
#define listen notcp
#define recv notcp
#define recvfrom notcp
#define recvmsg notcp
#define select notcp
#define send notcp
#define sendto notcp
#define sendmsg notcp
#define setsockopt notcp
#define sethosid notcp
#define shutdown notcp
#define socket notcp
#define setsockopt notcp
#define sethostid notcp
#define shutdown notcp
#define socket notcp
#define socketpair notcp
#endif
int	adjtime(), bsdgettime();

/* Sun system calls */
int	getdomainname();
int	setdomainname();

/* sgigsc support */
#if NSGIGSC > 0
int	sgigsc();
#else
#define	sgigsc	nosys
#endif

struct sysent sysent[] = {
	nosys,			/*  0 = indir */
	rexit,			/*  1 = exit */
	fork,			/*  2 = fork */
	read,			/*  3 = read */
	write,			/*  4 = write */
	open,			/*  5 = open */
	close,			/*  6 = close */
	wait,			/*  7 = wait */
	creat,			/*  8 = creat */
	link,			/*  9 = link */
	unlink,			/* 10 = unlink */
	exec,			/* 11 = exec */
	chdir,			/* 12 = chdir */
	gtime,			/* 13 = time */
	mknod,			/* 14 = mknod */
	chmod,			/* 15 = chmod */
	chown,			/* 16 = chown; now 3 args */
	sbreak,			/* 17 = break */
	oldstat,		/* 18 = oldstat */
	seek,			/* 19 = seek */
	getpid,			/* 20 = getpid */
	smount,			/* 21 = mount */
	sumount,		/* 22 = umount */
	setuid,			/* 23 = setuid */
	getuid,			/* 24 = getuid */
	stime,			/* 25 = stime */
	ptrace,			/* 26 = ptrace */
	alarm,			/* 27 = alarm */
	oldfstat,		/* 28 = oldfstat */
	pause,			/* 29 = pause */
	utime,			/* 30 = utime */
	truncate,		/* 31 = truncate a file by name */
	ftruncate,		/* 32 = truncate a file by file descriptor */
	saccess,		/* 33 = access */
	nice,			/* 34 = nice */
	fstat,			/* 35 = fstat (new, long inumber version) */
	sync,			/* 36 = sync */
	kill,			/* 37 = kill */
	lstat,			/* 38 = lstat (new, long inumber version) */
	setpgrp,		/* 39 = setpgrp */
	stat,			/* 40 = stat (new, long inumber version) */
	dup,			/* 41 = dup */
	pipe,			/* 42 = pipe */
	times,			/* 43 = times */
	profil,			/* 44 = prof */
	lock,			/* 45 = proc lock */
	setgid,			/* 46 = setgid */
	getgid,			/* 47 = getgid */
	ssig,			/* 48 = sig */
	msgsys,			/* 49 = IPC Messages */
	sgigsc,			/* 50 = sgigsc */
	sysacct,		/* 51 = turn acct off/on */
	shmsys,			/* 52 = IPC Shared Memory */
	semsys,			/* 53 = IPC Semaphores */
	ioctl,			/* 54 = ioctl */
	phys,			/* 55 = phys */
	nosys,			/* 56 = NOT USED: was unisoft file locking */
	utssys,			/* 57 = utssys */
	nosys,			/* 58 = NOT USED */
	exece,			/* 59 = exece */
	umask,			/* 60 = umask */
	chroot,			/* 61 = chroot */
	fcntl,			/* 62 = fcntl */
	ulimit,			/* 63 = ulimit */

	gethostname,		/* 64 = get host name */
	sethostname,		/* 65 = set host name */
	getversion,		/* 66 = get system version info */
	grsys,			/* 67 = graphics control stuff */
	reboot,			/* 68 = reboot system */
	tracesys,		/* 69 = trace control */
	sginap,			/* 70 = sginap */
	sgi_ip2,		/* 71 = sgi_ip2 */
	kprof,			/* 72 = kernel profiling enable/disable */
	kprofgather,		/* 73 = kernel profiling data gather */
	vadvise,		/* 74 = 4.2 vadvise */
	nosys,			/* 75 = NOT USED */
	nosys,			/* 76 = NOT USED */
	nosys,			/* 77 = NOT USED */
	nosys,			/* 78 = NOT USED */
	nosys,			/* 79 = NOT USED */
	symlink,		/* 80 = make symbolic link */
	exportfs,		/* 81 = export served filesystem */
	readlink,		/* 82 = read symbolic link */
	oldlstat,		/* 83 = old stat symbolic link */

	getstrmsg,		/* 84 = get stream message */
	putstrmsg,		/* 85 = send stream message */
	poll,			/* 86 = poll for stream messages */
	rmdir,			/* 87 = remove directory */
	mkdir,			/* 88 = make directory */
	getdents,		/* 89 = get directory entries */
	statfs,			/* 90 = get filesystem status */
	fstatfs,		/* 91 = get filesystem status */
	sysfs,			/* 92 = filesystem inquiries */
	rename,			/* 93 = rename file */
	nfs_mount,		/* 94 = mount network filesystem */
	nfs_svc,		/* 95 = nfs server */
	nfs_getfh,		/* 96 = get file handle */
	async_daemon,		/* 97 = nfs bio daemon */
	getdomainname,		/* 98 = get domain name */
	setdomainname,		/* 99 = get domain name */

/* 4.3 system calls */
	accept,			/* 100 = 4.2 accept */
	bind,			/* 101 = 4.2 bind */
	connect,		/* 102 = 4.2 connect */
	getdtablesize,		/* 103 = 4.2 getdtablesize */
	gethostid,		/* 104 = 4.2 gethostid */
	getpagesize,		/* 105 = 4.2 getpagesize */
	getpeername,		/* 106 = 4.2 getpeername */
	getsockname,		/* 107 = 4.2 getsockname */
	getsockopt,		/* 108 = 4.2 getsockopt */
	listen,			/* 109 = 4.2 listen */
	nosys,			/* 110 = NOT USED */
	recv,			/* 111 = 4.2 recv */
	recvfrom,		/* 112 = 4.2 recvfrom */
	recvmsg,		/* 113 = 4.2 recvmsg */
	select,			/* 114 = 4.2 select */
	send,			/* 115 = 4.2 send */
	sendmsg,		/* 116 = 4.2 sendmsg */
	sendto,			/* 117 = 4.2 sendto */
	sethostid,		/* 118 = 4.2 sethostid */
	setsockopt,		/* 119 = 4.2 setsockopt */
	shutdown,		/* 120 = 4.2 shutdown */
	socket,			/* 121 = 4.2 socket */
	socketpair,		/* 122 = 4.2 socketpair */
	adjtime,		/* 123 = 4.3 adjtime */
	bsdgettime,		/* 124 = 4.3 bsdgettime */
};
short	nsyscalls = sizeof(sysent) / sizeof(struct sysent);

/* XXX */
/*
 * nonexistent system call-- signal bad system call.
 */
nosys()
{
	psignal(u.u_procp, SIGSYS);
}

/*
 * Ignored system call
 */
nullsys()
{
}

#if NTCP <= 0
/*
 * stub when we have none of the 4.3 TCP system calls
 */
notcp()
{
	u.u_error = ENOPROTOOPT;
	return -1;
}
#endif
