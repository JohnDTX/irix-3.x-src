/*	boott.c	1.1	12/17/85	*/

/*
 * BOOTP/TFTP prom bootstrap.
 *
 * history
 * 12/17/85	croft	created.
 */


#include "bootp/defs.h"
#include "cpureg.h"
#include "tod.h"
#include "common.h"

struct ether_addr ether_broadcast = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

#ifdef DEBUG
extern char *ether_ntoa();
#endif

extern struct bootp *bootprecv();

/*
 * BOOTP protocol.
 */
bootp(host, file)
char *host;		/* requested server name */
char *file;		/* boot file name */
{
	struct bootpmsg msg;	/* BOOTP packet */
	register struct bootp *bp = &msg.bh;
	register struct udphdr *up = &msg.uh;
	register struct bootp *rbp;
	int trys = 0;
	iaddr_t ia;

	/*
	 * Construct the UDP and BOOTP headers.
	 */
	up->uh_sport = htons(IPPORT_BOOTPC);
	up->uh_dport = htons(IPPORT_BOOTPS);
	up->uh_ulen = htons(sizeof *up + sizeof (struct bootp));
	up->uh_sum = 0;
	bzero(bp, sizeof *bp);
	bp->bp_op = BOOTREQUEST;
	bp->bp_htype = ARPHRD_ETHER;
	bp->bp_hlen = sizeof (struct ether_addr);
	/*
	 * Select a new BOOTP transaction id
	 */
	bootp_xid = getrand(1024, 65535);
	bp->bp_xid = bootp_xid;
	bp->bp_ciaddr = btmyiaddr;
	bcopy((caddr_t)&btmyeaddr, bp->bp_chaddr, sizeof (struct ether_addr));
	if (host && strcmp(host, "*"))
		strcpy(bp->bp_sname, host);
	if (file)
		strcpy(bp->bp_file, file);
	dprintf(("Sending BOOTP request (host %s, file %s)\n",
		bp->bp_sname, bp->bp_file));

	ia = INADDR_BROADCAST;

	/*
	 * Broadcast request, receive replies until we get one or timeout.
	 */
	for (;;) {
		sendip(ia, ia, &msg, sizeof msg.ih + sizeof *up + sizeof *bp);
		if (rbp = bootprecv())
			break;
		/*
		 * Receive timed out.
		 */
		if (++trys > BOOTNTRIES) {
			dprintf(("BOOTP timeout - no server responds\n"));
			_tcperr = ETIMEDOUT;
			return(-1);
		}
	}
	dprintf(("BOOTP reply from %s(%x): file %s, myiaddr %x\n",
		rbp->bp_sname, rbp->bp_siaddr, rbp->bp_file, rbp->bp_yiaddr));
	/*
	 * Note that IP addresses are kept in network order
	 */
	if ( btmyiaddr == 0 && rbp->bp_yiaddr != 0 )
		btmyiaddr = rbp->bp_yiaddr;
	bootp_siaddr = rbp->bp_siaddr;
	if ( ! (bootp_giaddr = rbp->bp_giaddr) )
		bootp_giaddr = bootp_siaddr;
	if (host || strlen(host) == 0)
		strcpy(bootp_sname, rbp->bp_sname);
	else
		strcpy(bootp_sname, host);
	strcpy(bootp_file, rbp->bp_file);

	/*
	 * Now copy BOOTP values to the common area in static RAM,
	 * for use by programs that are loaded.
	 */
#define CPIADDR(p1, p2)	(*(iaddr_t *)(p2) = ntohl(*(iaddr_t *)(p1)))
	CPIADDR(&btmyiaddr, _commdat->c_iaddr);
	CPIADDR(&bootp_siaddr, _commdat->c_siaddr);
	CPIADDR(&bootp_giaddr, _commdat->c_giaddr);
	bcopy(rbp->bp_vend, _commdat->c_bootp, sizeof _commdat->c_bootp);

	_commdat->c_magic = COMMON_MAGIC;
	
	return(0);
}

/*
 * BOOTP receive
 *
 * Receive IP packets until a valid BOOTP reply is received or timeout.
 */
