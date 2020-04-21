/*
 * $Source: /d2/3.7/src/usr.bin/edge/common/RCS/shm.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:46:03 $
 */
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/pty_ioctl.h>
#include <stdio.h>
#include <errno.h>
#include <termio.h>
#include <fcntl.h>
#include "dbxshm.h"
#include "window.h"
#include "tf.h"
#include "manage.h"

char	*use_string[100];
int	nuses;

DBXSHM	*shmptr;
int	shm_id;
int	noread;
int	cur_line;
char	*cur_file;
SYNCPTY	syncptyin;
#ifndef mips
int	commpipe[2];
#endif
char	*malloc();

shm_init()
{

	key_t	shm_key = DBX_SHMKEY;
	register struct termio term;
	int	status;
	
#ifdef mips
	/*
	 * Get shared memory
	 */
try_again:
	if ((shm_id = shmget(shm_key, sizeof(DBXSHM), 
		IPC_EXCL|IPC_CREAT|0777)) == -1) {
		if (errno == EEXIST) {
			shm_key++;
			goto try_again;
		}
		perror("dbmex:shmget");
		exit (-1);
	}
	if ((shmptr = (DBXSHM *)shmat(shm_id, 0x1f000000, 0)) == 
		(DBXSHM *) -1) {
		perror("edge:shmat");
	}
#else
	if (pipe(commpipe) != 0) {
		perror("edge:cannot open pipe");
		exit(-1);
	}
	shmptr = (DBXSHM *) malloc(sizeof(DBXSHM));
#endif

	/*
	 * open psuedo tty for syncronization
	 * syncptyin for dbx to notify dbmex taht something is ready in shm
         * and for telling dbx we have recieved and taken the data
	 */
	syncptyin.sp_ptynum = open_pty(&(syncptyin.sp_slavename),
		&(syncptyin.sp_mastername),
		&(syncptyin.sp_masterfd), 0);
	
	if (ioctl(syncptyin.sp_masterfd, TCGETA, &term) < 0) {
		abort();
	}
	term.c_cc[VEOL] = 1;
	term.c_cc[VEOF] = 1;
	term.c_lflag = 0;
	term.c_iflag = 0;
	term.c_oflag = 0;
	term.c_cflag = B9600|CS8;
	if (ioctl(syncptyin.sp_masterfd, TCSETA, &term) < 0) {
		abort();
	}
	if (ioctl(syncptyin.sp_masterfd, PTIOC_QUEUE, 0) < 0) {
		fprintf(stderr, "cannot set sync. pty to PTIOC_QUEUE\n");
		perror("edge:");
		abort();
	}
	if (chown(syncptyin.sp_slavename, getuid(), getgid()) == -1) {
		fprintf(stderr, "cannot chown %s\n", syncptyin.sp_slavename);
	}
	(void) chmod(syncptyin.sp_slavename, 0666);
#ifdef mips
	return(shm_key);
#else
	return(commpipe[1]);
#endif
}

shmdetach() {

#ifdef mips
	shmdt(shmptr);
#endif
}


char	file[300];
int	first_src = 1;
int count;
process_shm() 
{
	char	buf[100];
	int	nb;
	int	i;

	nb = read(syncptyin.sp_masterfd, buf, sizeof(buf));
	(void) ioctl(syncptyin.sp_masterfd, PTIOC_QUEUE, 0);
	if (nb <= 0) {
		byebye();
	}
	for (i = 0; i < nb; i++) {
		int j;
#ifndef mips
		if (read(commpipe[0], shmptr, sizeof(DBXSHM)) != sizeof(DBXSHM)) {
			perror("edge:cannot read dbx->edge pipe");
			exit(-1);
		}
#endif
		switch (buf[i]) {
		/*
		 * List of source files from dbx
		 */
		case 'F':
			if (shmptr->sbuf[0] == 'M') {
				add_filetab(shmptr->filename, 
					shmptr->value, 0);
			} else if (shmptr->sbuf[0] == '1') {
				add_filetab(shmptr->filename, 
					shmptr->value, 1);
			} else {
				make_edit_menu();
			}
			strcpy(file, shmptr->filename);

			break;

		/*
		 * program stopped at shmptr->filename line
		 * shmptr->lineno
		 */
		case 'S':
			strcpy(file, shmptr->filename);
			display(shmptr->lineno - 1, file);
			j = shmptr->lineno;
			cur_file = file;
			cur_line = shmptr->lineno;
#ifdef SPEED_BAR
			if (shmptr->value == 1) {
				sginap(current_delay);
			}
#else
			sginap(current_delay);
#endif
			break;

		/*
		 * Breakpoint set at shmptr->filename 
		 * line shmptr->lineno
		 * shmptr->value contains bp number.
		 */
		case 'B':
			strcpy(file, shmptr->filename);
			add_bp(file, shmptr->lineno, shmptr->value);
			break;

		/*
		 * Delete bp number shmptr->value
		 */
		case 'D':
			del_bp(shmptr->value);
			break;

		/*
		 * Change current file to shmptr->filename
		 * and display it.
		 */
		case 'f':
			strcpy(file, shmptr->filename);
			cur_file = file;
			if (first_src) {
				init_srcwin(file);
				first_src = 0;
			} else {
				display(-1, file);
			}
			break;

		/*
		 * List shmptr->filename line shmptr->lineno
		 */
		case 'L':
			strcpy(file, shmptr->filename);
			cur_file = file;
			if (shmptr->value == TOP) {
				mk_topline(file, shmptr->lineno - 1);
			} else {
				mk_midline(file, shmptr->lineno - 1);
			}
			break;

		case 'e':
			strcpy(file, shmptr->filename);
			make_vi(file, shmptr->lineno);
			break;

		case 'V':
			if (var_win == -1) {
				init_varwin();
			}
			var_display(shmptr->sbuf);
			break;
			
		case 'U':
			use_string[nuses] = 
				malloc(strlen(shmptr->filename) + 1);
			strcpy(use_string[nuses++], shmptr->filename);
			break;

		default:
			fprintf(stderr, "edge: bad cookie from dbx: %x\n", buf[0]);
			goto skip_write;
		}

		/*
		 * Tell dbx to continue.
		 */
		if ((i=write(syncptyin.sp_masterfd, ".", 1)) == -1) {
			perror("dbmex");
			myexit(-1);
		}
skip_write:;
	}
}

shm_rm() {

	shmctl(shm_id, IPC_RMID, 0);
}
