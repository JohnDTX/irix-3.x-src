/*      rpcinfo.c     1.1     86/02/05     */
/* @(#)rpcinfo.c	2.1 86/04/17 NFSSRC */ 

/*
 * Copyright (C) 1984, Sun Microsystems, Inc.
 */

/*
 * rpcinfo: ping a particular rpc program
 *     or dump the portmapper
 */

#include <rpc/rpc.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <rpc/pmap_prot.h>
#include <rpc/pmap_clnt.h>
#include <ctype.h>

#define MAXHOSTLEN 256

main(argc, argv)
	char **argv;
{
	if (argc < 2) {
		usage();
		exit(1);
	}
	if (argv[1][0] == '-') {
		switch(argv[1][1]) {
			case 't':
				tcpping(argc-1, argv+1);
				break;
			case 'p':
				pmapdump(argc-1, argv+1);
				break;
			case 'u':
				udpping(argc-1, argv+1);
				break;
			default:
				usage();
				exit(1);
				break;
		}
	}
	else
		usage();
}
		
udpping(argc, argv)
	char **argv;
{
	int ans, prognum, vers;
	struct rpcent *rpc;
    
	if (argc < 3 || argc > 4) {
		usage();
		exit(1);
	}
	if (isalpha(argv[2][0])) {
		rpc = getrpcbyname(argv[2]);
		if (rpc == NULL) {
			fprintf(stderr, "%s is unknown name\n", argv[2]);
			exit(1);
		}
		prognum = rpc->r_number;
	}
	else
		prognum = atoi(argv[2]);
	if (argc == 3)
		vers = 1;
	else
		vers = atoi(argv[3]);
	ans = callrpc(argv[1], prognum, vers, NULLPROC,
	    xdr_void, 0, xdr_void, 0);
	if (ans != 0) {
		clnt_perrno(ans);
		fprintf(stderr, "\n");
		printf("program %d version %d is not available\n",
		    prognum, vers);
		exit(1);
	}
	else
		printf("program %d version %d ready and waiting\n",
		    prognum, vers);
}

tcpping(argc, argv)
	int argc;
	char **argv;
{
	struct timeval to;
	struct sockaddr_in addr;
	enum clnt_stat rpc_stat;
	CLIENT *client;
	int prognum, vers;
	int sock = -1;
	struct hostent *hp;
	struct rpcent *rpc;

	if (argc < 3 || argc > 4) {
		usage();
		exit(1);
	}
	if (isalpha(argv[2][0])) {
		rpc = getrpcbyname(argv[2]);
		if (rpc == NULL) {
			fprintf(stderr, "%s is unknown name\n", argv[2]);
			exit(1);
		}
		prognum = rpc->r_number;
	}
	else
		prognum = atoi(argv[2]);
	if ((hp = gethostbyname(argv[1])) == NULL) {
	    fprintf(stderr, "can't find %s\n", argv[1]);
	    exit(1);
	}
	addr.sin_family = AF_INET;
	addr.sin_port = 0;
	addr.sin_addr.s_addr = *(int *)hp->h_addr;
	if (argc == 3)
		vers = 1;
	else
		vers = atoi(argv[3]);
	if ((client = clnttcp_create(&addr, prognum,
		vers, &sock, 0, 0)) == NULL) {
			clnt_pcreateerror("");
			printf("program %d version %d is not available\n",
			    prognum, vers);
			exit(1);
		}
	to.tv_usec = 0;
	to.tv_sec = 10;
	rpc_stat = clnt_call(client, 0, xdr_void, NULL, xdr_void, NULL, to);
	if (rpc_stat != RPC_SUCCESS) {
		clnt_perrno(rpc_stat);
		fprintf(stderr, "\n");
		printf("program %d version %d is not available\n",
		    prognum, vers);
		exit(1);
	}
	else
		printf("program %d version %d ready and waiting\n",
		    prognum, vers);
}

pmapdump(argc, argv)
	int argc;
	char **argv;
{
	struct sockaddr_in server_addr;
	struct hostent *hp;
	struct pmaplist *head = NULL;
	char hoststr[MAXHOSTLEN];
	int socket = -1;
	struct timeval minutetimeout;
	char *hostnm;
	register CLIENT *client;
	enum clnt_stat rpc_stat;
	struct rpcent *rpc;
	
	if (argc > 2) {
		usage();
		exit(1);
	}
	if (argc == 2) {
		hostnm = argv[1];
	} else {
		gethostname(hoststr, sizeof(hoststr));
		hostnm = hoststr;
	}
	if ((hp = gethostbyname(hostnm)) == NULL) {
		fprintf(stderr, "cannot get addr for '%s'\n", hostnm);
		exit(0);
	}
	bcopy(hp->h_addr, (caddr_t)&server_addr.sin_addr, hp->h_length);
	server_addr.sin_family = AF_INET;
	minutetimeout.tv_sec = 60;
	minutetimeout.tv_usec = 0;
	server_addr.sin_port = htons(PMAPPORT);
	if ((client = clnttcp_create(&server_addr, PMAPPROG,
	    PMAPVERS, &socket, 50, 500)) == NULL) {
		clnt_pcreateerror("rpcinfo: can't contact portmapper");
		exit(1);
	}
	if ((rpc_stat = clnt_call(client, PMAPPROC_DUMP, xdr_void, NULL,
	    xdr_pmaplist, &head, minutetimeout)) != RPC_SUCCESS) {
		fprintf(stderr, "rpcinfo: can't contact portmapper: ");
		clnt_perrno(rpc_stat);
		fprintf(stderr, "\n");
		exit(1);
	}
	if (head == NULL) {
		printf("No remote programs registered.\n");
	} else {
		printf("   program vers proto   port\n");
		for (; head != NULL; head = head->pml_next) {
			printf("%10ld%5ld",
			    head->pml_map.pm_prog,
			    head->pml_map.pm_vers);
			if (head->pml_map.pm_prot == IPPROTO_UDP)
				printf("%6s",  "udp");
			else if (head->pml_map.pm_prot == IPPROTO_TCP)
				printf("%6s", "tcp");
			else
				printf("%6ld",  head->pml_map.pm_prot);
			printf("%7ld",  head->pml_map.pm_port);
			rpc = getrpcbynumber(head->pml_map.pm_prog);
			if (rpc)
				printf("  %s\n", rpc->r_name);
			else
				printf("\n");
		}
	}
}

usage()
{
	fprintf(stderr, "Usage: rpcinfo -u host prognum [versnum]\n");
	fprintf(stderr, "       rpcinfo -t host prognum [versnum]\n");
	fprintf(stderr, "       rpcinfo -p [host]\n");
}
