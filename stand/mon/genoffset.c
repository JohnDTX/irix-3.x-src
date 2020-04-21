/*
* $Source: /d2/3.7/src/stand/mon/RCS/genoffset.c,v $
* $Revision: 1.1 $
* $Date: 89/03/27 17:15:32 $
*/
#include "sys/types.h"
#include "common.h"

main()
{
	register struct commstruct *com = (struct commstruct *)0;

	printf("COMM_SZ	= 0x%x\n", sizeof (struct commstruct));
	printf("ENTRY_OFF	= 0x%x\n",&com->c_entrypt);
	printf("GOSTK_OFF	= 0x%x\n",&com->c_gostk);
	printf("ARGC_OFF	= 0x%x\n",&com->c_argc);
	printf("ARGV_OFF	= 0x%x\n",&com->c_argv[0]);

	exit(0);
}
