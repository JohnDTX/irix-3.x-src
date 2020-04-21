/*
 * $Source: /d2/3.7/src/usr.bin/edge/common/RCS/scanner.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:46:01 $
 */
#include <stdio.h>
#include <pwd.h>

#define INITFILE ".edgerc"

typedef	struct {
	char	*comm_string;
	int	(*comm_func)();
} COMMANDS;

extern	addbutton();
#ifdef notdef
extern	setfont();
#endif
extern	setsize();
extern	setorigin();
extern	setdcolor();

COMMANDS	commands[] = {
	"button", addbutton,
#ifdef notdef
	"font", setfont,
#endif
	"size", setsize,
	"origin", setorigin,
	"color", setdcolor,
	(char *) 0,  0
};


FILE	*comm_file;
char	filename[400];
char	linebuff[4096];


init_comm()
{
	FILE	*fp;
	FILE	*open_comm();

	if ((fp = open_comm()) != (FILE *) 0) {
		read_comm(fp);
	}
}

FILE	*
open_comm()
{

	struct	passwd	*pwent;
	struct	passwd	*getpwnam();
	FILE	*init_fp;

	
	if ((init_fp = fopen(INITFILE, "r")) == NULL) {
		if ((pwent = getpwnam(cuserid((char *) 0))) == NULL) {
			return(NULL);
		}
		strcpy(filename, pwent->pw_dir);
		strcat(filename, "/");
		strcat(filename, INITFILE);
		if ((init_fp = fopen(filename, "r")) == NULL) {
			return(NULL);
		}
	}
	return(init_fp);
}


read_comm(fp)
FILE	*fp;
{
	COMMANDS	*commp;
	char	*linep;
	char	*strtok();

	while (fgets(linebuff, 4095, fp) != NULL) {
		commp = commands;
		linep = linebuff;
		linep = strtok(linep, " \t");
		while (commp->comm_string != (char *) 0) {
			if (strncmp(linep, commp->comm_string, 
				strlen(commp->comm_string)) == 0) {
				(*commp->comm_func)
					(linep + strlen(commp->comm_string) + 1);
				break;
			}
			commp++;
		}
	}
}

	
