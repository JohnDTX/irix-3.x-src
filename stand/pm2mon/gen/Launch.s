|
| LaunchStack(func,stack,nargs,arg0,...)
|     int (*func)();
|     long *stack;
|     int nargs;
|     int arg0,...;
|
| push nargs args, and jump to func func with stack stack.
|
	.globl	_LaunchStack
_LaunchStack:
	movl	sp@+,d0			| no return!
	movl	sp@+,a0			| get entry point
	movl	sp@+,a1			| get new stack
	movl	sp@+,d0			| get nargs
	movl	d0,d1			| convert to bytes
	lsll	#2,d1
	addl	d1,sp			| point current stack to last arg
	jra	8$			| loop over nargs
3$:
	movl	sp@-,a1@-		| push on new stack
8$:
	subql	#1,d0
	jge	3$
	movl	a1,sp			| switch to new stack
	jmp	a0@			| enter new code
