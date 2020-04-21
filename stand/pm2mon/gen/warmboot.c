
/*
 *	warmboot.c - perform a prom warm boot.
 *
 */
warmboot()
{
	spl7();
	asm("	trap	#14");

}


error_restart()
{
	/* use this when the trap vectors cannot be trusted.
	   the equivalent of pushin' d' button! */

	spl7();
	asm("	.globl	start");
	asm("	jmp	start");
}
