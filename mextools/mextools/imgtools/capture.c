#include <stdio.h>
#include <gl.h>

main(argc,argv,envp)
    int argc;
    char *argv[], **envp;
{
    if(argc != 2) {
	fprintf(stderr,"usage: capture <filename>\n");
	exit(1);
    }
    foreground();
    noport();
    getport("capture");
    scrsave(argv[1],1,0,XMAXSCREEN,0,YMAXSCREEN);
    gexit();
}
