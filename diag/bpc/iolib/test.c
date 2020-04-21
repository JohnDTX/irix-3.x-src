char oneline[100];

int myhandler();

main()
{
	register i;

	serialhandler(0,myhandler);
	printf("starting now\n");
	while(1) {
	    for(i=0; i<300000; i++)
		;
	    printf("enter some text now: ");
	    getline(oneline);
	    printf("thx. that line was %s\n",oneline);
	}
}

myhandler( onechar )
int onechar;
{
    printf("\nmyhandler: onechar is %c\n",onechar);
    if(onechar == 'r')
	restart();
}