struct bootp *
bootprecv()
{
	register struct bootpmsg *rmp;
	register struct bootp *rbp;
	int count;

	for (;;) {
		if ((count = ipreceive(0)) == 0) /* if timeout */
			return ((struct bootp *)0);
		/*
		 * Verify the received message is a valid BOOTP reply
		 */
		if (count < sizeof (struct bootpmsg)) {
			dprintf(("bootprecv: msg too short (%d)\n", count));
			continue;
		}
		rmp = (struct bootpmsg *)_readdatabuf;
		rbp = &rmp->bh;
		if (checkudp(&rmp->ih,
		    sizeof (struct bootpmsg) - sizeof (struct ether_header),
		    IPPORT_BOOTPC) == 0) {
			dprintf(("bootprecv: ignore non-UDP pkt\n"));
			continue;
		}
		if (rbp->bp_xid != bootp_xid || rbp->bp_op != BOOTREPLY
		    || bcmp(rbp->bp_chaddr, (caddr_t)&btmyeaddr,
		    	    sizeof (struct ether_addr)) != 0) {
			dprintf(("bootprecv: ignore non-BOOTP pkt\n"));
			continue;
		}

		/* reply is valid */
		break;
	}
	
	return (rbp);
}

/*
 * TFTP connect
 *
 * Called from tcpstrategy to initiate a TFTP pseudo-connection.
 * Sends RRQ (Read ReQuest) packet to the server and waits for the response.
 */
tftpconnect()
{
	struct tftpdmsg msg;	/* max TFTP packet */
	register struct udphdr *uhp = &msg.uh;
	register struct tftphdr *thp = &msg.th;
	register int sendcount;	/* IP message size */
	int recvcount;
	int trys;
	register u_char *cp;

	/*
	 * Initialize the UDP and TFTP headers for RRQ packet.
	 */
	uhp->uh_dport = htons(IPPORT_TFTP);
	uhp->uh_sum = 0;
	thp->th_opcode = htons(RRQ);
	sendcount = TFTP_HDR_SIZE + sizeof *uhp;
	strcpy(thp->th_stuff, bootp_file);	/* set filename */
	for (cp = (u_char *)thp->th_stuff ; *cp ; cp++, sendcount++);
	cp++, sendcount++;
	strcpy(cp, "octet");			/* binary transfer mode */
	sendcount += 6;				/* 6 = strlen("octet") + 1 */
	uhp->uh_ulen = htons(sendcount);	/* UDP length */
	sendcount += sizeof (struct ip);	/* IP length */
	trys = 0;
	
	dprintf(("TFTP: RRQ for file %s to %s\n", bootp_file,
		(bootp_sname[0] != '\0') ? bootp_sname :
		(char *) inet_ntoa( ntohl( bootp_siaddr ))));

	/*
	 * send "RRQ" until a valid block is received or have to give up.
	 */
	for (;;) {
		/*
		 * Choose a different port on each attempt to avoid
		 * confusion with stale replies.
		 */
		tftp_myport = ++bootp_xid & 0x3FFF;
		uhp->uh_sport = htons(tftp_myport);

		if (++trys > 7) {
			dprintf(("TFTP: no response from server\n"));
			_tcperr = ETIMEDOUT;
			return(-1);
		}
		sendip(bootp_siaddr, bootp_giaddr, &msg, sendcount);
		/*
		 * Return if get positive number of bytes or negative
		 * return value indicating an error.
		 */
		if ((recvcount = tftprecv()) != 0)
			break;
		/*
		 * No data bytes returned.  If the last block has been
		 * received, this means the file was zero length!
		 */
		if (tftp_eof)
			break;
	}
	return(recvcount);
}

/*
 * TFTP read routine
 *
 * Called from tcpstrategy to get the next block of the current
 * boot file.  Current state of TFTP pseudo-connection and
 * current TFTP data pointer are updated before returning.
 *
 * Return value is number of new data bytes read.
 */
