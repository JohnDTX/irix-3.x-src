static char ID[] = "@(#)ranf.c	1.1";

ranf(max)
int	max;
{
	double	d;

	if (max <= 0)
		return (0);
	d = rand()/32768.0;
	return(d*max);
}


double franf()
{
	double		t;
	t = rand();
	return (t / 32767.0);
}
