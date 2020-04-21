/*
 * graph.c - a simple graphics interpreter for the iris.
 *
 */

# include <Vio.h>

# define BUFSIZE 1024
/* #define TRACE */

File *imagef;
char oneline[BUFSIZE];

main(argc,argv)
int argc;
char **argv;
{
    Message Msg;
    char fileName[128];
    SystemCode error;
    char ch;
    char c;
    int i;
    int line;
    char enstr[10];

    framebufinit();
    while(1)
    {
        printf("\nDisplay what image:  ");
        gets(fileName);
    
        imagef = Open( fileName, FREAD | FBLOCK_MODE, &error);
        if (error || (imagef == NULL)) 
          printf("Open error: %s\r\n",ErrorString(error));
        else 
        {
    
          printf("\nenables: rgb: ");
	  for (i=0; i<8; i++) {
		c = getchar ();
		if ((c == ' ') || (c == '\n') || (c == '	')) {
			enstr[i] = '\0';
			break;
			}
		enstr[i] = c;
		}
	  printf ("\nyou typed %8s\n", enstr);
          colorenable (findchar ('r', enstr),
		       findchar ('g', enstr),
		       findchar ('b', enstr));
          display();
          Close(imagef);
        }
    }
}

int	findchar (c, s)
char	c, s[];
{
    /*
     *	Returns TRUE if c occurs anywhere in s, FALSE otherwise.
     */
    while (*s != '\0')
	if (*s++ == c)
	    return (1);
    return (0);
    }

display()
{
    int i;

    if( Read(imagef,oneline,BUFSIZE) != BUFSIZE )
    {
        printf("error on read!!\n");
        return(0);
    }
        imagef->block++;
    drawpixels(0,511,oneline+512,512,1);

    for(i=510; i>0; i-=2)
    {
	printf(".");
        if( Read(imagef,oneline,BUFSIZE) != BUFSIZE )
        {
	    printf("error on read!!\n");
            return(0);
        }
        imagef->block++;
        drawpixels(0,i+0,oneline,512,1);
        drawpixels(0,i-1,oneline+512,512,1);
    }
}

framebufinit()
{
 	init();
	clearscreen();
	setrect(0,0,512,512);
	colorenable(1,1,1);
}
