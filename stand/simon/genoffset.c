/*
* $Source: /d2/3.7/src/stand/simon/RCS/genoffset.c,v $
* $Revision: 1.1 $
* $Date: 89/03/27 17:20:44 $
*/
#include "sys/types.h"
#include "common.h"

main()
{
	register struct commstruct *com = (struct commstruct *)0;

	printf("COMM_SZ	= 0x%x\n",&com->c_argbuf[256]);
	printf("ENTRY_OFF	= 0x%x\n",&com->c_entrypt);
	printf("GOSTK_OFF	= 0x%x\n",&com->c_gostk);
	printf("ARGC_OFF	= 0x%x\n",&com->c_argc);
	printf("ARGV_OFF	= 0x%x\n",&com->c_argv[0]);

	exit(0);
}
