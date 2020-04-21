/*	@(#)close.c	1.1	*/
#include <stdio.h>
closevt(){
	closepl();
}
closepl(){
	fflush(stdout);
	reset();
}
