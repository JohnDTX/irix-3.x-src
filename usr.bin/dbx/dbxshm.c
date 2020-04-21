#ifdef DBMEX
#include "defs.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <fcntl.h>
#include "tree.h"
#include "main.h"
#include "source.h"

#define	DBX_SHMKEY	101
#define	MIDDLE	0
#define TOP	1

typedef	struct	{
	int	lineno;
	int	value;
	char	sbuf[1000];
	char	filename[512];
} DBXSHM;

public	DBXSHM	*shmptr;
public	int	shm_id;
public	int	syncfd;
int	pipe_des;
String	cur_edge_src;

shm_write(type, string, value, lineno, buffer)
char	type;
String	string;
int	value;
int	lineno;
char	*buffer;
{
	char	*p;
	char	*s;
	int	i;
	char	*findsource();

	switch(type) {
	case 'L':
	case 'B':
	case 'S':
		shmptr->lineno = lineno;
		shmptr->value = value;
		s = findsource(string);
		if (s == nil) {
			return;
		} else {
			if (strncmp(s, "./", 2) == 0) {
				s += 2;
			}
			strcpy(shmptr->filename, s);
		}
		break;
	case 'f':
		if (cur_edge_src == string) {
			return;
		}
		s = findsource(string);
		if (s == nil) {
			return;
		} else {
			if (strncmp(s, "./", 2) == 0) {
				s += 2;
			}
			strcpy(shmptr->filename, s);
		}
		cur_edge_src = string;
		break;
	case 'D':
		shmptr->value = value;
		break;
	case 'F':
		bcopy(string, shmptr->filename, lineno);
		shmptr->value = value;
		shmptr->sbuf[0] = buffer[0];
		break;
	case 'U':
	case 'e':
		strcpy(shmptr->filename, string);
		shmptr->lineno = lineno;
		break;
	case 'V':
		bcopy(buffer, shmptr->sbuf, value);
		shmptr->sbuf[value] = '\0';
		/*
		strcpy(shmptr->filename, string);
		*/
		break;
		
	default:
		fatal("Illegal type in write_shm\n");
		break;
	}
	if (write(pipe_des, shmptr, sizeof(DBXSHM)) != sizeof(DBXSHM)) {
		perror("dbx:writeing pipe");
		fatal("cannot write dbx -> edge pipe\n");
	}
	if (write(syncfd, &type, 1) != 1) {
			fatal("1cannot write to synchronization pty\n");
		}

tryagain:
		while ((i = read(syncfd,
			 (char *) &i, 1)) == 0) {
		}
		if (i < 0) {	
			if (errno == EINTR) {
				goto tryagain;
			}
			perror("dbx:");
			fatal("1cannot read synchronization pty\n");
		}
}
		


/*
 * This function needs work in getting a good location
 * for the shm segment
 */
shm_init(ptyname, ascii_pipedes) 
char	*ptyname;
char	*ascii_pipedes;
{

	register struct termio termio;
	/*
	 * attach to shared mem. created by dbmex
	 */
	pipe_des = atoi(ascii_pipedes);
	shmptr = (DBXSHM *) malloc(sizeof(DBXSHM));
		
	/*
	 * Open synchronization pty.
	 */
	if ((syncfd = open(ptyname, O_RDWR)) == -1 ) {
		fatal("cannot open synchronization pty\n");
	}
	if (ioctl(syncfd, TCGETA, &termio) < 0) {
		abort();
	}
	if (ioctl(syncfd, TCGETA, &termio) == -1) {
		abort();
	}
	termio.c_lflag = 0;
	termio.c_cc[VEOL] = 1;
	termio.c_cc[VEOF] = 1;
	if (ioctl(syncfd, TCSETA, &termio) < 0) {
		abort();
	}
}

shm_detatch() {
}
#endif
