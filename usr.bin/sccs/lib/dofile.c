# include	"../hdr/defines.h"
# include	"dirent.h"

SCCSID(@(#)dofile.c	5.2);

int	nfiles;
char	had_dir;
char	had_standinp;


do_file(p,func)
register char *p;
int (*func)();
{
	extern char *Ffile;
	char str[FILESIZE];
	char ibuf[FILESIZE];
	DIR *iop;
	struct dirent *dirp;

	if (p[0] == '-') {
		had_standinp = 1;
		while (gets(ibuf) != NULL) {
			if (sccsfile(ibuf)) {
				Ffile = ibuf;
				(*func)(ibuf);
				nfiles++;
			}
		}
	}
	else if (exists(p) && (Statbuf.st_mode & S_IFMT) == S_IFDIR) {
		had_dir = 1;
		Ffile = p;
		if((iop = opendir(p)) == NULL)
			return;
		while((dirp = readdir(iop)) != NULL) {
			if(dirp->d_ino == 0) continue;
			if(!strcmp(dirp->d_name, ".")) continue;
			if(!strcmp(dirp->d_name, "..")) continue;
			sprintf(str,"%s/%s",p,dirp->d_name);
			if(sccsfile(str)) {
				Ffile = str;
				(*func)(str);
				nfiles++;
			}
		}
		closedir(iop);
	}
	else {
		Ffile = p;
		(*func)(p);
		nfiles++;
	}
}
