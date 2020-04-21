#include <stdio.h>

char	*getwd (), *ctime (), *getenv (), *malloc ();

main (argc, argv, envp)
int	argc;
char	*argv [];
char	*envp [];
{
	char	buff [256], marker [256], *p, **nargv, *flags;
	int	i;
	long	clock;

	time (&clock);
	sprintf (marker, "@ %.15s\t%s\n", ctime (&clock) + 4, getwd (buff));
	for (p = marker; *p; ++p) putc (*p == ':' ? '.' : *p, stderr);
	fflush (stderr);
	execve ("/bin/make", argv, envp);
	perror ("/bin/make");
	fprintf (stderr, "make wrapper: Cannot exec /bin/make.\n");
	exit (1);
}