tftpread()
{
	int recvcount, trys;

	/*
	 * Connection should have been established by tcpopen
	 */
	if (tftp_blkno < 0) {
		printf("tftpread: internal error, no connection\n");
		return(-1);
	}

	dprintf(("TFTP: waiting for blk %d\n", tftp_blkno+1));
	trys = 0;

	/*
	 * Wait for the next block.  If it doesn't arrive, resend
	 * the last acknowledgement.
	 */
	for (;;) {
		/*
		 * Receive until a good reply comes in or timeout.
		 */
		if ((recvcount = tftprecv()) != 0)
			break;

		/*
		 * No data bytes returned.  Check for the case in which
		 * the last block contains zero data bytes and just
		 * return instead of retrying.
		 */
		if (tftp_eof)
			break;
		
		if (++trys > 7) {
			dprintf(("Server timeout ... Giving up.\n"));
			_tcperr = ETIMEDOUT;
			return(-1);
		}

		/*
		 * Poke the server by sending another ACK for the last block
		 */
		tftpack();
	}
	return (recvcount);	/* return number of data bytes received */
}

/*
 * TFTP receive.
 *
 * Receive ip messages until we get a valid TFTP reply or time out.
 * ACKs are sent to valid TFTP data blocks.
 *
 * Return value is the actual number of TFTP data bytes received
 * or -1 if error.
 */
tftprecv()
{
	register struct tftpmsg *rmp;
	register int recvcount;

	for (;;) {
		recvcount = ipreceive(100000);
		if (recvcount == 0)	/* if receive timeout */
			return(0);	/* let higher level decide ... */
		/*
		 * Take a look at the received ether packet
		 */
		rmp = (struct tftpmsg *)_readdatabuf;
		if (recvcount < sizeof rmp->ih + sizeof rmp->uh + TFTP_HDR_SIZE
		    || checkudp(&rmp->ih, sizeof rmp->ih +
			sizeof rmp->uh + TFTP_HDR_SIZE,
			tftp_myport) == 0) {
			dprintf(("tftprecv: ignore non-TFTP pkt\n"));
			continue;
		}
		if (rmp->ih.ip_src != bootp_siaddr) {
			dprintf(("tftprecv: pkt from wrong server\n"));
			continue;
		}
		if (tftp_blkno > 0 && ntohs(rmp->uh.uh_sport) != tftp_srvport) {
			dprintf(("tftprecv: pkt from port %d, not %d\n",
				rmp->uh.uh_sport, tftp_srvport));
			continue;
		}
		switch (ntohs(rmp->th.th_opcode)) {
		default:
			dprintf(("tftprecv: invalid TFTP opcode 0x%x\n",
				rmp->th.th_opcode));
			continue;

		case ERROR:
			printf("\nTFTP error: %s (code %d)\n",
			    rmp->th.th_msg, ntohs(rmp->th.th_code));
			_tcperr = ECONNABORTED;
			return (-1);

		case DATA:
			break;
			/* fall thru and break out of receive loop */
		}
		break;
	}
	
	if (tftp_blkno < 0) {	/* receiving 1st block */
		tftp_srvport = ntohs(rmp->uh.uh_sport);
		tftp_blkno = 0;
		tftp_eof = 0;
	}

	if (ntohs(rmp->th.th_block) != tftp_blkno+1) {
		dprintf(("tftprecv: got blk %d, expect %d\n",
			rmp->th.th_block, tftp_blkno+1));
		return (0);	/* if not expected block, resend */
	}

	/*
	 * Increment block number
	 */
	tftp_blkno++;

	/*
	 * Compute actual count of TFTP data bytes received
	 */
	if ((recvcount=ntohs(rmp->uh.uh_ulen)
		- sizeof rmp->uh - TFTP_HDR_SIZE) > 0) {
		tftp_dataptr = rmp->th.th_data;
	}
	dprintf(("tftprecv: got blk nbr %d, %d bytes\n",
		tftp_blkno, recvcount));
	
	if (recvcount < SEGSIZE)
		/*
		 * This is the last block of the file.  Need to set a flag
		 * so that the higher levels can distinguish EOF from the
		 * case that the last block contains zero bytes.
		 */
		tftp_eof = 1;

	/*
	 * Acknowledge this block now to get some overlap with the server.
	 */
	tftpack();

	/*
	 * Note that recvcount may be 0 in the case that we
	 * have just received the last block of a file that has
	 * length divisible by 512 bytes.
	 */
	return(recvcount);
}

/*
 * TFTP send acknowlegment routine.
 *
 * Format and transmit an acknowledgement for the most recently
 * received block.
 */
