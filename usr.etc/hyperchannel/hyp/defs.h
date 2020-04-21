#
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <pwd.h>
#include <ctype.h>
#include <errno.h>

	/* table sizes */
#define INMAX	8500
#define NTRUNKS 4     /* trunks 0,1,2,3 */
#define HYRSIZE 257

/* This defines a particular hyperchannel adapter. */
struct hynode {
  struct hynode *nd_next;
  int            nd_use;
  char          *nd_hname;
  struct in_addr nd_addr;
  int            nd_adapter;
  int            nd_net[NTRUNKS];
};

char *makestr();
char *malloc();
char *rindex();
char *index();
char *makens();
char *strcpy();

