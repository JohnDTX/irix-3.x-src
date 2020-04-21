| C library -- _exit

| _exit(code)
| code is return in d0 to system
| Same as plain exit, for users who want to define their own exit.

include(../DEFS.m4)

ENTRY(_exit)
	moveq	#1,d0
	movl	sp@(4),a0	| fetch argument
	trap	#0		| exit to system, no return
	stop	#0x0000
