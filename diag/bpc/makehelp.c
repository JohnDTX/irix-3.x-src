#include <stdio.h>

#define LINELENGTH	100

main () {
    char line[LINELENGTH+2];
    int first = 1;
    printf ("/* I am automatically generated - do not edit me !!! */\n\n");
    printf ("#include \"getcmnd.h\"\n\n");
    printf ("#include \"commands.h\"\n\n");
    printf ("Help helplist[] = {\n");
    while (fgets (line, LINELENGTH, stdin)) {
	if (line[0] == 'C' && line[1] == '_') {
	    char *s;
	    if (first == 0)
		printf ("\"},\n");
	    printf ("    {");
	    for (s=line; *s != '\n'; s++)
		putchar (*s);
	    printf (",\"");
	    first = 0;
	    }
	else if (line[0] == '#') {
	    /* do nothing - comment */
	    }
	else {
	    output (line);
	    }
	}
    printf ("\"},\n    {C_NOTACOMMAND,\"\"}\n    };\n");
    }

output (s)
char *s;
{
    for (;*s; s++) {
	switch (*s) {
	    case '\n':	printf ("\\n"); break;
	    case 042:	break;
	    default:	putchar (*s); break;
	    }
	}
    }
