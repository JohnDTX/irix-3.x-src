
#include <stdio.h>

main() {

	/* read in a microcode file, converting the ascii
	   microcode to binary */

	unsigned long junk,mcwd;
	int counter;

	FILE *ip,*op;

	if ((ip = fopen("ascii","r")) == NULL) {
		fprintf(stderr,"cant open input\n");
		exit (0);
	}
	if ((op = fopen("binary","w")) == NULL) {
		fprintf(stderr,"cant open output\n");
		exit (0);
	}

	counter = 0;
	while (!feof(ip)) {
		fscanf(ip,"%x %x",&junk,&mcwd);
		if (feof(ip)) break;
		fwrite(&mcwd,4,1,op);
		counter++;
	}

	mcwd = 0;
	printf("%d microcode words translated\n",counter );
	for (;counter < 0x1000 ;counter++) 
		fwrite(&mcwd,4,1,op);
	fclose(op);
	fclose(ip);
	printf("padded to %d.\n",counter );
	exit(0);
}
