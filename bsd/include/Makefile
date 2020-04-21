

include ${ROOT}/usr/include/make/commondefs


#
#	Build BSD compatibility subtree of /usr/include
#
# 

BSDHEADERS = dbm.h ndbm.h netdb.h sgtty.h syslog.h 

SYSHEADERS = sys/dir.h sys/file.h sys/ioctl.h sys/msgbuf.h sys/param.h \
	     sys/resource.h sys/time.h sys/ttychars.h sys/types.h sys/wait.h 

ARPAHEADERS = arpa/ftp.h arpa/inet.h arpa/nameser.h arpa/resolv.h \
	      arpa/telnet.h arpa/tftp.h 

NETNSHEADERS = netns/idp.h netns/idp_var.h netns/ns.h netns/ns_error.h \
	       netns/ns_if.h netns/ns_pcb.h netns/sp.h netns/spidp.h \
	       netns/spp_debug.h netns/spp_var.h 

PROTOHEADERS = protocols/dumprestore.h protocols/routed.h protocols/rwhod.h \
	       protocols/timed.h 

PERMW  = -u root -g sys -m 644
PERM0  = -u root -g sys -m 444
IDBTAG = -idb "std.sw.unix"
DIRS   = /usr/include/bsd /usr/include/sys /usr/include/protocols \
 	 /usr/include/arpa /usr/include/netns"

default:	Include

Include:
	${INSTALL} -dir "${DIRS} ${PERMW}" -${IDBTAG} 
	${INSTALL} ${PERM0} -F /usr/include/bsd ${IDBTAG} \
		"${BSDHEADERS} ${SYSHEADERS} ${ARPAHEADERS} ${NETNSHEADERS} \
	 	 ${PROTOHEADERS}" 
	${INSTALL} -ln /usr/include/bsd/syslog.h -F /usr/include/bsd/sys \
	${IDBTAG} syslog.h