tftpack()
{
	struct tftpmsg msg;	/* TFTP packet */
	register struct udphdr *uhp = &msg.uh;
	register struct tftphdr *thp = &msg.th;
	int sendcount;

	/*
	 * It is an error if we are acknowledging a block
	 * number less than 1.
	 */
	if (tftp_blkno < 1) {
		printf("tftpack: called for bogus block %d\n", tftp_blkno);
		return;
	}

	/*
	 * Initialize the UDP and TFTP headers for ACK.
	 */
	uhp->uh_sport = htons(tftp_myport);
	uhp->uh_dport = htons(tftp_srvport);
	uhp->uh_sum = 0;
	thp->th_opcode = htons(ACK);
	thp->th_block = htons(tftp_blkno);
	sendcount = sizeof *uhp + TFTP_HDR_SIZE;
	uhp->uh_ulen = htons(sendcount);	/* UDP packet length */
	sendcount += sizeof (struct ip);	/* IP length */
	dprintf(("TFTP send ACK for blk %d\n", tftp_blkno));

	sendip(bootp_siaddr, bootp_giaddr, &msg, sendcount);
}

/*
 * TFTP error send routine.
 *
 * Used when we don't intend to read all the blocks of the file.
 * Sends a TFTP error packet to stop the server from retrying.
 */
tftpabort()
{
	struct tftpmsg msg;	/* TFTP packet */
	register struct udphdr *uhp = &msg.uh;
	register struct tftphdr *thp = &msg.th;
	int sendcount;

	/*
	 * Initialize the UDP and TFTP headers for ERROR packet.
	 */
	uhp->uh_sport = htons(tftp_myport);
	uhp->uh_dport = htons(tftp_srvport);
	uhp->uh_sum = 0;
	thp->th_opcode = htons(ERROR);
	thp->th_code = htons(ENOSPACE);
	sendcount = sizeof *uhp + TFTP_HDR_SIZE;
	uhp->uh_ulen = htons(sendcount);	/* UDP packet length */
	sendcount += sizeof (struct ip);	/* IP length */
	dprintf(("tftpabort: TFTP send ERROR\n"));

	/*
	 * Send "ERROR" only once, since it will not be acknowledged.
	 */
	sendip(bootp_siaddr, bootp_giaddr, &msg, sendcount);
}

/*
 * Send an IP packet.  Perform ARP if necessary.
 * 'dstaddr' is the address to be used in the IP header,
 * 'gateaddr' is the immediate destination address.
 * 'mp' is a pointer to the full IP packet to be sent.
 * 'count' is the length of the IP header plus data.
 */
sendip(dstaddr, gateaddr, mp, count)
	iaddr_t dstaddr, gateaddr;
	register struct ipmsg *mp;
	int count;
{
	register struct ether_header *ehp = &mp->eh;
	register struct ip *ihp = &mp->ih;
	register int hlen;

	Dprintf(2, ("sendip dest %x, count %d\n", gateaddr, count));

	ihp->ip_v = IPVERSION;
	ihp->ip_len = htons(count);
	ihp->ip_id++;
	ihp->ip_off = 0;
	ihp->ip_ttl = MAXTTL;
	ihp->ip_p = IPPROTO_UDP;
	ihp->ip_src = btmyiaddr;
	ihp->ip_dst = dstaddr;
	ihp->ip_sum = 0;
	hlen = sizeof (struct ip);
	ihp->ip_hl = (u_char) (hlen >> 2);
	ihp->ip_sum = in_cksum((caddr_t)ihp, hlen);

	count += sizeof (struct ether_header);

	if (gateaddr == INADDR_BROADCAST)
		ehp->ether_dhost = ether_broadcast;
	else if (gateaddr == arpiaddr && arpvalid)
		ehp->ether_dhost = arpeaddr;
	else {
		arpiaddr = gateaddr;
		arpwait = 1;
		arpvalid = 0;
		arpqueue = mp;
		arpqueuelen = count;
		sendarp(ARPOP_REQUEST, &gateaddr, 0);
		return;
	}

	_etherwrite(ETHERTYPE_IPTYPE, mp, count);
}


#ifdef DEBUG
/*
 * Note: this buffer must NOT be static, or the compiler will make
 * it DATA instead of BSS with unfortunate results when burned into
 * PROMs.
 */
char _ether_ntoa_buf[19];
/*
 * Converts a 48 bit ethernet number to its string representation.
 */
#define EI(i)	(unsigned char)(e->ether_addr_octet[(i)])
char *
ether_ntoa(e)
	struct ether_addr *e;
{
	char *retp;

	if (e == (struct ether_addr *)0)
		strcpy(_ether_ntoa_buf, "NULL");
	else {
		bzero(_ether_ntoa_buf, sizeof _ether_ntoa_buf);
		sprintf(_ether_ntoa_buf, "%x:%x:%x:%x:%x:%x",
			EI(0), EI(1), EI(2), EI(3), EI(4), EI(5));
	}
	return (_ether_ntoa_buf);
}
#endif


/*
 * Send an ARP packet.
 */
sendarp(req, tpa, tha)
	int req;		/* ARP request type */
	iaddr_t	*tpa;		/* Target protocol address */
	eaddr_t	*tha;		/* Target hardware address */
{
	struct arpmsg msg;
	register struct ether_arp *eap = &msg.ah;

	dprintf(("sendarp: req %d, tpa %x, tha %s\n",
		req, *tpa, ether_ntoa(tha)));

	msg.eh.ether_dhost = (tha ? *tha : ether_broadcast);
	bzero((caddr_t)eap, sizeof *eap);
	eap->arp_hrd = htons(ARPHRD_ETHER);
	eap->arp_pro = htons(ETHERTYPE_IPTYPE);
	eap->arp_hln = sizeof (eaddr_t);
	eap->arp_pln = sizeof (iaddr_t);
	eap->arp_op = htons(req);
	arp_sha(eap) = btmyeaddr;
	if (btmyiaddr == 0) {
		printf("sendarp: can't send: my ipaddr is zero!\n");
		return;
	}
	arp_spa(eap) = btmyiaddr;
	arp_tpa(eap) = *tpa;
	if (tha)
		arp_tha(eap) = *tha;

	_etherwrite(ETHERTYPE_ARPTYPE, &msg, sizeof msg);
}


/*
 * Receive the next IP packet.  If an ARP reply packet is received,
 * we send any pending IP packet, then continue receiving.
 *
 * We finally return if a packet is received or the driver timer
 * goes off.  Return value is receive count or 0 if timeout.
 * Filter out any non-IP packets;  remove any IP options if present.
 */
