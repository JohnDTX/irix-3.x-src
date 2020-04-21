
/* C library -- exit	*/
/* exit(code) */

exit(status)
int status;
{
	_cleanup();
	_exit(status);
}
