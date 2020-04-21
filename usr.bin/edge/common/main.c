/*
 * $Source: /d2/3.7/src/usr.bin/edge/common/RCS/main.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:45:56 $
 */

#include <signal.h>
#include <stdio.h>
#include <sys/pty_ioctl.h>
#include "sys/termio.h"
#include <errno.h>
#include "window.h"
#include <gl.h>
#include "tf.h"
#include "manage.h"
#include "dbxshm.h"

char	flag_shell;		/* shell to use instead of default */
int	flag_hold;		/* hold output after shell exits */
char	*flag_font;		/* use a particular font */
char	flag_position;		/* set position */
char	flag_signal;		/* send SIGWINCH on redraws */
char	**shell_argv;		/* arguments for user shell */
char	**global_argv;		/* global version of argv */
int	flag_debug;
int	flag_rows, flag_cols;	/* arguments selection rows && cols */
char	flag_script;		/* script input/output to a file */

short	charwidth, charheight, chardescender;
float	max_width, max_height;
int	llx, lly, urx, ury;
char	*fontlib = FONTLIB;
WINTTYMAP	*dbx_win;
WINTTYMAP	*user_win;
char	*dbx_argv[20];
char	flag_blink;
int	dbflag;
char	*user_name;


int	script_fd;

main(argc, argv)
int	argc;
char	*argv[];
{
	int	av = 0;
	int	i;
	int	key;
	char	asc_key[10];
	char	*corefile;
	FILE	*f;

	/*
	 * Get the arg's
	 */

	av = 5;
	if (ismex() == 0) {
		fprintf(stderr, "You must be running mex to use %s\n", argv[0]);
		myexit(-1);
	}
	dbx_argv[0] = "dbx";
	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			switch(argv[i][1]) {
			case 'd':
				dbflag++;
				break;
			case 'I':
			case 'c':
				dbx_argv[av++] = argv[i++];
				dbx_argv[av++] = argv[i];
				break;
			default:
				dbx_argv[av++] = argv[i];
			}
		} else if (user_name == (char *) 0) {
			user_name = argv[i];
		} else {
			corefile = argv[i];
		}
	}

	/*
	 * Make the dbx and user windows
	 */

try_again:
	if (user_name == (char *) 0)  {
		user_name = (char *) malloc(512);
		printf("enter object file name (hit return to quit): ");
		fflush(stdout);
		gets(user_name);
		if (user_name[0] == (char) 0) {
			myexit(1);
		}
	}
	if ((f = fopen(user_name, "r")) == (FILE *) 0) {
		printf("cannot open %s\n", user_name);
		user_name = (char *) malloc(512);
		free(user_name);
		user_name = (char *) 0;
		goto try_again;
	} else {
		fclose(f);
    }
	dbx_argv[av++] = user_name;
	init_comm();
/*
	if (flag_font == (char *) 0) {
		flag_font = getenv("FONT");
	}
*/
	make_windows(argv[0]);

	/*
	 * Initialize shared memory and it's sync. pty's and
	 * pass the names of the slaves to dbx.
	 */
	dbx_argv[1] = "-w";
	key = shm_init();
	sprintf(asc_key, "%d\n", key);
	dbx_argv[2] = syncptyin.sp_slavename;
	dbx_argv[3] = asc_key;

	/*
	 * Pass dbx the name of the user window slave pty, so that dbx
	 * can redirect the user output there.
	 * chown the slave pty to the user's uid.
	 */
	dbx_argv[4] = user_win->wt_slavename;
	if (chown(user_win->wt_slavename, getuid(), getgid()) == -1) {
		fprintf(stderr, "cannot chown %s\n", user_win->wt_slavename);
	}
	(void) chmod(user_win->wt_slavename, 0666);


	if (corefile != (char *) 0) {
		dbx_argv[av++] = corefile;
	}
	dbx_argv[av++] = NULL;
	/*
	 * Fire off dbx.
	 */
	makechild(dbx_win, dbx_argv);
	cur_win = dbx_win;

	/*
	 * Initialize dbx main menu.
	 */
	initmenus();

	/*
	 * Let's do it
	 */
#ifdef SPEED_BAR
	speed_bar_init();
#else
	current_delay = 6;
#endif
	winattach(dbx_win->wt_gid);
	winset(dbx_win->wt_gid);
	winattach(dbx_win->wt_gid);
	dbxtool();
}

make_windows(name) 
char	*name;
{

	WINTTYMAP	*open_wintty();
	struct	termio	termio;

	dbx_win = open_wintty(name, WT_DBX);
	user_win = open_wintty(user_name, WT_USER);
	/*
	 * Open the slave for the user window and leave it open so 
	 * that the when the user program dies I don't have to
	 * reopen the master.
	 */
	user_win->wt_slavefd = open(user_win->wt_slavename, 2);
	(void) ioctl(0, TCGETA, &termio);
	set_child_termio(&termio);
	termio.c_iflag &= ~(IGNBRK | PARMRK | INPCK | INLCR | IGNCR | IBLKMD);
	termio.c_iflag |= (BRKINT | IGNPAR | ISTRIP | ICRNL | IXON);
	termio.c_oflag &= ~(OCRNL | ONOCR);
	termio.c_oflag |= (OPOST | ONLCR);
	termio.c_lflag |= (ISIG | ICANON | ECHO);
	if (ioctl(user_win->wt_slavefd, TCSETA, &termio) < 0) {
		abort();
	}
}

