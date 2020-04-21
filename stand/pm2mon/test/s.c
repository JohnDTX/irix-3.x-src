main()
{
    int rgc; char **rgv;

    for( ;; )
    {
	readargs(&rgc,&rgv);
	while( --rgc >= 0 )
	    printf(" %s",*rgv++);
	printf("\n");
    }
}

char *MBMemVA;
unsigned short switches;
char *MBioVA;
long MBMemSize;
char *MBMemArea;
godebugger() {}
