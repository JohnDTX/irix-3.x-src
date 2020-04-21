static char Sccsid[]="@(#)userexit.c	3.1";
/*
	Default userexit routine for fatal and setsig.
	User supplied userexit routines can be used for logging.
*/

userexit(code)
{
	return(code);
}
