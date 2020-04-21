# Generate Sun compatibility and RPC service libraries.
include ${ROOT}/usr/include/make/commondefs

RPCHEADERS = rpc/auth.h rpc/auth_unix.h rpc/clnt.h rpc/pmap_clnt.h  \
	     rpc/pmap_prot.h rpc/rpc.h rpc/rpc_msg.h rpc/svc.h  \
	     rpc/svc_auth.h rpc/types.h rpc/xdr.h 

SVCHEADERS = rpcsvc/ether.h rpcsvc/mount.h rpcsvc/rquota.h \
	     rpcsvc/rstat.h rpcsvc/rusers.h rpcsvc/rwall.h \
	     rpcsvc/spray.h rpcsvc/yppasswd.h rpcsvc/yp_prot.h \
	     rpcsvc/ypclnt.h rpcsvc/ypv1_prot.h 

# set NOPROF to '#' to avoid generating profiled version
NOPROF=
POK= p

LIBNAMES= libsun.a librpcsvc.a
PLIBNAMES= libsun_p.a librpcsvc_p.a
.PRECIOUS: ${LIBNAMES} ${PLIBNAMES}

IDBTAG= -idb "std.sw.unix"
INSCMD=${INSTALL} ${IDBTAG} -u root -g sys 
SUBDIRS= gen net rpc rpcsvc yp

default: ${SUBDIRS}

${SUBDIRS}: _force
	@echo "====\c"
	cd $@; ${MAKE} default 

install: default
	${INSTALL} ${IDBTAG} -F /usr/lib "${LIBNAMES} ${PLIBNAMES}"
	cd ${ROOT}/usr/lib; ranlib ${LIBNAMES} ${PLIBNAMES}

Include:
	${INSCMD} -dir "/usr/include/rpc /usr/include/rpcsvc"
	${INSCMD} -m 444 -F /usr/include "${RPCHEADERS} ${SVCHEADERS}"

depend incdepend clean rmtargets fluff: _force
	@for i in ${SUBDIRS} ; do \
		echo "====\tcd $$i; make $@" ; \
		cd $$i; ${MAKE} $@ ; cd ..; \
	done

clobber : clobber_libs

clobber_libs: _force
	rm -f ${LIBNAMES} ${PLIBNAMES}
	@for i in ${SUBDIRS} ; do \
		echo "====\tcd $$i; make clobber" ; \
		cd $$i; ${MAKE} clobber ; cd ..; \
	done

tags: _force
	ctags `find . -name '*.[ch]' -print`

_force:
