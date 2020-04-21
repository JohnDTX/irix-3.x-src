/*
 * Clone a shell
 */
#include "gsh.h"
#include "window.h"

/*
 * Clone the current gsh, making as near a duplicate that we can.
 * XXX recode to use a pointer instead of array indexing
 */
void
clone()
{
	register int av;
	register char **argv;
	int pid;
	char *newargv[500];
	char rowbuf[20], colbuf[20];
	char blinkbuf[20];
	char c1buf[20], c2buf[20], c3buf[20];
	int xsize, ysize, xorg, yorg;

	/*
	 * We get this information before we fork, because the child
	 * process won't have graphics capability.
	 */
	getsize(&xsize, &ysize);
	xsize--;
	ysize--;
	getorigin(&xorg, &yorg);

	pid = fork();
	switch (pid) {
	  case 0:			/* child */
		/*
		 * Construct newargv vector from command line flags.
		 */
		av = 0;
		newargv[av++] = "gsh";
		if (flag_font) {
			newargv[av++] = "-f";
			newargv[av++] = flag_font;
		}
		if (flag_hold)
			newargv[av++] = "-h";
		if (flag_blink) {
			newargv[av++] = "-b";
			sprintf(blinkbuf, "%d", flag_blink);
			newargv[av++] = blinkbuf;
		}
		if (flag_debug)
			newargv[av++] = "-d";
		if (force_title) {
			newargv[av++] = "-t";
			newargv[av++] = title_value;
		}
		if (force_colors) {
			sprintf(c1buf, "%d", c1);
			sprintf(c2buf, "%d", c2);
			sprintf(c3buf, "%d", c3);
			newargv[av++] = "-C";
			newargv[av++] = c1buf;
			newargv[av++] = c2buf;
			newargv[av++] = c3buf;
		}
		if (flag_script)
			newargv[av++] = "-S";
#ifdef	SHRINK
		if (flag_icon) {
			newargv[av++] = "-i";
			newargv[av++] = icon_name;
		}
		if (flag_movie)
			newargv[av++] = "-m";
#endif

		sprintf(rowbuf, "%d", txport[0].tx_rows);
		sprintf(colbuf, "%d", txport[0].tx_cols);
		newargv[av++] = "-s";
		newargv[av++] = rowbuf;
		newargv[av++] = colbuf;
		if (flag_shell) {
			newargv[av++] = "-c";
			argv = shell_argv;
			while (*argv)
				newargv[av++] = *argv++;
		}
		newargv[av] = 0;
		execvp(newargv[0], newargv);
		/* FALLTHROUGH */
	  case -1:			/* no more processes */
		/*
		 * If we fail for any reason, beep the bell so user knows
		 * that SOMETHING went worng
		 */
		ringbell();
		break;
	  default:			/* parent */
		break;
	}
}
