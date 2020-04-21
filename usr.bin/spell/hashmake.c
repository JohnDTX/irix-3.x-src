char _Version_[] = "(C) Copyright 1983 UniSoft Corp. Version V.1.0";
char _Origin_[] = "System V";
/*	@(#)hashmake.c	1.1	*/
#include "hash.h"

main()
{
	char word[30];
	long h;
	hashinit();
	while(gets(word)) {
		printf("%.*lo\n",(HASHWIDTH+2)/3,hash(word));
	}
	return(0);
}
