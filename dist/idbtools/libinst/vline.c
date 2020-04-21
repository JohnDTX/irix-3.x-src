vline (argc, argv, line)
	int		argc;
	char		*argv [];
	char		*line;
{
	int		f, n;

	if ((f = runpipe (argc, argv, 0)) < 0) return (-1);
	while ((n = read (f, line, 1)) == 1 && *line++ != '\n') ;
	*line = '\0';
	close (f);
	return (waitpipe (f) < 0 || n < 0 ? -1 : 0);
}
