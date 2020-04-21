# Construct the various systems that are needed
# $Header: /d2/3.7/src/sys/conf/RCS/Makefile,v 1.1 89/03/27 17:26:53 root Exp $

include ${ROOT}/usr/include/make/commondefs

INSCMD=${INSTALL} -u root -g sys

PRODUCTS=	 1000 2000 3000

default: stomp config

../kernels:
	-mkdir ../kernels

#
# PRODUCT = 1000 or 2000 or 3000 -- set by default by wrapper
# 
install: stomp config ../kernels mkkernel $(SUBTYPE)
	./genall.sh $$PRODUCT
	${INSCMD} -idb "upd root sys" -dir /kernels 
	${INSCMD} -idb "sys" -dir "/usr/sys/kernels /usr/sys/conf"
	${INSCMD} -idb "upd root sys" -F /kernels \
		-src ../kernels/"$$PRODUCT.tcp" "$$PRODUCT.tcp"
	${INSCMD} -idb "xns sys" -F /kernels \
		-src ../kernels/$$PRODUCT.xns "$$PRODUCT.xns"
	${INSCMD} -idb "nfs" -F /kernels \
		-src ../kernels/$$PRODUCT.nfs "$$PRODUCT.nfs"
	${INSCMD} -idb "sys" -F /usr/sys/conf \
		"config stomp mkkernel newvers.sh genall.sh GENCONFIG"
	${INSCMD} -idb "sys" -m 644 -F /usr/sys/conf \
		"`echo $$PRODUCT.* proto_* files*` \
			devices.ip2 devices.pm2 \
			c-depend.awk c-depend.sh \
			stomp.c mkkernel.sh Makefile"

mkkernel: mkkernel.sh
	cp $@.sh $@
	chmod 755 $@

clean:
	rm -f *.o a.out
	for i in ${PRODUCTS}; do rm -rf ../$$i.*/*.o; done

clobber:
	rm -f *.o a.out
	rm -f config stomp mkkernel
	for i in ${PRODUCTS}; do rm -rf ../$$i.*; done
	for i in ${PRODUCTS}; do rm -rf $$i.*; done

config:	./config anything
./config:
	cd ../config; make
	rm -f ./config; ln ../config/config ./config

stomp: stomp.c
	${CC} -O -o stomp stomp.c

anything:


# these targets are used by genall.sh to avoid re-configuring, which means
#	not rebuilding the makefile every time, which allows us to not have
#	to make the dependencies every time.  They are used to generate
#	a makefile for the target kernel, and by side effects, make the
#	necessary configuration files and directories.
SUBTYPE=3000.tcp

../${SUBTYPE}/makefile:${SUBTYPE} ./config
	if test ! -d ../${SUBTYPE}; then \
		rm -rf ../${SUBTYPE}; mkdir ../${SUBTYPE}; \
	fi
	./config ${SUBTYPE}
	rm -f  ../${SUBTYPE}/incdepend

$(SUBTYPE):GENCONFIG ${CONFILES}
	./GENCONFIG
