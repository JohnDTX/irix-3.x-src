int	tick(), tock();

main()
{
	int i;

	for (i = 0; i < 10; i++)
		lwstart("tock", tock, 0, i, 0, 0, 0);
	tick();
}

tick()
{
	for (;;) {
		sleep(1);
		printf("\nTick ");
		lwyield();
	}
}

tock(tocknum)
{
	for (;;) {
		printf(" Tock %d", tocknum);
		lwyield();
	}
}
