
zsend(f, msg, size)
register struct zmsg *msg;
{
	return(write(f, msg, size));
}
