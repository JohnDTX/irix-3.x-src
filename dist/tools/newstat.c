#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

int		failed;

main ()
{
	struct stat	st;
	int		caught ();

	signal (SIGSYS, caught);
	failed = 0;
	stat ("/", &st);
	exit (failed);
}

caught (sig)
	int		sig;
{
	if (sig == SIGSYS) failed = 1;
}
