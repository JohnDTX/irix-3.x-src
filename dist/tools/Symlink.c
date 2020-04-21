#include <sys/signal.h>

main ()
{
	int	exit ();

	signal (SIGSYS, exit);
	symlink ("/", 0);
	exit (0);
}
