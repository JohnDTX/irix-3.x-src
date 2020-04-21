char _Origin_[] = "System V";

/*	@(#)unlink.c	1.1	*/
main(argc, argv) char *argv[]; {
	if(argc!=2) {
		write(2, "Usage: /etc/unlink name\n", 24);
		exit(1);
	}
	unlink(argv[1]);
	exit(0);
}

exit(arg)
{
	_exit(arg);
}
