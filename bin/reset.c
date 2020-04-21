char _Origin_[] = "UC Berkeley";

static	char *Sccsid = "@(#)reset.c	4.3 (Berkeley) 12/3/80";
/*
 * reset - restore tty to sensible state after crapping out in raw mode.
 */
#include <termio.h>
#include <sys/ioctl.h>

/*
** The following defines taken from stty.c with minor changes:
**	CFLAG:	Does not enable PARENB and turns on CS8 rather than CS7.
**	LFLAG:	Does not reset any of the synchronous modes
*/
#define	CFLAG_SET 	(CS8|CREAD)
#define	CFLAG_RESET	(CSIZE|PARENB|CLOCAL)
#define	IFLAG_SET	(BRKINT|IGNPAR|ISTRIP|ICRNL|IXON)
#define	IFLAG_RESET	(IGNBRK|PARMRK|INPCK|INLCR|IGNCR|IUCLC|IXOFF)
#define	LFLAG_SET	(ISIG|ICANON|ECHO|ECHOK|ECHOE)
#define	LFLAG_RESET	(XCASE|ECHONL|NOFLSH /* |STFLUSH|STWRAP|STAPPL */)
#define	OFLAG_SET	(OPOST|ONLCR)
#define	OFLAG_RESET	(OLCUC|OCRNL|ONOCR|ONLRET|OFILL|OFDEL| \
			NLDLY|CRDLY|TABDLY|BSDLY|VTDLY|FFDLY)


main()
{
	struct termio buf;

	ioctl(2, TCGETA, &buf);

	buf.c_cflag &= ~CFLAG_RESET;
	buf.c_cflag |= CFLAG_SET;

	buf.c_iflag &= ~IFLAG_RESET;
	buf.c_iflag |= IFLAG_SET;;

	buf.c_lflag &= ~LFLAG_RESET;
	buf.c_lflag |= LFLAG_SET;

	buf.c_oflag &= ~OFLAG_RESET;
	buf.c_oflag |= OFLAG_SET;

	buf.c_cc[VERASE] = CERASE;
	buf.c_cc[VKILL] = CKILL;
	buf.c_cc[VQUIT] = CQUIT;
	buf.c_cc[VINTR] = CINTR;
	buf.c_cc[VEOF] = CEOF;
	buf.c_cc[VEOL] = CNUL;

	ioctl(2, TCSETA, &buf);
	execlp("tset", "tset", "-Q", "-I", 0);	/* fix term dependent stuff */
	exit(1);
}
