
include ${ROOT}/usr/include/make/commondefs

# These dot.c files are here for history, they are self modifying
# depending on where they get included

KERNELH = device.h gl.h get.h gltypes.h gr.h mouse.h shmem.h window.h

DOTC = gf2init.c

DOTH =	TheMacro.h addrs.h curvmac.h dcdev.h device.h dials.h get.h \
	gf2.h gl.h gl2cmds.h glerror.h glipc.h globals.h gltypes.h gr.h \
	grioctl.h imattrib.h imdraw.h immatrix.h immed.h imsetup.h interp.h \
	kb.h kfont.h mouse.h printf.h rpc.h shmem.h splinegl.h window.h \
	uc4.h fastpix.h

INCS = $(DOTC) $(DOTH)

install Include:
	${INSTALL} -dir /usr/include/gl2 -u bin -g bin -m 755   \
		-idb "std.sw.unix mr" 
	${INSTALL} -u root -g sys -m 444 -F /usr/include/gl2 \
		-idb "std.sw.unix mr" "${INCS}"
	${INSTALL} -ln /usr/include/gl2/gl.h -F /usr/include \
		-idb "std.sw.unix mr"  "gl.h "
	${INSTALL} -ln /usr/include/gl2/get.h -F /usr/include \
		-idb "std.sw.unix mr"  " get.h"
	${INSTALL} -ln /usr/include/gl2/device.h -F /usr/include \
		-idb "std.sw.unix mr" device.h

# install: last_installed
# 
# last_installed:	$(INCS)
#	cp $? $(ROOT)/usr/include/gl2
#	cp $(KERNELH) $$SYSDEST
#	date > last_installed

tomsinstall:	$(INCS)
	cp $(KERNELH) $$SYSDEST
	date > last_installed

clean:

release:
	release $(INCS) Makefile Files
