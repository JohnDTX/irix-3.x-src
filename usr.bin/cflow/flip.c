char _Version_[] = "(C) Copyright 1983 UniSoft Corp. Version V.1.0";
char _Origin_[] = "System V";

/*	@(#)flip.c	1.2	*/
#include <stdio.h>
#include <ctype.h>

main()
	{
	char line[BUFSIZ], *pl, *gets();

	while (pl = gets(line))
		{
		while (*pl != ':')
			++pl;
		*pl++ = '\0';
		while (isspace(*pl))
			++pl;
		printf("%s : %s\n", pl, line);
		}
	}
