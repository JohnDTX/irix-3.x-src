char *basename(path)
    register char *path;
{
    register char *bp;

    bp = path;
    while( *path != 000 )
    {
	if( *path == '/' )
	{
	    while( *path == '/' )
		path++;
	    if( *path == 000 )
		break;
	    bp = path;
	}
	path++;
    }

    return bp;
}
