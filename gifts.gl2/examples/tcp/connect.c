#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <bsd/netdb.h>
#include <stdio.h>


main(argc,argv)
int argc;
char **argv;
{
   int cnt,sock;
   struct sockaddr_in sin;
   struct hostent *hp, *gethostbyname();
   char line[80];

   if (argc != 2) {
      printf("usage:%s host\n",argv[0]);
      exit(0);
   }

/* open socket */

   if ((sock = socket (AF_INET,SOCK_STREAM,0)) < 0) {
      perror("opening stream socket");
      exit(0);
      }

/* initialize socket data structure */

   sin.sin_family = AF_INET;
   hp = gethostbyname(argv[1]);    /* to get host address */
   bcopy (hp->h_addr, &(sin.sin_addr.s_addr), hp->h_length);
   sin.sin_port = htons(IPPORT_RESERVED+1); 

/* connect to remote host */

   if (connect(sock,&sin,sizeof(sin)) < 0) {
      close(sock);
      perror("connection streams socket");
      exit(0);
      }

   while ((cnt = read(0,line,80)) > 0) {
      printf("sending <");
      fflush(stdout);
      write(1,line,cnt-1);
      printf("\\n>\n");
      fflush(stdout);

/* send input to remote host */

      if (write(sock,line,cnt) < 0) {
         perror("writing on stream socket");
         exit(0);
      }
   }
   printf ("Done\n");
   close(sock);
}
