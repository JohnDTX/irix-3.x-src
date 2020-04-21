#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <bsd/netdb.h>
#include <stdio.h>

main()
{
   int sock, length;
   struct sockaddr_in sin;
   int msgsock;
   char line[80];
   int i, cnt;

/* create a socket */

   if ((sock = socket (AF_INET,SOCK_STREAM,0)) < 0) {
      perror("opening stream socket");
      exit(0);
      }

/* initialize socket data structure */

   sin.sin_family = AF_INET;
   sin.sin_addr.s_addr = INADDR_ANY;
   sin.sin_port = htons(IPPORT_RESERVED + 1);

/* bind socket data structure to this socket */

   if (bind (sock,&sin,sizeof(sin))) {
      perror("binding stream socket");
      }

/* getsockname fills in the socket structure with information, such as
   the port number assigned to this socket */

   length = sizeof(sin);
   if (getsockname (sock,&sin,&length)) {
      perror("getting socket name");
      exit(0);
      }
   printf ("Socket has port# %d\n",htons(sin.sin_port));

/* prepare socket queue for connection requests and accept connections */

   listen(sock,5);
   if ((msgsock = accept(sock,0,0)) <= 0) {
      perror("accept on socket");
      exit(0);
      }

/* read from the message socket and write to standard out */

   while ( (cnt = read(msgsock, line, 80)) > 0)
      write(1,line,cnt);

   close(msgsock);
   printf("Done\n");
}