ipreceive(timer)
register long timer;
{
	register struct ipmsg *mp;
	register struct ether_header *ehp;
	register struct ether_arp *eap;
	register struct ip *ihp;
	int count;
	int options;

	if (timer == 0)
		timer = 60000;	/* default */

	while (timer--) {
		_readenable();
		if ((count = _nxread()) <= sizeof *ehp)
			continue;

		mp = (struct ipmsg *)_readdatabuf;
		ehp = (struct ether_header *)&mp->eh;

		Dprintf(2, ("ipreceive: got count %d, type 0x%x, from %s\n",
			count, ehp->ether_type, ether_ntoa(&ehp->ether_shost)));

		switch (ntohs(ehp->ether_type)) {
		case ETHERTYPE_ARPTYPE:
			if (count < (sizeof *ehp + sizeof *eap)) {
				Dprintf(2, ("ipreceive: bad ARP packet (len %d)\n",
					count));
				continue;
			}
			break;	/* break switch and handle ARP */

		case ETHERTYPE_IPTYPE:
			if (count < (sizeof *ehp + sizeof *ihp)) {
				dprintf(("ipreceive: bad IP packet (len %d)\n",
					count));
				continue;
			}
			ihp = &mp->ih;
			if (in_cksum((caddr_t)ihp, ihp->ip_hl<<2) != 0) {
				dprintf(("ipreceive: IP packet w/bad checksum\n"));
				continue;
			}
			if (ihp->ip_v != IPVERSION
			    || (ntohs(ihp->ip_off) & 0x3FFF)) {
				dprintf(("ipreceive: bad IP packet: version %d, off %x\n",
					ihp->ip_v, ntohs(ihp->ip_off)));
				continue;
			}
			if ((options = (ihp->ip_hl<<2) - sizeof *ihp) > 0) {
				/* discard options */
				bcopy((caddr_t)&ihp[1] + options,
				 (caddr_t)&ihp[1], count-options-sizeof *ihp);
				ihp->ip_len = htons(ntohs(ihp->ip_len)-options);
				count -= options;
			}
			return (count);

		default:
			continue;
		}

		/*
		 * We've received an ARP.
		 */
		eap = &((struct arpmsg *)mp)->ah;
		if (ntohs(eap->arp_hrd) != ARPHRD_ETHER
		    || eap->arp_hln != sizeof (eaddr_t)
		    || eap->arp_pln != sizeof (iaddr_t)) {
			Dprintf(2,("ipreceive: malformed ARP packet\n"));
			continue;
		}
		if (ntohs(eap->arp_pro) != ETHERTYPE_IPTYPE) {
			/*
			 * Probably a TRAILER request ... ignore it
			 */
			Dprintf(2,("ipreceive: ARP for proto other than IP\n"));
			continue;
		}
		if (ntohs(eap->arp_op) == ARPOP_REPLY) {
			if (arpwait == 0
			    || arp_tpa(eap) != btmyiaddr
			    || arp_spa(eap) != arpiaddr) {
				Dprintf(2,("ipreceive: ARP reply for tpa %x (ignored)\n",
					arp_tpa(eap)));
				continue;
			}
			/*
			 * This is the reply we were waiting for!
			 * Fill in the resolution in the ARP table and
			 * send the pending message.
			 */
			arpeaddr = arp_sha(eap);
			dprintf(("ipreceive: got ARP reply: tpa %x -> tha %s\n",
				arpiaddr, ether_ntoa(&arpeaddr)));
			arpwait = 0;
			arpvalid = 1;
			arpqueue->eh.ether_dhost = arpeaddr;
			_etherwrite(ETHERTYPE_IPTYPE, arpqueue, arpqueuelen);
			arpqueue = (struct ipmsg *)0;
		} else if (ntohs(eap->arp_op) == ARPOP_REQUEST) {
			if (btmyiaddr == 0 || arp_tpa(eap) != btmyiaddr) {
				Dprintf(2,("ipreceive: ARP request for tpa %x from spa %x\n",
					arp_tpa(eap), arp_spa(eap)));
				continue;
			}
			dprintf(("ipreceive: ARP request for %x (ME!)\n",
				arp_tpa(eap)));
			sendarp(ARPOP_REPLY, &arp_spa(eap), &arp_sha(eap));
		}
	}
	Dprintf(2,("ipreceive: timeout\n"));
}


/*
 * Check an incoming IP/UDP datagram.
 * Returns 0 if bad.
 */
checkudp(ip, miniplen, dport)
	struct udpmsg *ip;
	int miniplen, dport;
{
	register struct ip *ihp = &ip->ih;
	register struct udphdr *uhp = &ip->uh;

	if ( ntohs(ihp->ip_len) < miniplen || ihp->ip_p != IPPROTO_UDP)
		return(0);
	if (btmyiaddr && ihp->ip_dst != btmyiaddr)
		return(0);
	if (uhp->uh_dport != htons(dport)
	    || ntohs(uhp->uh_ulen) < (miniplen - sizeof ip->ih))
		return(0);

	return(1);
}


/*
 * Make up a sort-of-random number.
 *
 * Note that this is not a general purpose routine, but is geared
 * toward the specific application of generating an integer in the
 * range of 1024 to 65k.
 */
getrand(lb, ub)
int lb;		/* lower bound */
int ub;		/* upper bound */
{
	struct tod_dev	tod_dev;
	register int	rand;

	/*
	 * Read the real time clock
	 */
	todread(CLK_DATA, (u_char *)&tod_dev, sizeof tod_dev);

	/*
	 * Mush together the date and time numbers
	 */
	rand =  tod_dev.td_year  << 7;
	rand += tod_dev.td_month << 6;
	rand += tod_dev.td_sec   << 5;
	rand += tod_dev.td_min   << 4;
	rand += tod_dev.td_hrs   << 3;
	/*
	 * Use the fastest changing to introduce more variability
	 */
	rand >>= (tod_dev.td_sec & 3);

	/*
	 * Now fit the range
	 */
	ub -= lb;
	rand = rand % ub;
	rand += lb;

	return (rand);
}
