/*	@(#)sh.init.c	2.1	SCCS id keyword	*/
/* $Source: /d2/3.7/src/bin/csh/RCS/sh.init.c,v $ */
static	char	*Sccsid = "@(#)$Revision: 1.1 $";
/* $Date: 89/03/27 14:53:25 $ */
/* Copyright (c) 1980 Regents of the University of California */
#include "sh.local.h"
/*
 * C shell
 */

extern	int await();
extern	int doalias();
extern	int dobreak();
extern	int dochngd();
extern	int docontin();
extern	int dodirs();
extern	int doecho();
extern	int doelse();
extern	int doend();
extern	int doendif();
extern	int doendsw();
extern	int doeval();
extern	int doexit();
extern	int doflip();
extern	int doforeach();
extern	int doglob();
extern	int dogoto();
extern	int dohash();
extern	int dohist();
extern	int doif();
extern	int dolet();
extern	int dologin();
extern	int dologout();
extern	int donewgrp();
extern	int donice();
extern	int donohup();
extern	int dopopd();
extern	int dopushd();
extern	int doonintr();
extern	int dorepeat();
extern	int doset();
extern	int dosetenv();
extern	int dosource();
extern	int doswbrk();
extern	int doswitch();
extern	int dotime();
#ifndef V6
extern	int doumask();
#endif
extern	int dowhile();
extern	int dozip();
extern	int execash();
#ifdef VFORK
extern	int hashstat();
#endif
extern	int goodbye();
extern	int shift();
extern	int showall();
extern	int unalias();
extern	int dounhash();
extern	int unset();
extern	int dounsetenv();

#define INF	1000

struct	biltins {
	char	*bname;
	int	(*bfunct)();
	short	minargs, maxargs;
} bfunc[] = {
	"%",		doflip,		0,	1,
	"@",		dolet,		0,	INF,
	"alias",	doalias,	0,	INF,
#ifdef debug
	"alloc",	showall,	0,	1,
#endif
	"break",	dobreak,	0,	0,
	"breaksw",	doswbrk,	0,	0,
	"case",		dozip,		0,	1,
	"cd",		dochngd,	0,	1,
	"chdir",	dochngd,	0,	1,
	"continue",	docontin,	0,	0,
	"default",	dozip,		0,	0,
	"dirs",		dodirs,		0,	1,
	"echo",		doecho,		0,	INF,
	"else",		doelse,		0,	INF,
	"end",		doend,		0,	0,
	"endif",	dozip,		0,	0,
	"endsw",	dozip,		0,	0,
	"eval",		doeval,		0,	INF,
	"exec",		execash,	1,	INF,
	"exit",		doexit,		0,	INF,
	"foreach",	doforeach,	3,	INF,
#ifdef	IIASA
	"gd",		dopushd,	0,	1,
#endif
	"glob",		doglob,		0,	INF,
	"goto",		dogoto,		1,	1,
#ifdef VFORK
	"hashstat",	hashstat,	0,	0,
#endif
	"history",	dohist,		0,	2,
	"if",		doif,		1,	INF,
	"login",	dologin,	0,	1,
	"logout",	dologout,	0,	0,
	"newgrp",	donewgrp,	1,	1,
	"nice",		donice,		0,	INF,
	"nohup",	donohup,	0,	INF,
	"onintr",	doonintr,	0,	2,
	"popd",		dopopd,		0,	1,
	"pushd",	dopushd,	0,	1,
#ifdef	IIASA
	"rd",		dopopd,		0,	1,
#endif
	"rehash",	dohash,		0,	0,
	"repeat",	dorepeat,	2,	INF,
	"set",		doset,		0,	INF,
#ifndef V6
	"setenv",	dosetenv,	2,	2,
#endif
	"shift",	shift,		0,	1,
	"source",	dosource,	1,	2,
	"switch",	doswitch,	1,	INF,
	"time",		dotime,		0,	INF,
#ifndef V6
	"umask",	doumask,	0,	1,
#endif
	"unalias",	unalias,	1,	INF,
	"unhash",	dounhash,	0,	0,
	"unset",	unset,		1,	INF,
	"unsetenv",	dounsetenv,	1,	INF,
	"wait",		await,		0,	0,
	"while",	dowhile,	1,	INF,
	0,		0,		0,	0,
};

#define	ZBREAK		0
#define	ZBRKSW		1
#define	ZCASE		2
#define	ZDEFAULT 	3
#define	ZELSE		4
#define	ZEND		5
#define	ZENDIF		6
#define	ZENDSW		7
#define	ZEXIT		8
#define	ZFOREACH	9
#define	ZGOTO		10
#define	ZIF		11
#define	ZLABEL		12
#define	ZLET		13
#define	ZSET		14
#define	ZSWITCH		15
#define	ZTEST		16
#define	ZTHEN		17
#define	ZWHILE		18

struct srch {
	char	*s_name;
	short	s_value;
} srchn[] = {
	"@",		ZLET,
	"break",	ZBREAK,
	"breaksw",	ZBRKSW,
	"case",		ZCASE,
	"default", 	ZDEFAULT,
	"else",		ZELSE,
	"end",		ZEND,
	"endif",	ZENDIF,
	"endsw",	ZENDSW,
	"exit",		ZEXIT,
	"foreach", 	ZFOREACH,
	"goto",		ZGOTO,
	"if",		ZIF,
	"label",	ZLABEL,
	"set",		ZSET,
	"switch",	ZSWITCH,
	"while",	ZWHILE,
	0,		0,
};

char	*mesg[] = {
	0,
	"Hangup",
	0,
	"Quit",
	"Illegal instruction",
	"Trace/BPT trap",
	"IOT trap",
	"EMT trap",
	"Floating/integer exception",
	"Killed",
	"Bus error",
	"Segmentation violation",
	"Bad system call",
	0,
	"Alarm clock",
	"Terminated",
};
