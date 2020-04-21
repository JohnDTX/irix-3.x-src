/* expectint.c
 *
 *	'x' command routine expectint()
 */

extern char line[];	/* command line buffer */
extern short ix;	/* command line index */
extern char intcmd;
extern short expecting_interrupt;
extern char cmd,which,how;

expectint()
{
    short num;

    switch (which=line[ix++]) {
	case 's':
		intcmd = 's';
		break;
	default:
		illcmd();
		return(0);
	}
    num = getnum();
    expecting_interrupt += (num==0) ? 1 : num;
}
