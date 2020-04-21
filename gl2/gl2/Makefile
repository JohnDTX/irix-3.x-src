#
# Make the gl - all the dirty work is done in the processor dependent
# trees - kipp
#
# do not use $Install -- an install is done before commondefs are available

all:
	cd ucode; make $@
	-@if test "$$machine"x = "juniper"x; then \
		echo cd ip2 \; make $@ ; \
		cd ip2; make $@; \
	else \
		echo cd pm2 \; make $@ ; \
		cd pm2; make $@; \
	fi
#
# Install the mess
install:
	cd ucode; make $@
	cp lib.prim ${ROOT}/usr/lib/${GLHDWR}/.
	-@if test "$$machine"x = "juniper"x; then \
		echo cd ip2 \; make $@ ; \
		cd ip2; make $@; \
	else \
		echo cd pm2 \; make $@ ; \
		cd pm2; make $@; \
	fi

#
# machine is not set yet when 'clean' and 'clobber' are done
# from the top level mk script.  so we clobber both ip2 and pm2.
#
clean clobber:
	cd ucode; make $@
	cd ip2; make $@
	cd pm2; make $@
