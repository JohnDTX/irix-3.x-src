char _Version_[] = "(C) Copyright 1983 UniSoft Corp. Version V.1.0";
char _Origin_[] = "System V";
/*	@(#)fwtmp.c	1.2 of 3/31/82	*/
# include <stdio.h>
# include <sys/types.h>
# include <utmp.h>
# include "acctdef.h"

struct	utmp	Ut;

main ( c, v, e )
char	**v, **e;
int	c;
{

	int	iflg,cflg;

	iflg = cflg = 0;

	while ( --c > 0 ){
		if(**++v == '-') while(*++*v) switch(**v){
		case 'c':
			cflg++;
			continue;
		case 'i':
			iflg++;
			continue;
		}
		break;
	}

	for(;;){
		if(iflg){
			if(inp(stdin,&Ut) == EOF)
				break;
		} else {
			if(fread(&Ut,sizeof Ut, 1, stdin) != 1)
				break;
		}
		if(cflg)
			fwrite(&Ut,sizeof Ut, 1, stdout);
		else
		  printf("%-8.8s %-4.4s %-12.12s %5hd %2hd %4.4ho %4.4ho %lu %s",
				Ut.ut_name,
				Ut.ut_id,
				Ut.ut_line,
				Ut.ut_pid,
				Ut.ut_type,
				Ut.ut_exit.e_termination,
				Ut.ut_exit.e_exit,
				Ut.ut_time,
				ctime(&Ut.ut_time)+4);
	}
	exit ( 0 );
}
inp(file, u)
FILE *file;
register struct utmp *u;
{

	char	buf[BUFSIZ];
	register char *p;
	register i;

	if(fgets((p = buf), BUFSIZ, file)==NULL)
		return EOF;

	for(i=0; i<NSZ; i++)	/* Allow a space in name field */
		u->ut_name[i] = *p++;
	for(i=NSZ-1; i >= 0; i--) {
		if(u->ut_name[i] == ' ')
			u->ut_name[i] = '\0';
		else
			break;
	}
	p++;

	for(i=0; i<4; i++)
		if((u->ut_id[i] = *p++)==' ')
			u->ut_id[i] = '\0';
	p++;

	for(i=0; i<LSZ; i++)	/* Allow a space in line field */
		u->ut_line[i] = *p++;
	for(i=LSZ-1; i >= 0; i--) {
		if(u->ut_line[i] == ' ')
			u->ut_line[i] = '\0';
		else
			break;
	}

	sscanf(p, "%hd %hd %ho %ho %ld",
		&u->ut_pid,
		&u->ut_type,
		&u->ut_exit.e_termination,
		&u->ut_exit.e_exit,
		&u->ut_time);

	return((unsigned)u);
}
