char _Origin_[] = "System V";

/*	@(#)link.c	1.1	*/
main(argc, argv) char *argv[]; {
	if(argc!=3) {
		write(2, "Usage: /etc/link from to\n", 25);
		exit(1);
	}
	exit((link(argv[1], argv[2])==0)? 0: 2);
}

exit(arg)
{
	_exit(arg);
}
